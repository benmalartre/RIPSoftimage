//---------------------------------------------------
#include "IRLeastSquareFitLine.h"

#include <cstdio>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <cstring>
#define MAX 10

enum IRLeastSquareFitLineIDs
{
  ID_IN_Position = 0,
  ID_IN_MaxIterations = 1,

  ID_G_100 = 100,
  ID_OUT_Origin = 200,
  ID_OUT_Direction = 201,

  ID_TYPE_CNS = 400,
  ID_STRUCT_CNS,
  ID_CTXT_CNS,
  ID_UNDEF = ULONG_MAX
};

static CVector3f IRLeastSquareFitLine_ComputeOrigin(CDataArray2DVector3f::Accessor& points)
{
  CVector3f result(0.f, 0.f, 0.f);
  for (int i = 0; i < points.GetCount(); ++i) {
    result += points[i];
  }
  result *= 1.f / (float)points.GetCount();
  return result;
}

static CVector3f IRLeastSquareFitLine_ComputeDirection(CDataArray2DVector3f::Accessor& points, LONG maxIterations)
{
  CVector3f direction(0, 1, 0);
  size_t numPoints = points.GetCount();
  if (!numPoints || !maxIterations) return direction;

  CVector3f centroid(0.f, 0.f, 0.f);
  for (size_t p = 0; p < numPoints; ++p) {
    centroid.AddInPlace(points[p]);
  }
  centroid.ScaleInPlace(1.f / (float)numPoints);
  
  CVector3f point;
  CVector3f last = direction;
  for (size_t i = 0; i < maxIterations; ++i) {
    for (size_t p = 0; p < numPoints; ++p) {
      point.Sub(points[p], centroid);
      direction.AddInPlace(point.ScaleInPlace(point.Dot(last)));
    }
    direction.NormalizeInPlace();
    if (last.EpsilonEquals(direction, 0.00001)) break;
    last = direction;
  }

  return last;
}

CStatus RegisterIRLeastSquareFitLine( PluginRegistrar& in_reg )
{
  ICENodeDef nodeDef;
  nodeDef = Application().GetFactory().CreateICENodeDef(L"IRLeastSquareFitLine",L"IRLeastSquareFitLine");

  CStatus st;
  st = nodeDef.PutColor(ICERigNodeR,ICERigNodeG,ICERigNodeB);
  st.AssertSucceeded( ) ;

  st = nodeDef.PutThreadingModel(XSI::siICENodeMultiThreading);
  st.AssertSucceeded();

  // Add input ports and groups.
  st = nodeDef.AddPortGroup(ID_G_100);
  st.AssertSucceeded( ) ;

  st = nodeDef.AddInputPort(
    ID_IN_Position, ID_G_100, siICENodeDataVector3, siICENodeStructureArray, siICENodeContextAny, 
    L"Position", L"Position", MATH::CVector3f(1.0, 1.0, 1.0), CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  st = nodeDef.AddInputPort(
    ID_IN_MaxIterations, ID_G_100, siICENodeDataLong, siICENodeStructureSingle, siICENodeContextSingleton,
    L"MaxIterations", L"MaxIterations", 16, 0, 256, ID_UNDEF, ID_UNDEF, ID_UNDEF
  );
  st.AssertSucceeded();

  // Add output ports.
  st = nodeDef.AddOutputPort(
    ID_OUT_Origin,siICENodeDataVector3, siICENodeStructureSingle, siICENodeContextAny,
    L"Origin", L"Origin", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded( ) ;

  st = nodeDef.AddOutputPort(
    ID_OUT_Direction, siICENodeDataVector3, siICENodeStructureSingle, siICENodeContextAny, 
    L"Direction", L"Direction", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
  nodeItem.PutCategories(L"ICERig");

  return CStatus::OK;
}

SICALLBACK IRLeastSquareFitLine_Evaluate(ICENodeContext& in_ctxt)
{
  IRLeastSquareFitLineData_t* data = 
    (IRLeastSquareFitLineData_t*)(CValue::siPtrType)in_ctxt.GetUserData();
  
  CDataArrayLong maxIterations(in_ctxt, ID_IN_MaxIterations);
  CDataArray2DVector3f inputPositionArray(in_ctxt, ID_IN_Position);
  
  // The current output port being evaluated...
  ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID();

  switch (out_portID)
  {
    case ID_OUT_Origin:
    {
      CDataArrayVector3f outputPosition(in_ctxt);
      CIndexSet indexSet(in_ctxt);

      for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next()) {
        // Creates an Accessor object to iterate over the 2D array data
        CDataArray2DVector3f::Accessor inputPosition = inputPositionArray[it];
        outputPosition[it] =
          IRLeastSquareFitLine_ComputeOrigin(inputPosition);


      }
      break;
    }

    case ID_OUT_Direction:
    {
      CDataArrayVector3f outputDirection(in_ctxt);
      CIndexSet indexSet(in_ctxt);

      for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next()) {
        // Creates an Accessor object to iterate over the 2D array data
        CDataArray2DVector3f::Accessor inputPosition = inputPositionArray[it];
        outputDirection[it] =
          IRLeastSquareFitLine_ComputeDirection(inputPosition, maxIterations[0]);
      }
      break;
    } 
  }
  return CStatus::OK;
}

SICALLBACK IRLeastSquareFitLine_Init(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  IRLeastSquareFitLineData_t* data = new IRLeastSquareFitLineData_t();
  ctxt.PutUserData((CValue::siPtrType)data);
  return CStatus::OK;
}

SICALLBACK IRLeastSquareFitLine_Term(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  IRLeastSquareFitLineData_t* data = 
    (IRLeastSquareFitLineData_t*)(CValue::siPtrType)ctxt.GetUserData();
  if(data)delete data;
  ctxt.PutUserData((CValue)NULL);
  return CStatus::OK;
}