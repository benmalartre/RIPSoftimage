#pragma once
//---------------------------------------------------
#include <vector>
#include "IRRegister.h"

using namespace ICERIG;
using namespace XSI;
using namespace MATH;

struct IRLeastSquareFitLineData_t {
};

SICALLBACK IRLeastSquareFitLine_Evaluate(ICENodeContext& in_ctxt);
SICALLBACK IRLeastSquareFitLine_Init(CRef& in_ctxt);
SICALLBACK IRLeastSquareFitLine_Term(CRef& in_ctxt);
