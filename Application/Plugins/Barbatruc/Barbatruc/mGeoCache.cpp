#include "mRegister.h"

using namespace std;

const MTypeId bbppGeoCacheReader::id(0*00000002);
const MString bbppGeoCacheReader::typeName("bbppGeoCacheReader");

MObject bbppGeoCacheReader::currenttime;
MObject bbppGeoCacheReader::timeoffset;
MObject bbppGeoCacheReader::file;
MObject bbppGeoCacheReader::outmesh;
bbppGeoCache* bbppGeoCacheReader::reader = NULL;

void* bbppGeoCacheReader::creator(){ return new bbppGeoCacheReader;}

bool FileExists(MString filename);

MStatus bbppGeoCacheReader::initialize()
{
	MFnNumericAttribute nAttr;
	MFnTypedAttribute tAttr;

	outmesh = tAttr.create("outMesh","out",MFnData::kMesh);
	nAttr.setWritable(false);
	nAttr.setStorable(false);

	currenttime = nAttr.create("currentTime","ct",MFnNumericData::kFloat);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setHidden(true);

	timeoffset = nAttr.create("timeOffset","to",MFnNumericData::kInt);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);

	file = tAttr.create("file","f",MFnData::kString);
	tAttr.setStorable(true);

	addAttribute(outmesh);
	addAttribute(currenttime);
	addAttribute(timeoffset);
	addAttribute(file);

	attributeAffects(currenttime,outmesh);
	attributeAffects(timeoffset,outmesh);
	attributeAffects(file,outmesh);

	reader = new bbppGeoCache();

	return MS::kSuccess;
}

MStatus bbppGeoCacheReader::compute(const MPlug& plug, MDataBlock& data)
{
	MStatus stat;
	if(plug != outmesh)return MS::kUnknownParameter;

	MDataHandle hInput;
	hInput = data.inputValue(currenttime);
	int currentframe = (int)hInput.asFloat();

	hInput = data.inputValue(timeoffset);
	int timeoffset = hInput.asInt();

	hInput = data.inputValue(file);
	MString filename = hInput.asString();
	bool fExist = FileExists(filename);

	if(fExist)
	{
		if(reader->_filename != filename.asChar() || !reader->_file.is_open())
		{
			bool isopen = reader->OpenFile(filename.asChar());
			if(!isopen)return MS::kInvalidParameter;

			bool isvalid = reader->GetHeader();
			if(!isvalid)return MS::kInvalidParameter;
		}

		currentframe +=timeoffset;
		currentframe -= reader->_startframe;
		reader->ReadFrame(currentframe);

		MPointArray vertices;
		MVectorArray normals;
		MIntArray faceverticescount;
		MIntArray facevertices;
		
		for(int a=0;a<reader->_nbv;a++)
		{
			vertices.append(MPoint(reader->_vertices[a*3],reader->_vertices[a*3+1],reader->_vertices[a*3+2]));
		}

		int fverticessum = 0;
		for(int a=0;a<reader->_nbf;a++)
		{
			fverticessum+=reader->_faceverticescount[a];
			faceverticescount.append(reader->_faceverticescount[a]);
		}

		for(int a=0;a<fverticessum;a++)
		{
			facevertices.append(reader->_facevertices[a]);
		}

		// get outMesh
		MDataHandle outputValueDataHandle = data.outputValue( outmesh );
		
		MFnMeshData outputDataCreator;
		MObject newOutputMeshData = outputDataCreator.create(&stat);

		MFnMesh mesh = newOutputMeshData;
		mesh.create(reader->_nbv, reader->_nbf, vertices, faceverticescount, facevertices, newOutputMeshData,&stat);

		// set outMesh
		outputValueDataHandle.set ( newOutputMeshData );
		data.setClean(plug);

		return MS::kSuccess;
	}
	else
	{
		return MS::kInvalidParameter;
	}
}

bool FileExists(MString filename)
{
	struct stat stFileInfo;

	int intStat;

	// Attempt to get the file attributes
	intStat = stat(filename.asChar(),&stFileInfo);
	if(intStat == 0) return true;
	else
	{
		MString message = "The file : "+ filename +" DOESN'T exists !!";
		MGlobal::displayInfo(message);
	}
	return false;
}