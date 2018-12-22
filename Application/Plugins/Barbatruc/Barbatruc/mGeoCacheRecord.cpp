#include "mRegister.h"

bbppGeoCacheRecord::bbppGeoCacheRecord(){}

bbppGeoCacheRecord::~bbppGeoCacheRecord() {}

void* bbppGeoCacheRecord::creator()
{
    return new bbppGeoCacheRecord;
}

bool bbppGeoCacheRecord::isUndoable() const
{
    return true;
}

MStatus bbppGeoCacheRecord::undoIt()
{
    return MS::kSuccess;
}

MSyntax bbppGeoCacheRecord::initSyntax()
{
  MSyntax syntax;

  syntax.addFlag("n","objectName", MSyntax::kString);		// Name of the polymesh you want to cache
  syntax.addFlag("f", "folder", MSyntax::kString);			// Cache File Name
  syntax.addFlag("sf", "startFrame", MSyntax::kLong);		// Cache Start Frame
  syntax.addFlag("ef", "endFrame", MSyntax::kLong);			// Cache End Frame
  syntax.addFlag("c", "coord", MSyntax::kBoolean);			// Cache In Local(false) or Global(true) Coordinates
  syntax.addFlag("at", "advanceTime", MSyntax::kBoolean);	// Advance Timeline during record(preventing bad rig update)

  return syntax;
}

MStatus bbppGeoCacheRecord::doIt( const MArgList& args )
{
	MString objectName;
	int startFrame = 1;
	int endFrame = 100;
	MString folder;
	bool coord;
	bool advanceTime;

	MSpace::Space transformSpace = MSpace::kWorld;

	MStatus status;
	MArgDatabase argDB(initSyntax(), args, &status);
	if(status != MS::kSuccess)return status;

	if(argDB.isFlagSet("objectName"))
		argDB.getFlagArgument("objectName",0,objectName);

	if(argDB.isFlagSet("folder"))
		argDB.getFlagArgument("folder", 0, folder);

	if(argDB.isFlagSet("startFrame"))
		argDB.getFlagArgument("startFrame",0,startFrame);

	if(argDB.isFlagSet("endFrame"))
		argDB.getFlagArgument("endFrame",0,endFrame);

	if(argDB.isFlagSet("coord"))
		argDB.getFlagArgument("coord",0,coord);

	if(argDB.isFlagSet("advanceTime"))
		argDB.getFlagArgument("advanceTime",0,advanceTime);

	MObject     component;
	MDagPath    dagPath;

	MSelectionList list;

	if (! list.add( objectName ) ) 
	{
		objectName += ": no such object";
		displayError(objectName);
		return MS::kFailure; // no such object
	}

	MItSelectionList iter( list );
	iter.getDagPath( dagPath, component );
	dagPath.extendToShape();

	MStringArray objectNameArray;
	objectName.split(':',objectNameArray);

	MString objectShortName = objectNameArray[objectNameArray.length()-1];
	MGlobal::displayInfo("shortName : "+objectShortName);

	// Build Out File Name
	MString fileName = folder+"\\"+objectShortName+".bgc";

	MString info("Writing point cache to file: ");
	info += fileName;
	displayInfo( info );

	bbppGeoCache Writer;
	int timelapse = endFrame - startFrame +1;

	Writer.SetStartFrame(startFrame);
	Writer.SetEndFrame(endFrame);
	Writer.SetTimelapse(timelapse);

	Writer.CreateOffset();

	MString title = "bbppGeoCacheRecord";
	MString progressWindowState = MString("Writing GeoCache for :");	
	progressWindowState += objectName;

	MProgressWindow::reserve();
	CHECK_MSTATUS(MProgressWindow::setProgressRange(0, timelapse));
	CHECK_MSTATUS(MProgressWindow::setTitle(title));
	CHECK_MSTATUS(MProgressWindow::setInterruptable(true));
	CHECK_MSTATUS(MProgressWindow::startProgress());
	
	int frameid = 0;	
	int offset = SIZE_SIGNATURE + SIZE_INT*(2+timelapse);

	MTime cTime;

	MPlug plugMesh;
	MObject meshData;

	MFnDependencyNode dependNode( dagPath.node(), &status );
	// Get the .outMesh plug for this mesh
	plugMesh = dependNode.findPlug( MString( "outMesh" ), &status );

	for(int a=startFrame;a<endFrame+1;a++,frameid++)
	{
		if (MProgressWindow::isCancelled()) 
		{
			MGlobal::displayInfo("bbppGeoCacheRecord canceled by the user!");
			Writer.Delete();
			return MStatus::kSuccess;
		}

		MFnMesh fnMesh;
		cTime.setValue(a);
		MGlobal::viewFrame( cTime );

		Writer.SetOffset(frameid,offset);
		/*
		cTime.setValue(a);
		if(advanceTime)
		{
			// Move Maya to current frame
			MGlobal::viewFrame( cTime );
		}
		// Get its value at the specified Time.
		//status = plugMesh.getValue( meshData, MDGContext( cTime ) );

		// Get the MFnMesh
		//MFnMesh fnMesh(meshData,&status);
		*/

		fnMesh.setObject( dagPath );

		// Get Actual Geometry Data
		MPointArray points;
		fnMesh.getPoints(points,transformSpace);

		int nbv = points.length();
		int nbf = fnMesh.numPolygons();

		MIntArray polyVerts;

		int nbfv = 0;
		for(int i=0;i<nbf;i++)
		{
			fnMesh.getPolygonVertices(i,polyVerts);
			nbfv += polyVerts.length();
		}

		bbppGeoFrame* frame = new bbppGeoFrame(nbv,nbf,nbfv);

		// Write out the polygons indices table
		int incr = 0;
		for ( int i=0; i<nbf; i++ )
		{
			fnMesh.getPolygonVertices( i, polyVerts );
			int pvc = polyVerts.length();
			frame->_fnbv[i] = pvc;
			for ( int v=0; v<pvc; v++ ) 
			{
				frame->_fv[incr+v] = polyVerts[v];
			}
			incr += pvc;
		}
		
		// Write out the vertex table
		for(int i=0;i<points.length();i++)
		{
			frame->_pos[i*3] = points[i].x;
			frame->_pos[i*3+1] = points[i].y;
			frame->_pos[i*3+2] = points[i].z;
		}

		Writer.AddFrame(frame);
		offset += frame->_size;

		CHECK_MSTATUS(MProgressWindow::setProgressStatus(progressWindowState));
		CHECK_MSTATUS(MProgressWindow::advanceProgress(1));
	}

	CHECK_MSTATUS(MProgressWindow::endProgress());
	Writer.Write(fileName.asChar());
	Writer.Delete();

	return status;
}

MStatus bbppGeoCacheRecord::redoIt()
{
    return MS::kSuccess;
}
