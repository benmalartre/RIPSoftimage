#include "STL_File.h"
#include <sys\stat.h>

//#include <xsi_application.h>
//#include <xsi_string.h>

//------------------------------------------
//		Writer
//------------------------------------------
STL_File::STL_File()
{
	//_frames = new std::vector<STL_GeoFrame*>;
}

bool STL_File::Write(const char *filename,bool binary)
{
	if(binary){
		_file.open(filename, ios::out|ios::binary|ios::trunc);
		if(!_file.is_open())return false;

		_file.write((char*) &_header, SIZE_HEADER_BINARY*sizeof(char));
		_file.write((char*) &_nbt,sizeof(long));
		char abc[2];
		for(long i=0;i<_nbt;i++){
			_file.write((char*) &_normals[i],sizeof(STL_Vec3));
			_file.write((char*) &_vertices[i*3],sizeof(STL_Vec3));
			_file.write((char*) &_vertices[i*3+1],sizeof(STL_Vec3));
			_file.write((char*) &_vertices[i*3+2],sizeof(STL_Vec3));
			_file.write((char*) &abc,sizeof(char[2]));
		}
	}
	else{

	}
	//_file.write((char*) &_startframe, SIZE_INT);
	//_file.write((char*) &_endframe, SIZE_INT);
	//_file.write((char*) _offsets,_timelapse * SIZE_INT);

	/*
	for(int a=0;a<_timelapse;a++)
	{
		int nbv = (*_frames)[a]->_nbv;
		int nbf = (*_frames)[a]->_nbf;
		_file.write((char*)&nbv, SIZE_INT);
		_file.write((char*)&nbf, SIZE_INT);
		_file.write((char*)(*_frames)[a]->_pos, 3*SIZE_FLOAT*nbv);
		_file.write((char*)(*_frames)[a]->_fnbv, SIZE_INT*nbf);
		_file.write((char*)(*_frames)[a]->_fv, SIZE_INT*(*_frames)[a]->_fvc);
	}
	*/
	_file.close();

	return true;
}


void STL_File::Delete()
{
	if(_file.is_open())_file.close();

}

inline void StringToSTLVec3(std::string& s,STL_Vec3& v){
	std::istringstream buf(s);
	std::istream_iterator<std::string> beg(buf),end;
	std::vector<std::string> tokens(beg,end);
	v.x = (float)atof(tokens[0].c_str());
	v.y = (float)atof(tokens[1].c_str());
	v.z = (float)atof(tokens[2].c_str());
}

//------------------------------------------
//		Reader
//------------------------------------------
bool STL_File::Open(const char* filename)
{
	//_filename = NULL;
	if(_file.is_open())_file.close();
	_file.open(filename, ios::in|ios::binary|ios::beg);
	//_file.open(filename, ios::in|ios::beg);
	if(!_file.is_open())return false;

	_filename = filename;
	return true;
}

int STL_File::GetHeader()
{
	_file.seekg(0,ios::beg);
	// first check for ascii signature
	//_file.read((char*) &_signature,SIZE_SIGNATURE_ASCII);
	char s[SIZE_LINE];
	_file.getline(s,SIZE_LINE);
	const char* f = strstr(s,"solid ");
	if(f!=NULL){
		_binary = false;
		return true;
	}
	else{
		_file.seekg(0,ios::beg);
		_file.read((char*) &_header,SIZE_HEADER_BINARY);
		_file.read((char*) &_nbt,sizeof(long));
		_binary = true;
		return true;
	}
	return false;
}


bool STL_File::ReadAsciiTriangle()
{
	/* Exemple Ascii Facet
	#
	# INFO : facet normal 0.39073698910498 0.355524531416872 0.849074150416787
	# INFO : outer loop
	# INFO : vertex 12.647682094574 -39.5271588261135 35.0569501730396
	# INFO : vertex 12.4257948994637 -30.6554037282805 31.4442774306669
	# INFO : vertex 8.6591204404831 -38.9287309094474 36.6418797887381
	# INFO : endloop
	# INFO : endfacet
	#
	*/
	if(_file.eof())return false;
	std::string t;
	std::string rm;
	std::string::size_type i;

	char s[SIZE_LINE];

	// normal
	_file.getline(s,SIZE_LINE);
	if(strstr(s,"facet normal ")!=NULL){
		t = s;
		rm = "facet normal ";
		i=t.find(rm);
		if(i!=std::string::npos)
			t.erase(i,rm.length());
		STL_Vec3 n;
		StringToSTLVec3(t,n);
		_normals.push_back(n);
	}

	// begin outer loop
	_file.getline(s,SIZE_LINE);
	if(strstr(s,"outer loop")==NULL)return false;

	// vertex 1
	_file.getline(s,SIZE_LINE);
	if(strstr(s,"vertex ")!=NULL){
		t = s;
		rm = "vertex ";
		i=t.find(rm);
		if(i!=std::string::npos)
			t.erase(i,rm.length());

		STL_Vec3 v;
		StringToSTLVec3(t,v);
		_vertices.push_back(v);
		//XSI::Application().LogMessage(L"P1 : "+(XSI::CString)v.x+L","+(XSI::CString)v.y+L","+(XSI::CString)v.z);
		/*
		XSI::Application().LogMessage(t.c_str());
		XSI::Application().LogMessage(L"Normal : "+(XSI::CString)n.x+L","+(XSI::CString)n.y+L","+(XSI::CString)n.z);
		*/
	}

	// vertex 2
	_file.getline(s,SIZE_LINE);
	if(strstr(s,"vertex ")!=NULL){
		t = s;
		rm = "vertex ";
		i=t.find(rm);
		if(i!=std::string::npos)
			t.erase(i,rm.length());

		STL_Vec3 v;
		StringToSTLVec3(t,v);
		_vertices.push_back(v);
		//XSI::Application().LogMessage(L"P2 : "+(XSI::CString)v.x+L","+(XSI::CString)v.y+L","+(XSI::CString)v.z);
		//XSI::Application().LogMessage(t.c_str());
	}
	
	// vertex 3
	_file.getline(s,SIZE_LINE);
	if(strstr(s,"vertex ")!=NULL){
		t = s;
		rm = "vertex ";
		i=t.find(rm);
		if(i!=std::string::npos)
			t.erase(i,rm.length());

		STL_Vec3 v;
		StringToSTLVec3(t,v);
		_vertices.push_back(v);
		//XSI::Application().LogMessage(L"P3 : "+(XSI::CString)v.x+L","+(XSI::CString)v.y+L","+(XSI::CString)v.z);

		//XSI::Application().LogMessage(t.c_str());
	}

	// end loop
	_file.getline(s,SIZE_LINE);
	if(strstr(s,"endloop")==NULL)return false;

	// end facet
	_file.getline(s,SIZE_LINE);
	if(strstr(s,"endfacet")==NULL)return false;

	this->_nbt++;

	return true;
}

void STL_File::Read()
{
	if(_binary){
		_file.seekg(SIZE_HEADER_BINARY*sizeof(char)+sizeof(long),ios::beg);

		_normals.resize(this->_nbt);
		_vertices.resize(this->_nbt*3);
		
		char abc[2];
		for(long i=0;i<this->_nbt;i++){
			_file.read((char*)&_normals[i],sizeof(STL_Vec3));
			_file.read((char*)&_vertices[i*3+0],sizeof(STL_Vec3));
			_file.read((char*)&_vertices[i*3+1],sizeof(STL_Vec3));
			_file.read((char*)&_vertices[i*3+2],sizeof(STL_Vec3));
			_file.read((char*)&abc,sizeof(char[2]));
		}
	}
	else{
		_normals.clear();
		_vertices.clear();
		_nbt = 0;
		
		while(ReadAsciiTriangle()){}
	}

	/*
	_file.read((char*)&_nbv,sizeof(int));
	_file.read((char*)&_nbf,sizeof(int));

	// position data
	float* pos = new float[3*_nbv];
	_file.read((char*)pos,3*_nbv*sizeof(float));

	_vertices.clear();

	for(int a=0;a<_nbv;a++)
	{
		_vertices.push_back(pos[a*3]);
		_vertices.push_back(pos[a*3+1]);
		_vertices.push_back(pos[a*3+2]);
	}

	int* faceverticescount = new int[_nbf];
	_file.read((char*)faceverticescount,_nbf*sizeof(int));

	_faceverticescount.clear();
	int fverticessum = 0;
	for(int a=0;a<_nbf;a++)
	{
		fverticessum+=faceverticescount[a];
		_faceverticescount.push_back(faceverticescount[a]);
	}

	_facevertices.clear();
	int* facevertices = new int[fverticessum];
	_file.read((char*)facevertices,fverticessum*sizeof(int));

	for(int a=0;a<fverticessum;a++)
	{
		_facevertices.push_back(facevertices[a]);
	}

	delete[] pos;
	delete[] faceverticescount;
	delete[] facevertices;
	*/
}



//------------------------------------------
//		Utilities
//------------------------------------------
bool FileExists(const char* inFilename) 
{
	struct stat stFileInfo;
	bool blnReturn = false;
	int intStat;

	// Attempt to get the file attributes
	intStat = stat(inFilename,&stFileInfo);
	if(intStat == 0) 
	{
	  // the file exist
	  return true;
	} 
	return(false);
}
