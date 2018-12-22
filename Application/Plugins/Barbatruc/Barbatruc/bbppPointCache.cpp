// bbppPointCache.Cpp
//------------------------------------
#include "bbppPointCache.h"

bool bbppPointCache::Init(const char* in_filename,long in_nbpoints )
{
	if(_s.is_open())_s.close();

	struct stat results;
	if (stat(in_filename, &results) == 0)
	{
		_s.open(in_filename,ios::binary|ios::in);

		_s.read((char*) &_signature, SIZE_SIGNATURE);
		// check Signature...
		if(strncmp (_signature,SIGNATURE_POINTCACHE,16) != 0)return false;

		_s.read((char*) &_nbPoints, sizeof(long));
		_s.read((char*) &_startFrame, sizeof(long));
		_s.read((char*) &_endFrame, sizeof(long));

		if(in_nbpoints != _nbPoints)return false;

		return true;
	}
	else
	{
		return false;
	}
}

void bbppPointCache::ReadFrame(long in_frame)
{
	long id = in_frame * SIZE_POINT * _nbPoints + START_CACHE;
	_s.seekg(id,ios::beg);
	_s.read((char*) _position, sizeof(double)* 3 *_nbPoints);
}

void bbppPointCache::ReadSubFrame(long in_previous,long in_next,float in_interpolate,long in_mode,float in_tension,float in_bias)
{
	long id = in_previous * SIZE_POINT * _nbPoints + START_CACHE;
	_s.seekg(id,ios::beg);

	_s.read((char*) _previous, sizeof(float)* 3 *_nbPoints);

	id = in_next * SIZE_POINT * _nbPoints + START_CACHE;
	_s.seekg(id,ios::beg);

	_s.read((char*) _next, sizeof(float)* 3 *_nbPoints);

	if(in_mode==0)
	{
		for(long a=0;a<_nbPoints;a++)
		{
			_position[a*3] = LinearInterpolate(_previous[a*3],_next[a*3],in_interpolate);
			_position[a*3+1] = LinearInterpolate(_previous[a*3+1],_next[a*3+1],in_interpolate);
			_position[a*3+2] = LinearInterpolate(_previous[a*3+2],_next[a*3+2],in_interpolate);
		}
	}
	
	else 
	{
		id = (in_previous - 1) * SIZE_POINT * _nbPoints + START_CACHE;
		_s.seekg(id,ios::beg);

		_s.read((char*) _previous2, sizeof(float)* 3 *_nbPoints);

		id = (in_next + 1) * SIZE_POINT * _nbPoints + START_CACHE;
		_s.seekg(id,ios::beg);

		_s.read((char*) _next2, sizeof(float)* 3 *_nbPoints);

		if(in_mode == 1)
		{
			for(long a=0;a<_nbPoints;a++)
			{
				_position[a*3] = CubicInterpolate(_previous2[a*3],_previous[a*3],_next[a*3],_next2[a*3],in_interpolate);
				_position[a*3+1] = CubicInterpolate(_previous2[a*3+1],_previous[a*3+1],_next[a*3+1],_next2[a*3+1],in_interpolate);
				_position[a*3+2] = CubicInterpolate(_previous2[a*3+2],_previous[a*3+2],_next[a*3+2],_next2[a*3+2],in_interpolate);
			}
		}
		else
		{
			for(long a=0;a<_nbPoints;a++)
			{
				_position[a*3] = HermiteInterpolate(_previous2[a*3],_previous[a*3],_next[a*3],_next2[a*3],in_interpolate,in_tension,in_bias);
				_position[a*3+1] = HermiteInterpolate(_previous2[a*3+1],_previous[a*3+1],_next[a*3+1],_next2[a*3+1],in_interpolate,in_tension,in_bias);
				_position[a*3+2] = HermiteInterpolate(_previous2[a*3+2],_previous[a*3+2],_next[a*3+2],_next2[a*3+2],in_interpolate,in_tension,in_bias);
			}
		}
	}
}

bool bbppPointCache::isSubFrame(float in_frame,long in_roundedframe)
{
	if(fabs(in_frame-in_roundedframe)>0.001)return 1;
	else return 0;
}

void bbppPointCache::CreatePointers(long in_nb)
{
	_position = new float[in_nb*3];
	_previous = new float[in_nb*3];
	_next = new float[in_nb*3];
	_previous2 = new float[in_nb*3];
	_next2 = new float[in_nb*3];
}

void bbppPointCache::DeletePointers()
{
	if(_position)delete[]_position;
	if(_previous)delete[]_previous;
	if(_next)delete[]_next;
	if(_previous2)delete[]_previous2;
	if(_next2)delete[]_next2;
}

inline float bbppPointCache::LinearInterpolate(float y1,float y2,float mu)
{
	 return(y1*(1-mu)+y2*mu);
};

inline float bbppPointCache::CubicInterpolate(float y0,float y1,float y2,float y3,float mu)
{
   float a0,a1,a2,a3,mu2;

   mu2 = (mu*mu);
   a0 = y3 - y2 - y0 + y1;
   a1 = y0 - y1 - a0;
   a2 = y2 - y0;
   a3 = y1;

   return(a0*mu*mu2+a1*mu2+a2*mu+a3);
};

inline float bbppPointCache::HermiteInterpolate(float y0,float y1,float y2,float y3,float mu,float tension,float bias)
{
   float m0,m1,mu2,mu3;
   float a0,a1,a2,a3;

	mu2 = mu * mu;
	mu3 = mu2 * mu;
   m0  = (y1-y0)*(1+bias)*(1-tension)/2;
   m0 += (y2-y1)*(1-bias)*(1-tension)/2;
   m1  = (y2-y1)*(1+bias)*(1-tension)/2;
   m1 += (y3-y2)*(1-bias)*(1-tension)/2;
   a0 =  2*mu3 - 3*mu2 + 1;
   a1 =    mu3 - 2*mu2 + mu;
   a2 =    mu3 -   mu2;
   a3 = -2*mu3 + 3*mu2;

   return(a0*y1+a1*m0+a2*m1+a3*y2);
};