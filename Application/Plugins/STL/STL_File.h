// STL_File.h
//-------------------------------------
#ifndef _STL_File_
#define _STL_File_

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

#define SIZE_FLOAT 4
#define SIZE_INT 4
#define SIZE_SIGNATURE_ASCII 6
#define SIZE_HEADER_BINARY 80

#define SIZE_LINE 512

struct STL_Vec3{
	float x;
	float y;
	float z;
};


class STL_File
{
public:
	STL_File();
	~STL_File(){};

	//Reader
	bool Open(const char* filename);
	int GetHeader();
	void Read();
	bool ReadAsciiTriangle();
	bool ReadAsciiVertex(char* s);

	//Writer
	bool Write(const char* filename,bool binary);
	void Delete();

	const char* _filename;
	fstream _file;

	std::vector<STL_Vec3> _normals;
	std::vector<STL_Vec3> _vertices;

	long _nbt;
	int _test;

	bool _binary;
	char _header[SIZE_HEADER_BINARY];
	char _signature[SIZE_SIGNATURE_ASCII];
};

bool FileExists(const char* inFilename);

#endif