//---------------------------------------------------
#include "IRLeastSquareFitPlane.h"

#include <cstdio>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <cstring>
#define MAX 10

enum IRLeastSquareFitPlaneIDs
{
  ID_IN_Position = 0,
  ID_IN_Method = 1,

  ID_G_100 = 100,
  ID_OUT_Origin = 200,
  ID_OUT_Normal = 201,

  ID_TYPE_CNS = 400,
  ID_STRUCT_CNS,
  ID_CTXT_CNS,
  ID_UNDEF = ULONG_MAX
};

static CVector3f IRLeastSquareFitPlane_ComputeOrigin(CDataArray2DVector3f::Accessor& points)
{
  CVector3f result(0.f, 0.f, 0.f);
  for (int i = 0; i < points.GetCount(); ++i) {
    result += points[i];
  }
  result *= 1.f / (float)points.GetCount();
  return result;
}

static CVector3f IRLeastSquareFitPlane_ComputeNormal(CDataArray2DVector3f::Accessor& points)
{
  CVector3f Y(0.f, 0.f, 0.f);;
  CMatrix3f matrix, invMatrix;
  CVector3f result(0.f, 0.f, 0.f);

  float* data = matrix.Get();
  memset(data, 0.f, 9 * sizeof(float));

  for (int j = 0; j < 3; ++j) {
    for (int i = 0; i < points.GetCount(); ++i) {
      data[0 + j] += points[i][0] * points[i][j];
      data[3 + j] += points[i][1] * points[i][j];
      data[6 + j] += points[i][2] * points[i][j];
      Y[j] -= points[i][j];
    }
  }
  
  if (invMatrix.Invert(matrix))
  {
    data = invMatrix.Get();
    for (int i = 0; i < 3; ++i) {
      result[0] += data[0 + i] * Y[i];
      result[1] += data[3 + i] * Y[i];
      result[2] += data[6 + i] * Y[i];
    }
    result.NormalizeInPlace();
  }

  return result;
}

static CVector3f IRLeastSquareFitPlane_ComputeNormal2(CDataArray2DVector3f::Accessor& points)
{
  CVector3f Y(0.f, 0.f, 0.f);;
  CMatrix3f matrix, invMatrix;
  CVector3f result(0.f, 0.f, 0.f);

  float* data = matrix.Get();
  memset(data, 0.f, 9 * sizeof(float));

  for (int j = 0; j < 3; ++j) {
    for (int i = 0; i < points.GetCount(); ++i) {
      data[0 + j] += points[i][0] * points[i][j];
      data[3 + j] += points[i][1] * points[i][j];
      data[6 + j] += points[i][2] * points[i][j];
      Y[j] -= points[i][j];
    }
  }

  if (invMatrix.Invert(matrix))
  {
    data = invMatrix.Get();
    for (int i = 0; i < 3; ++i) {
      result[0] += data[0 + i] * Y[i];
      result[1] += data[3 + i] * Y[i];
      result[2] += data[6 + i] * Y[i];
    }
    result.NormalizeInPlace();
  }

  return result;
}

CStatus RegisterIRLeastSquareFitPlane( PluginRegistrar& in_reg )
{
  ICENodeDef nodeDef;
  nodeDef = Application().GetFactory().CreateICENodeDef(L"IRLeastSquareFitPlane",L"IRLeastSquareFitPlane");

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

  // Add output ports.
  st = nodeDef.AddOutputPort(
    ID_OUT_Origin,siICENodeDataVector3, siICENodeStructureSingle, siICENodeContextAny,
    L"Origin", L"Origin", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded( ) ;

  st = nodeDef.AddOutputPort(
    ID_OUT_Normal, siICENodeDataVector3, siICENodeStructureSingle, siICENodeContextAny, 
    L"Normal", L"Normal", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
  nodeItem.PutCategories(L"ICERig");

  return CStatus::OK;
}

SICALLBACK IRLeastSquareFitPlane_Evaluate(ICENodeContext& in_ctxt)
{
  IRLeastSquareFitPlaneData_t* data = 
    (IRLeastSquareFitPlaneData_t*)(CValue::siPtrType)in_ctxt.GetUserData();

  // The current output port being evaluated...
  ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID();

  switch (out_portID)
  {
    case ID_OUT_Origin:
    {
      CDataArray2DVector3f inputPositionArray(in_ctxt, ID_IN_Position);
      CDataArrayVector3f outputPosition(in_ctxt);
      CIndexSet indexSet(in_ctxt);

      for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next()) {
        // Creates an Accessor object to iterate over the 2D array data
        CDataArray2DVector3f::Accessor inputSample = inputPositionArray[it];

        outputPosition[it] = IRLeastSquareFitPlane_ComputeOrigin(inputSample);
      }
      break;
    }

    case ID_OUT_Normal:
    {
      CDataArray2DVector3f inputPositionArray(in_ctxt, ID_IN_Position);
      CDataArrayLong inputMethod(in_ctxt, ID_IN_Method);
      CDataArrayVector3f outputNormal(in_ctxt);

      CIndexSet indexSet(in_ctxt);

      for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next()) {
        // Creates an Accessor object to iterate over the 2D array data
        CDataArray2DVector3f::Accessor inputPosition = inputPositionArray[it];

        // output
        if (inputMethod[0]) {
          outputNormal[it] = IRLeastSquareFitPlane_ComputeNormal2(inputPosition);
        } else {
          outputNormal[it] = IRLeastSquareFitPlane_ComputeNormal(inputPosition);
        }

      }
      break;
    } 

  
  }
  return CStatus::OK;
}

SICALLBACK IRLeastSquareFitPlane_Init(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  IRLeastSquareFitPlaneData_t* data = new IRLeastSquareFitPlaneData_t();

  ctxt.PutUserData((CValue::siPtrType)data);
  return CStatus::OK;
}

SICALLBACK IRLeastSquareFitPlane_Term(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  IRLeastSquareFitPlaneData_t* data = (IRLeastSquareFitPlaneData_t*)(CValue::siPtrType)ctxt.GetUserData();
  if(data)delete data;
  ctxt.PutUserData((CValue)NULL);
  return CStatus::OK;
}