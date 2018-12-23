#pragma once
//---------------------------------------------------
#include "Voronoi.h"

#include <xsi_icenodedef.h>
#include <xsi_icenodecontext.h>
#include <xsi_vector3f.h>
#include <xsi_icegeometry.h>


using namespace XSI;
using namespace MATH;

enum IMVoronoi_IDs
{
	ID_IN_Mesh = 0,
	ID_IN_Points = 1,

	ID_G_100 = 100,
	ID_OUT_Position = 200,
	ID_OUT_Indices = 201,

	ID_TYPE_CNS = 400,
	ID_STRUCT_CNS,
	ID_CTXT_CNS,
	ID_UNDEF = ULONG_MAX
};

SICALLBACK IMVoronoi_BeginEvaluate(ICENodeContext& in_ctxt);
SICALLBACK IMVoronoi_Evaluate(ICENodeContext& in_ctxt);
SICALLBACK IMVoronoi_Init(CRef& in_ctxt);
SICALLBACK IMVoronoi_Term(CRef& in_ctxt);
