// PointCache.h
//------------------------------------
#ifndef _POINTCACHE_H_
#define _POINTCACHE_H_

#include <xsi_status.h>
#include <xsi_application.h>
#include <xsi_value.h>
#include <xsi_customoperator.h>
#include <xsi_operator.h>
#include <xsi_operatorcontext.h>
#include <xsi_string.h>
#include <xsi_x3dobject.h>
#include <xsi_kinematics.h>
#include <xsi_primitive.h>
#include <xsi_geometry.h>
#include <xsi_point.h>
#include <xsi_command.h>
#include <xsi_argument.h>
#include <xsi_utils.h>
#include <xsi_uitoolkit.h>
#include <xsi_progressbar.h>

#include <vector>
#include <fstream>

using namespace XSI;
using namespace XSI::MATH;

// CPointCacheReader Class
//------------------------------------
class CPointCacheOp  
{  
public:
  CPointCacheOp()
    : _position(NULL)
    , _previous(NULL)
    , _next(NULL)
    , _previous2(NULL)
    , _next2(NULL)
    , _blobdatas(NULL) {};

	~CPointCacheOp(){};

	CStatus Update(OperatorContext&);
	void DeletePointers();

private:
	bool isSubFrame(double in_frame,long in_roundedframe);

	inline CStatus ReadFrame(long in_frame);
	inline CStatus ReadSubFrame(long in_previous,long in_next,double interpolate,long in_mode,double in_tensio, double in_bias);

	void CreatePointers(long nbp);
	CStatus Init(const XSI::CString& in_file, long in_nb);

	CString _file;  
	long _nbPoints;
	long _startFrame;     
	long _endFrame;
	double* _position;
	double* _previous;
	double* _next;
	double* _previous2;
	double* _next2;
	double* _blobdatas;
	bool _blobvalid;
	std::fstream _s;	
};

const long START_CACHE = 12;
const long SIZE_POINT = 24;


// ------------------------------------------------------------------
// Interpolations 
// ------------------------------------------------------------------
inline float LinearInterpolate(float y1, float y2, double mu)
{
	return((float)(y1*(1 - mu) + y2 * mu));
};

inline float CubicInterpolate(float y0, float y1, float y2, float y3, double mu)
{
	double a0, a1, a2, a3, mu2;

	mu2 = (mu*mu);
	a0 = y3 - y2 - y0 + y1;
	a1 = y0 - y1 - a0;
	a2 = y2 - y0;
	a3 = y1;

	return((float)(a0*mu*mu2 + a1 * mu2 + a2 * mu + a3));
};

inline float HermiteInterpolate(float y0, float y1, float y2, float y3, double mu, double tension, double bias)
{
	double m0, m1, mu2, mu3;
	double a0, a1, a2, a3;

	mu2 = mu * mu;
	mu3 = mu2 * mu;
	m0 = (y1 - y0)*(1 + bias)*(1 - tension) / 2;
	m0 += (y2 - y1)*(1 - bias)*(1 - tension) / 2;
	m1 = (y2 - y1)*(1 + bias)*(1 - tension) / 2;
	m1 += (y3 - y2)*(1 - bias)*(1 - tension) / 2;
	a0 = 2 * mu3 - 3 * mu2 + 1;
	a1 = mu3 - 2 * mu2 + mu;
	a2 = mu3 - mu2;
	a3 = -2 * mu3 + 3 * mu2;

	return((float)(a0*y1 + a1 * m0 + a2 * m1 + a3 * y2));
};

#endif
