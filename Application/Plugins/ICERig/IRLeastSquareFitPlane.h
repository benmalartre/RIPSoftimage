#pragma once
//---------------------------------------------------
#include "IRRegister.h"

using namespace ICERIG;
using namespace XSI;
using namespace MATH;

struct IRLeastSquareFitPlaneData_t {
};


SICALLBACK IRLeastSquareFitPlane_Evaluate(ICENodeContext& in_ctxt);
SICALLBACK IRLeastSquareFitPlane_Init(CRef& in_ctxt);
SICALLBACK IRLeastSquareFitPlane_Term(CRef& in_ctxt);
