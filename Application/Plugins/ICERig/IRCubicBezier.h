#pragma once
//---------------------------------------------------
#include "IRRegister.h"

using namespace ICERIG;
using namespace XSI;
using namespace MATH;

struct IRCubicBezierSample_t {
	size_t m_i;
	float m_u;
};

struct IRCubicBezier_t
{
	size_t m_segments;
	std::vector<CVector3f> m_position;
	std::vector<IRCubicBezierSample_t> m_samples;
	CVector3f m_start_knot;
	CVector3f m_end_knot;
	bool m_valid;
};

struct IRCubicBezierData_t
{
	std::vector<IRCubicBezier_t> m_curves;
	ULONG m_numElements;
};

void IRCubicBezierDuplicateEndKnots(IRCubicBezier_t* crv);
void IRCubicBezierSetCurveKnots(IRCubicBezier_t* crv, CDataArray2DVector3f::Accessor& positions);
void IRCubicBezierGetSamples(IRCubicBezier_t* crv, size_t numSamples);
//void IRComputePositionOnBezierCurve(float u, const CVector3f& p0, const CVector3f& p1, const CVector3f& p2, const CVector3f& p3, CVector3f* p);
//void IRComputeLinearSegmentsLengths(int segmentCount, const CVector3f& p0, const CVector3f& p1, const CVector3f& p2, const CVector3f& p3, std::vector<float>* lengths);
void IRComputeTangentOnBezierCurve(float u, const CVector3f& p0, const CVector3f& p1, const CVector3f& p2, const CVector3f& p3, CVector3f* t);

enum IRCubicBezierIDs
{
	ID_IN_Position = 0,
	ID_IN_Segment = 1,
	//ID_IN_Rotation = 2,
	//ID_IN_Scale = 3,
	ID_IN_SegmentCount = 2,
	ID_G_100 = 100,
	ID_OUT_Position = 200,
	//ID_OUT_Tangent = 201,
	//ID_OUT_Length = 202,
	ID_OUT_U = 201,
	//ID_OUT_Rotation = 203,
	//ID_OUT_Scale = 204,
	ID_TYPE_CNS = 400,
	ID_STRUCT_CNS,
	ID_CTXT_CNS,
	ID_UNDEF = ULONG_MAX
};

void IRCubicBezierDuplicateEndKnots(IRCubicBezier_t* crv);
void IRCubicBezierSetCurveKnots(IRCubicBezier_t* crv, CDataArray2DVector3f::Accessor& positions);
void IRCubicBezierGetSamples(IRCubicBezier_t* crv, size_t numSamples);

void Interpolate(const CVector3f& p1,
				const CVector3f&  p2,
				const CVector3f&  p3,
				const CVector3f&  p4,
				float u,
				CVector3f* result);


SICALLBACK IRCubicBezier_BeginEvaluate(ICENodeContext& in_ctxt);
SICALLBACK IRCubicBezier_Evaluate(ICENodeContext& in_ctxt);
SICALLBACK IRCubicBezier_Init(CRef& in_ctxt);
SICALLBACK IRCubicBezier_Term(CRef& in_ctxt);

SICALLBACK IRCubicBezierGenerator_BeginEvaluate(ICENodeContext& in_ctxt);
SICALLBACK IRCubicBezierGenerator_Evaluate(ICENodeContext& in_ctxt);
SICALLBACK IRCubicBezierGenerator_Init(CRef& in_ctxt);
SICALLBACK IRCubicBezierGenerator_Term(CRef& in_ctxt);