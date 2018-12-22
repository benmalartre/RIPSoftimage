// bbppGeoCache.h
//-------------------------------------
#ifndef _bbppGeoCache_
#define _bbppGeoCache_

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>

using namespace std;

#define SIZE_FLOAT 4
#define SIZE_INT 4
#define SIZE_SIGNATURE 12

#define SIGNATURE_GEOCACHE "bbppGeoCache"

class bbppGeoFrame
{
public:
	int _nbv;				// number of vertices
	int _nbf;				// number of faces
	float* _pos;			// position as a flat array of float
	int* _fnbv;				// number vertices per face
	int* _fv;				// faces vertices indices
	int _size;				// size of the whole frame
	int _fvc;				// face vertices indices count

	bbppGeoFrame(int nbv, int nbf, int fnbv);
	~bbppGeoFrame();
	
};

class bbppGeoCache
{
public:
	bbppGeoCache();
	~bbppGeoCache(){};

	//Reader
	bool OpenFile(const char* filename);
	bool GetHeader();
	void ReadFrame(int frameid);

	//Writer
	void SetStartFrame(const int startframe);
	void SetEndFrame(const int endframe);
	void SetTimelapse(const int timelapse);
	bool Write(const char* filename);
	void AddFrame(bbppGeoFrame*&);
	void CreateOffset();
	void SetOffset(int frameid,int offset);
	void Delete();

	const char* _filename;
	fstream _file;

	std::vector<bbppGeoFrame*>* _frames;
	int* _offsets;
	int _startframe;
	int _endframe;
	int _timelapse;

	std::vector<int> _faceverticescount;
	std::vector<int> _facevertices;
	std::vector<float> _vertices;

	int _nbv;
	int _nbf;

	char _signature[SIZE_SIGNATURE];
};

bool FileExists(const char* inFilename);

#endif