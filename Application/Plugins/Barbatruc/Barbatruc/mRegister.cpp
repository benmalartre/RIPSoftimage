// bbppRegister.cpp
#include "mRegister.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj,"benmalartre","1.0","Any");
	status = plugin.registerNode("bbppGeoCacheReader",bbppGeoCacheReader::id,bbppGeoCacheReader::creator,bbppGeoCacheReader::initialize);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	status = plugin.registerCommand( "bbppGeoCacheRecord", bbppGeoCacheRecord::creator );
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj);

	status = plugin.deregisterNode(bbppGeoCacheReader::id);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	status = plugin.deregisterCommand( "bbppGeoCacheRecord" );
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}