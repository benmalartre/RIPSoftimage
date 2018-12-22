// bbppRegister.h
#ifndef _bbppRegister
#define _bbppRegister

#include <iostream>
#include <sys/stat.h>						// required by stat, mkdir
#include <math.h>
#include <string.h>

#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MStatus.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MPxNode.h>
#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MIOStream.h>
#include <maya/MItGeometry.h>
#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MFnData.h>
#include <maya/MFnStringData.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MAnimControl.h>
#include <maya/MTime.h>
#include <maya/MPoint.h>
#include <maya/MMatrix.h>
#include <maya/MArgList.h>
#include <maya/MObject.h>
#include <maya/MDagPath.h>
#include <maya/MItSelectionList.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshEdge.h>
#include <maya/MSelectionList.h>
#include <maya/MString.h>
#include <maya/MPoint.h>
#include <maya/MItCurveCV.h>
#include <maya/MItSurfaceCV.h>
#include <maya/MPointArray.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MProgressWindow.h>

#include "bbppGeoCache.h"

#define PI 3.14159265358979323846

#define McheckErr(stat,msg)		\
	if ( MS::kSuccess != stat ) {	\
		cerr << msg;				\
		return MS::kFailure;		\
	}


struct CVector3
{
	double x;
	double y;
	double z;
};

//////////////////////////////////////////////////////////////////////////////
//
// bbppGeoCacheReader class
//
//////////////////////////////////////////////////////////////////////////////
class bbppGeoCacheReader : public MPxNode
{
public:
	bbppGeoCacheReader(){};
	virtual MStatus compute(const MPlug& plug, MDataBlock& data);
	static void* creator();
	static MStatus initialize();

	static const MTypeId id;
	static const MString typeName;

	static MObject currenttime;
	static MObject timeoffset;
	static MObject file;
	static MObject outmesh;

	static bbppGeoCache* reader;
};

//////////////////////////////////////////////////////////////////////////////
//
// bbppGeoCacheRecord class
//
//////////////////////////////////////////////////////////////////////////////
class bbppGeoCacheRecord : public MPxCommand
{
public:
                bbppGeoCacheRecord();
    virtual     ~bbppGeoCacheRecord();

    MStatus     doIt ( const MArgList& args );
    MStatus     redoIt ();
    MStatus     undoIt ();
    bool        isUndoable() const;

    static      void* creator();

	MSyntax initSyntax();
};

#endif