#include "bbppGeoCache.h"
#include <sys\stat.h>

//------------------------------------------
//		Writer
//------------------------------------------
bbppGeoCache::bbppGeoCache()
{
	_frames = new std::vector<bbppGeoFrame*>;
}

bool bbppGeoCache::Write(const char *filename)
{
	_file.open(filename, ios::out|ios::binary|ios::trunc);
	if(!_file.is_open())return false;

	_file.write((char*) SIGNATURE_GEOCACHE, SIZE_SIGNATURE);
	_file.write((char*) &_startframe, SIZE_INT);
	_file.write((char*) &_endframe, SIZE_INT);
	_file.write((char*) _offsets,_timelapse * SIZE_INT);

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
	_file.close();

	return true;
}

void bbppGeoCache::AddFrame(bbppGeoFrame*& frame)
{
	_frames->push_back(frame);
}

void bbppGeoCache::SetStartFrame(const int startframe)
{
	_startframe = startframe;
}

void bbppGeoCache::SetEndFrame(const int endframe)
{
	_endframe = endframe;
}
void bbppGeoCache::SetTimelapse(const int timelapse)
{
	_timelapse = timelapse;
}

void bbppGeoCache::CreateOffset()
{
	_offsets = new int[_timelapse];
}

void bbppGeoCache::SetOffset(int frameid, int offset)
{
	_offsets[frameid] = offset;
}

void bbppGeoCache::Delete()
{
	if(_file.is_open())_file.close();
	if(_frames!=NULL)
	{
		for(int a=0;a<(int)_frames->size();a++)delete (*_frames)[a];
		_frames->clear();
		delete _frames;
	}
	if(_offsets!=NULL)delete[] _offsets;
}

//------------------------------------------
//		Reader
//------------------------------------------
bool bbppGeoCache::OpenFile(const char* filename)
{
	//_filename = NULL;
	if(_file.is_open())_file.close();
	_file.open(filename, ios::in|ios::binary|ios::beg);
	if(!_file.is_open())return false;

	_filename = filename;
	return true;
}

bool bbppGeoCache::GetHeader()
{
	_file.seekg(0,ios::beg);
	_file.read((char*) &_signature,SIZE_SIGNATURE);

	// check Signature...
	if(strncmp (_signature,SIGNATURE_GEOCACHE,SIZE_SIGNATURE) != 0)return false;

	_file.read((char*) &_startframe,sizeof(int));
	_file.read((char*) &_endframe,sizeof(int));
	_timelapse = _endframe - _startframe + 1;

	_offsets = new int[_timelapse];
	_file.read((char*)_offsets,_timelapse*sizeof(int));
	return true;
}

void bbppGeoCache::ReadFrame(int frameid)
{
	if(frameid<0)frameid = 0;
	if(frameid>_timelapse-1)frameid = _timelapse-1;

	_file.seekg(_offsets[frameid],ios::beg);
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
}

//------------------------------------------
//		Frame
//------------------------------------------
bbppGeoFrame::bbppGeoFrame(int nbv,int nbf,int fvc)
{
	_nbv = nbv;
	_nbf = nbf;
	_fvc = fvc;

	_pos = new float[_nbv*3];
	_fnbv = new int[_nbf];
	_fv = new int[_fvc];

	_size = sizeof(int)* 2;
	_size += sizeof(int)*_nbf;
	_size += sizeof(int)*_fvc;
	_size += sizeof(float)*_nbv*3;
}

bbppGeoFrame::~bbppGeoFrame()
{
	if(_pos != NULL)delete[]_pos;
	if(_fnbv != NULL)delete[]_fnbv;
	if(_fv != NULL)delete[]_fv;
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
