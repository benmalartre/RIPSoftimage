// bbppPointCache.h
//------------------------------------
#ifndef _BBPPPOINTCACHE_H_
#define _BBPPPOINTCACHE_H_

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <math.h>

#define SIZE_POINT 12
#define START_CACHE 12
#define SIZE_SIGNATURE 16

#define SIGNATURE_POINTCACHE "_bbppPointCache"

using namespace std;

// bbppPointCache Class
//------------------------------------
class bbppPointCache  
{  
public:
	void CreatePointers(long nbp);
	void DeletePointers();
	bool isSubFrame(float in_frame,long in_roundedframe);

	void ReadFrame(long in_frame);
	void ReadSubFrame(long in_previous,long in_next,float interpolate,long in_mode,float in_tensio, float in_bias);

	inline float LinearInterpolate(float y1,float y2,float mu);
	inline float CubicInterpolate(float y0,float y1,float y2,float y3,float mu);
	inline float HermiteInterpolate(float y0,float y1,float y2,float y3,float mu,float tension,float bias);

	bool Init(const char* in_file, long in_nb);

	const char* _file;  
	long _nbPoints;
	long _startFrame;     
	long _endFrame;
	float* _position;
	float* _previous;
	float* _next;
	float* _previous2;
	float* _next2;
	fstream _s;	

	char _signature[SIZE_SIGNATURE];

};

#endif
