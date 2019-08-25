#pragma once
//---------------------------------------------------
#include "IRRegister.h"

using namespace ICERIG;
using namespace XSI;
using namespace MATH;

struct IRCubicBezierSample_t {
	size_t m_i;
	float m_u;
	float m_nu;
};

struct IRCubicBezier_t
{
	size_t m_segments;
	std::vector<CVector3f> m_position;
	std::vector<IRCubicBezierSample_t> m_samples;
	CVector3f m_start_knot;
	CVector3f m_end_knot;
	CVector3f m_start_tangent;
	CVector3f m_end_tangent;
	bool m_valid;
	bool m_initialized;
};

struct IRCubicBezierData_t
{
	std::vector<IRCubicBezier_t> m_curves;
	ULONG m_numElements;
	bool m_initialized;
	bool m_valid;
};

enum IRCubicBezierIDs
{
	ID_IN_Position = 0,
	ID_IN_Segment = 1,
	ID_IN_U = 2,

	ID_G_100 = 100,
	ID_OUT_Position = 200,
	ID_OUT_Tangent = 201,
	ID_OUT_U = 202,
	ID_OUT_NormalizedU = 203,
	ID_OUT_Segment = 204,

	ID_TYPE_CNS = 400,
	ID_STRUCT_CNS,
	ID_CTXT_CNS,
	ID_UNDEF = ULONG_MAX
};

void IRCubicBezierDuplicateEndKnots(IRCubicBezier_t* crv);
void IRCubicBezierSetCurveKnots(IRCubicBezier_t* crv, CDataArray2DVector3f::Accessor& positions);
void IRCubicBezierGetSamples(IRCubicBezier_t* crv, size_t numSamples);
void IRCubicBezierGetSample(IRCubicBezier_t* crv, float u);

void IRComputePointOnBezierCurve(
	const CVector3f& p1,
	const CVector3f&  p2,
	const CVector3f&  p3,
	const CVector3f&  p4,
	float u,
	CVector3f* result);

void IRComputeTangentOnBezierCurve(
	const CVector3f& p0,
	const CVector3f& p1,
	const CVector3f& p2,
	const CVector3f& p3,
	float u,
	CVector3f* t);

SICALLBACK IRCubicBezier_Evaluate(ICENodeContext& in_ctxt);
SICALLBACK IRCubicBezier_Init(CRef& in_ctxt);
SICALLBACK IRCubicBezier_Term(CRef& in_ctxt);

SICALLBACK IRCubicBezierGenerator_BeginEvaluate(ICENodeContext& in_ctxt);
SICALLBACK IRCubicBezierGenerator_Evaluate(ICENodeContext& in_ctxt);
SICALLBACK IRCubicBezierGenerator_Init(CRef& in_ctxt);
SICALLBACK IRCubicBezierGenerator_Term(CRef& in_ctxt);

// ------------------------------------------------------------------
// Interpolations 
// ------------------------------------------------------------------
/*
inline float LINEAR_INTERPOLATE(float y1, float y2, double mu)
{
	return((float)(y1*(1 - mu) + y2 * mu));
};

inline float CUBIC_INTERPOLATE(float y0, float y1, float y2, float y3, double mu)
{
	double a0, a1, a2, a3, mu2;

	mu2 = (mu*mu);
	a0 = y3 - y2 - y0 + y1;
	a1 = y0 - y1 - a0;
	a2 = y2 - y0;
	a3 = y1;

	return((float)(a0*mu*mu2 + a1 * mu2 + a2 * mu + a3));
};

inline float HERMITE_INTERPOLATE(float y0, float y1, float y2, float y3, double mu, double tension, double bias)
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
*/