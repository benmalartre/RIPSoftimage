// mPointCache.cpp
//-------------------------------------
#include "mRegister.h"

MTypeId     bbppPointCacheReader::id( 0x0F77F );

////////////////////////
//	bbppPointCacheReader attributes  //
////////////////////////
MObject     bbppPointCacheReader::file;
MObject     bbppPointCacheReader::startFrame;
MObject     bbppPointCacheReader::endFrame;
MObject		bbppPointCacheReader::currentTime;
MObject     bbppPointCacheReader::timeOffset;
MObject     bbppPointCacheReader::timeWarp;
MObject     bbppPointCacheReader::customFrame;
MObject     bbppPointCacheReader::useCustomPlayback;
MObject     bbppPointCacheReader::interpolation;
MObject     bbppPointCacheReader::hermiteTension;
MObject     bbppPointCacheReader::hermiteBias;

bbppPointCache * bbppPointCacheReader::reader = NULL;


bbppPointCacheReader::bbppPointCacheReader() 
//
//	Description:
//		constructor
//
{ 
}

bbppPointCacheReader::~bbppPointCacheReader()
//
//	Description:
//		destructor
//
{
	if(reader!=NULL)delete reader;
}

void* bbppPointCacheReader::creator()
//
//	Description:
//		create the bbppPointCacheReader
//
{
	return new bbppPointCacheReader();
}

MStatus bbppPointCacheReader::initialize()
{
	MStatus	stat;
	
	MFnNumericAttribute nAttr;
	MFnTypedAttribute tAttr;
	MFnEnumAttribute eAttr;

	file=tAttr.create( "file", "fle", MFnData::kString );
	tAttr.setStorable(true);

	startFrame = nAttr.create( "startFrame", "sf", MFnNumericData::kInt, 0);
	nAttr.setStorable(false);
	nAttr.setKeyable(false);

	endFrame = nAttr.create( "endFrame", "ef", MFnNumericData::kInt, 0);
	nAttr.setStorable(false);
	nAttr.setKeyable(false);

	currentTime = nAttr.create("currentTime","ct",MFnNumericData::kFloat);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	//nAttr.setHidden(true);
	
	timeOffset = nAttr.create( "timeOffset", "off", MFnNumericData::kFloat, 0.0);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	
	timeWarp=nAttr.create( "timeWarp", "twp", MFnNumericData::kFloat, 1.0 );
	nAttr.setStorable(true);
	nAttr.setKeyable(true);	

	customFrame = nAttr.create( "customFrame", "cfr", MFnNumericData::kFloat, 1.0 );
	nAttr.setStorable(true);
	nAttr.setKeyable(true);	

	useCustomPlayback=nAttr.create( "useCustomPlayback", "cpb", MFnNumericData::kBoolean, 0 );
	nAttr.setStorable(true);
	nAttr.setKeyable(true); 
		
	interpolation = eAttr.create( "interpolation", "ipl", 0 );  
	eAttr.setStorable(true);
	eAttr.addField("linear", 0) ;	
	eAttr.addField("cubic", 1) ;
	eAttr.addField("hermite", 2) ;		

	hermiteTension=nAttr.create( "hermiteTension", "htn", MFnNumericData::kFloat, 0.0 );
	nAttr.setMax(1.0);
	nAttr.setMin(-1.0);
	nAttr.setStorable(true);
	nAttr.setKeyable(true); 

	hermiteBias=nAttr.create( "hermiteBias", "hbs", MFnNumericData::kFloat, 0.0 );
	nAttr.setMax(1.0);
	nAttr.setMin(-1.0);
	nAttr.setStorable(true);
	nAttr.setKeyable(true); 
	
	reader = new bbppPointCache();

	addAttribute( file );
	addAttribute( startFrame );
	addAttribute( endFrame );
	addAttribute( currentTime );
	addAttribute( timeOffset );
	addAttribute( timeWarp );
	addAttribute( useCustomPlayback );
	addAttribute( customFrame );
	addAttribute( interpolation );
	addAttribute( hermiteTension );
	addAttribute( hermiteBias );
		
	// affects
	attributeAffects( file, bbppPointCacheReader::outputGeom );
	attributeAffects( currentTime, bbppPointCacheReader::outputGeom );
	attributeAffects( timeOffset, bbppPointCacheReader::outputGeom );
	attributeAffects( timeWarp, bbppPointCacheReader::outputGeom );
	attributeAffects( useCustomPlayback, bbppPointCacheReader::outputGeom );
	attributeAffects( customFrame, bbppPointCacheReader::outputGeom );
	attributeAffects( interpolation, bbppPointCacheReader::outputGeom );
	attributeAffects( hermiteTension, bbppPointCacheReader::outputGeom );
	attributeAffects( hermiteBias, bbppPointCacheReader::outputGeom );

	return MS::kSuccess;
}

MStatus
bbppPointCacheReader::deform( MDataBlock& block,
				MItGeometry& iter,
				const MMatrix& /*m*/,
				unsigned int /*multiIndex*/)

{
	MGlobal::displayInfo("PointCacheReader Update...");

	MStatus status = MS::kSuccess;
	MDataHandle data = block.inputValue(file,&status);
	McheckErr(status, "Error getting file data handle\n");
	MString cacheFileV = data.asString();
	const char* cacheFileC = cacheFileV.asChar();
	MGlobal::displayInfo(cacheFileV);

	data = block.inputValue(currentTime,&status);
	McheckErr(status, "Error getting current time data handle\n");
	float currentTimeV = data.asFloat();
	MString s;
	s+= currentTimeV;
	MGlobal::displayInfo(s);

	data = block.inputValue(timeOffset,&status);
	McheckErr(status, "Error getting time offset data handle\n");
	float timeOffsetV = data.asFloat();

	data = block.inputValue(timeWarp,&status);
	McheckErr(status, "Error getting timeWarp data handle\n");
	float timeWarpV = data.asFloat();

	data = block.inputValue(useCustomPlayback,&status);
	McheckErr(status, "Error getting useCustomPlayback data handle\n");
	int useCustomPlaybackVAsInt = data.asInt();
	bool useCustomPlaybackV = (useCustomPlaybackVAsInt == 1);

	data = block.inputValue(customFrame,&status);
	McheckErr(status, "Error getting customFrame data handle\n");
	float customFrameV = data.asFloat();

	data = block.inputValue(interpolation,&status);
	McheckErr(status, "Error getting interpolation data handle\n");
	long interpolationV = data.asLong();

	data = block.inputValue(hermiteTension,&status);
	McheckErr(status, "Error getting hermiteTension data handle\n");
	float hermiteTensionV = data.asFloat();

	data = block.inputValue(hermiteBias,&status);
	McheckErr(status, "Error getting hermiteBias data handle\n");
	float hermiteBiasV = data.asFloat();

	MAnimControl animControl;
	MTime T = animControl.currentTime();
	
	int nbP = iter.count();

	// Initialize Pointers
	if(reader->_position == NULL)reader->CreatePointers(nbP);

	if(cacheFileV.length()>0 && (!reader->_s.is_open() || cacheFileC == reader->_file))
	{
		if(!reader->Init(cacheFileC,nbP))
		{
			MGlobal::displayInfo("Error Initializing...");
			return MStatus::kInvalidParameter;
		}
		data = block.inputValue(startFrame,&status);
		McheckErr(status, "Error getting startframe data handle\n");
		data.set(reader->_startFrame);

		data = block.inputValue(endFrame,&status);
		McheckErr(status, "Error getting endframe data handle\n");
		data.set(reader->_endFrame);
	
	}

	MString info = "Cache File Name ---> ";
	info+=cacheFileV;
	info+="\n";

	info += "Cache File Is Open ---> ";
	info+=reader->_s.is_open();
	info+="\n";

	info += "Compare File Name ---> ";
	info+=cacheFileC;
	info+=",";
	info+=reader->_file;
	info+=cacheFileC == reader->_file;
	info+="\n";

	MGlobal::displayInfo(info);

	if(cacheFileV.length()>0  && reader->_s.is_open()/*&& cacheFileC == reader->_file*/)
	{
		if(nbP == reader->_nbPoints)
		{
			if(timeWarpV == 0)timeWarpV = 1;

			float t = float(T.value());
			float epsilon = 0.01;
			if(t<0)epsilon = -0.01;
			float currentFrame;
			if(useCustomPlaybackV == true)currentFrame = customFrameV;
			else currentFrame = (t-timeOffsetV-reader->_startFrame)*timeWarpV;

			MString info = "Current Frame ---> ";
			info+=currentFrame;
			MGlobal::displayInfo(info);

			if(reader->isSubFrame(currentFrame,(long)(currentFrame+epsilon)))
			{
				long previous = (long)(currentFrame);
				long next = previous + 1;
				long p = (long)(currentFrame);
				float interpolate = (float)fabs(p - currentFrame);
				
				if(previous<0)reader->ReadFrame(0);

				else if(next>(reader->_endFrame - reader->_startFrame))
				{
					reader->ReadFrame(reader->_endFrame - reader->_startFrame);
				}

				else
				{
					if(previous == 0 ||next == reader->_endFrame - reader->_startFrame)
					{
						interpolationV = 0;
					}
						
					reader->ReadSubFrame(previous,next,interpolate,interpolationV,hermiteTensionV,hermiteBiasV);
				}
			}
			else
			{
				long ct;
				if(t>=0)ct = (long)(currentFrame + epsilon);
				else ct = (long)(currentFrame + 1 + epsilon);

				if(ct<0)
				{
					reader->ReadFrame(0);
				}

				else if(ct>(reader->_endFrame - reader->_startFrame))
				{
					reader->ReadFrame(reader->_endFrame - reader->_startFrame);
				}

				else
				{
					reader->ReadFrame(ct);

				}	
			}

			// write target array to geometry iterator
			iter.reset();
			long index = 0;
			for ( ; !iter.isDone(); iter.next()) 
			{
				MPoint resultPoint = iter.position();
				resultPoint.x = reader->_position[index*3];
				resultPoint.y = reader->_position[index*3+1];
				resultPoint.z = reader->_position[index*3+2];
				iter.setPosition(resultPoint);
				index++;
			}

			return MStatus::kSuccess;
		}
	}

	return status;
}

#define MATCH(str, shortName, longName) \
		(((str)==(shortName))||((str)==(longName)))


bbppPointCacheRecord::bbppPointCacheRecord() :
    point(0.0, 0.0, 0.0)
{
}

bbppPointCacheRecord::~bbppPointCacheRecord() {}

void* bbppPointCacheRecord::creator()
{
    return new bbppPointCacheRecord;
}

bool bbppPointCacheRecord::isUndoable() const
{
    return true;
}

MStatus bbppPointCacheRecord::undoIt()
{
    return MS::kSuccess;
}

MStatus bbppPointCacheRecord::doIt( const MArgList& args )
{
	/*
    MString componentName;
	MString filename;

	double		startFrame;
	double		endFrame;
	double		samplingRate = 1.0;
	bool		worldSpace = true;
	bool		prefixNamespace = false;
	MString		nsDelimeter = ".";
	int			multipleObjects = -2;

	MSpace::Space transformSpace = MSpace::kWorld;

	for (unsigned int i = 0; i < args.length (); i++)
	{
		MString argStr;
		args.get (i, argStr);
		if (MATCH(argStr, "-f", "-filename"))
			stringArg(args, i, filename);
		if (MATCH(argStr, "-s", "-start"))
			doubleArg(args, i, startFrame);
		else if (MATCH(argStr, "-e", "-end"))
			doubleArg(args, i, endFrame);
		else if (MATCH(argStr, "-r", "-rate"))
			doubleArg(args, i, samplingRate);
		else if (MATCH(argStr, "-ws", "-worldSpace"))
			boolArg(args, i, worldSpace);
		else if (MATCH(argStr, "-mo", "-multipleObjects"))
			intArg(args, i, multipleObjects);
		else if (MATCH(argStr, "-ns", "-prefixNamespace"))
			boolArg(args, i, prefixNamespace);
		else if (MATCH(argStr, "-d", "-delimeter"))
			stringArg(args, i, nsDelimeter);
		else
			componentName = argStr;
	}

	if ( multipleObjects != -2 ) // handle several objects
	{
		int chunk = multipleObjects;
		MDagPath            node; 
		MObject             component; 
		MSelectionList      list; 
		MFnDagNode          nodeFn; 
		MGlobal::getActiveSelectionList( list );
		long index = 0;
		long numSamples = (long) ceil(((endFrame - startFrame) / samplingRate) + 1);
		int length = 500;
		std::vector< PointCacheCore* > cacheCores; 

		if ( chunk == 0 )
			chunk = list.length();
		for (int i=0; i < (int)list.length(); i++ )
		{
			cacheCores.push_back( new PointCacheCore() );
		}
		while ( index < (int) list.length() )
		{
		
			if ( chunk > ( (int) list.length() - index ) )
				chunk = list.length() - index;
		
			// open files
			for ( int i=0; i < chunk; i++ )
			{	
				list.getDagPath( index + i, node, component );
				
				PointCacheCore* pcCore = cacheCores[index + i];
				nodeFn.setObject( node ); 
				
				MItMeshVertex vertexCountIter( node );
				
				char str[500];
				strcpy (str, filename.asChar() );

				//generate filename from object name

				MStringArray arr;

				nodeFn.name().split(':', arr);

				if ( arr.length() == 1 )
					strcat ( str, nodeFn.name().asChar() );
				else
				{
					if ( prefixNamespace == true )
					{
						for ( int si = 0; si < arr.length()-1; si ++ )
						{
							strcat ( str, arr[si].asChar() );
							strcat ( str, nsDelimeter.asChar() );
						}
					}
					strcat ( str, arr[arr.length()-1].asChar() );
				}

				strcat(str, ".pc2");

                pcCore->WriteCache(str, vertexCountIter.count(), static_cast<float> (startFrame), static_cast<float> (samplingRate), numSamples, 1);
			}

			// write frame data
			double currentFrame = startFrame;
			for ( long sampleIndex = 0; sampleIndex < numSamples; sampleIndex++ )
			{
				
				MGlobal::viewFrame (currentFrame);		
				for ( int i=0; i < chunk; i++ )
				{
					
					PointCacheCore* core = cacheCores[index + i];

					list.getDagPath( index + i, node, component );
					MItMeshVertex vertexIter( node);
					if ( worldSpace )
					while(!vertexIter.isDone())
					{
						point = vertexIter.position(MSpace::kWorld); //transformSpace);
						double x = point.x;
						double y = point.z * -1.0;
						double z = point.y;
						core->Write(x);
						core->Write(y);
						core->Write(z);
						vertexIter.next();
					}
					else
					while(!vertexIter.isDone())
					{
						point = vertexIter.position(MSpace::kObject);
						double x = point.x;
						double y = point.z * -1.0;
						double z = point.y;
						core->Write(x);
						core->Write(y);
						core->Write(z);
						vertexIter.next();
					}
				}
				currentFrame += samplingRate;
			}
			// close files and delete cores
			for ( int i=0; i < chunk; i++ )
			{
				PointCacheCore* core = cacheCores[index + i];
				core->Close();
				delete core;
				cacheCores[index + i] = NULL;
			}
			index += chunk;
		}
	}
	else
	{// handle single object

		MObject     component;
		MDagPath    dagPath;

		if (!componentName.length ()) {
			displayError ("No object selected");
				return MS::kFailure;
		} else {
			MSelectionList list;

			if (! list.add( componentName ) ) {
				componentName += ": no such object";
				displayError(componentName);
				return MS::kFailure; // no such component
			}
			MItSelectionList iter( list );
			iter.getDagPath( dagPath, component );
		}

		MString info("writing point cache to file: ");
		info += filename;
		displayInfo( info );
		
		PointCacheCore pcCore;

		long numSamples = (long) ((endFrame - startFrame) / samplingRate) + 1;
		MItMeshVertex vertexCountIter(dagPath);
		int length = 500;

		pcCore.WriteCache(filename.asChar(), vertexCountIter.count(), static_cast<float>(startFrame), static_cast<float>(samplingRate), numSamples, 1);

		double currentFrame = startFrame;
		for ( long sampleIndex = 0; sampleIndex < numSamples; sampleIndex ++ )
		{
			MGlobal::viewFrame (currentFrame);		
			MItMeshVertex vertexIter( dagPath);
			
			if ( worldSpace )
			while(!vertexIter.isDone())
			{
				point = vertexIter.position(MSpace::kWorld);//transformSpace);
				double x = point.x;
				double y = point.z * -1.0;
				double z = point.y;
				pcCore.Write(x);
				pcCore.Write(y);
				pcCore.Write(z);
				vertexIter.next();
			}
			else
			while(!vertexIter.isDone())
			{
				point = vertexIter.position(MSpace::kObject);
				double x = point.x;
				double y = point.z * -1.0;
				double z = point.y;
				pcCore.Write(x);
				pcCore.Write(y);
				pcCore.Write(z);
				vertexIter.next();
			}
			currentFrame += samplingRate;
		}
		pcCore.Close();
		return redoIt();
	}
	return redoIt();
	*/
return MS::kSuccess;
}

MStatus bbppPointCacheRecord::redoIt()
{
    return MS::kSuccess;
}