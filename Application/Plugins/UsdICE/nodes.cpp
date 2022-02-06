#include "nodes.h"
#include "stage.h"

CStatus RegisterUsdStageNode(PluginRegistrar& in_reg)
{
  ICENodeDef nodeDef;
  nodeDef = Application().GetFactory().CreateICENodeDef(L"UsdStageNode", L"UsdStageNode");

  CStatus st;
  st = nodeDef.PutColor(U2XNodeR, U2XNodeG, U2XNodeB);
  st.AssertSucceeded();

  st = nodeDef.PutThreadingModel(XSI::siICENodeSingleThreading);
  st.AssertSucceeded();

  st = nodeDef.DefineCustomType("UsdStage", "UsdStage", "Smart Pointer to UsdStage",
    U2XLayerDataR, U2XLayerDataG, U2XLayerDataB);

  // Add input ports and groups.
  st = nodeDef.AddPortGroup(ID_G_100);
  st.AssertSucceeded();

  st = nodeDef.AddInputPort(
    ID_IN_Filename, ID_G_100, siICENodeDataString, siICENodeStructureArray, siICENodeContextAny,
    L"Uri", L"Uri", CValue(), CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  st = nodeDef.AddInputPort(
    ID_IN_DefaultPrim, ID_G_100, siICENodeDataString, siICENodeStructureSingle, siICENodeContextAny,
    L"Prim", L"Prim", CValue(), CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded();


  CStringArray U2XStageNodeDataType(1);
  U2XStageNodeDataType[0] = L"UsdStage";

  // Add output ports.
  st = nodeDef.AddOutputPort(
    ID_OUT_Stage, U2XStageNodeDataType, siICENodeStructureSingle, siICENodeContextAny,
    L"Stage", L"Stage", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
  nodeItem.PutCategories(L"Usd");

  return CStatus::OK;
}


SICALLBACK UsdStageNode_Evaluate(ICENodeContext& in_ctxt)
{
  U2XStageNodeDatas_t* data = (U2XStageNodeDatas_t*)(CValue::siPtrType)in_ctxt.GetUserData();
 
  /*
  IRCubicBezierData_t* data = (IRCubicBezierData_t*)(CValue::siPtrType)in_ctxt.GetUserData();

  // Read the current phase. 
  ULONG nPhase = in_ctxt.GetEvaluationPhaseIndex();

  switch (nPhase)
  {
  case 0:
  {
    CDataArrayLong segmentArray(in_ctxt, ID_IN_Segment);
    CDataArray2DVector3f inputPositionArray(in_ctxt, ID_IN_Position);
    CIndexSet indexSet(in_ctxt, ID_IN_Segment);
    data->m_curves.clear();
    data->m_valid = false;
    for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next()) {
      IRCubicBezier_t crv;
      crv.m_segments = segmentArray[it.GetIndex()];
      IRCubicBezierSetCurveKnots(&crv, inputPositionArray[it]);
      IRCubicBezierDuplicateEndKnots(&crv);
      if (crv.m_valid)
      {
        IRCubicBezierGetSamples(&crv, crv.m_segments);
        data->m_valid = true;
      }
      data->m_curves.push_back(crv);
    }
    return CStatus::OK;
  }break;
  };

  // exit on invalid input
  if (!data->m_valid) return CStatus::OK;

  // The current output port being evaluated...
  ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID();

  switch (out_portID)
  {
  case ID_OUT_Position:
  {
    CDataArray2DVector3f inputPositionArray(in_ctxt, ID_IN_Position);
    CDataArray2DVector3f outputPositionArray(in_ctxt);

    CIndexSet indexSet(in_ctxt);

    for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next()) {
      IRCubicBezier_t& crv = data->m_curves[it.GetAbsoluteIndex()];
      // output
      CDataArray2DVector3f::Accessor outputPosition = outputPositionArray.Resize(it, crv.m_samples.size());

      for (ULONG i = 0; i < crv.m_samples.size(); i++) {
        size_t base = crv.m_samples[i].m_i;
        IRComputePointOnBezierCurve(crv.m_position[base],
          crv.m_position[base + 1],
          crv.m_position[base + 2],
          crv.m_position[base + 3],
          crv.m_samples[i].m_u,
          &outputPosition[i]);
      }
    }
  }break;

  case ID_OUT_Tangent:
  {
    CDataArray2DVector3f outputTangentArray(in_ctxt);

    CIndexSet indexSet(in_ctxt);

    for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next()) {
      IRCubicBezier_t& crv = data->m_curves[it.GetAbsoluteIndex()];
      // output
      CDataArray2DVector3f::Accessor outputTangent = outputTangentArray.Resize(it, crv.m_samples.size());

      outputTangent[0] = crv.m_start_tangent;
      for (ULONG i = 1; i < crv.m_samples.size() - 1; i++) {
        size_t base = crv.m_samples[i].m_i;
        IRComputeTangentOnBezierCurve(crv.m_position[base],
          crv.m_position[base + 1],
          crv.m_position[base + 2],
          crv.m_position[base + 3],
          crv.m_samples[i].m_u,
          &outputTangent[i]);
      }
      outputTangent[crv.m_samples.size() - 1] = crv.m_end_tangent;
    }
  } break;

  case ID_OUT_U:
  {
    CDataArray2DFloat outputUArray(in_ctxt);

    CIndexSet indexSet(in_ctxt);

    for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next()) {
      IRCubicBezier_t& crv = data->m_curves[it.GetAbsoluteIndex()];
      // output
      CDataArray2DFloat::Accessor outputU = outputUArray.Resize(it, crv.m_samples.size());

      for (ULONG i = 0; i < crv.m_samples.size(); i++) {
        outputU[i] = crv.m_samples[i].m_u;
      }
    }
  } break;

  case ID_OUT_NormalizedU:
  {
    CDataArray2DFloat outputNormalizedUArray(in_ctxt);

    CIndexSet indexSet(in_ctxt);

    for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next()) {
      IRCubicBezier_t& crv = data->m_curves[it.GetAbsoluteIndex()];
      // output
      CDataArray2DFloat::Accessor outputU = outputNormalizedUArray.Resize(it, crv.m_samples.size());

      for (ULONG i = 0; i < crv.m_samples.size(); i++) {
        outputU[i] = crv.m_samples[i].m_nu;
      }
    }
  } break;

  case ID_OUT_Segment:
  {
    CDataArray2DLong outputSArray(in_ctxt);

    CIndexSet indexSet(in_ctxt);

    for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next()) {
      IRCubicBezier_t& crv = data->m_curves[it.GetAbsoluteIndex()];
      // output
      CDataArray2DLong::Accessor outputS = outputSArray.Resize(it, crv.m_samples.size());

      for (ULONG i = 0; i < crv.m_samples.size(); i++) {
        outputS[i] = crv.m_samples[i].m_i;
      }
    }
  } break;
  }*/
Application().LogMessage("StageNode EVALUATE :D");
  return CStatus::OK;
}

SICALLBACK UsdStageNode_Init(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  U2XStageNodeDatas_t* data = new U2XStageNodeDatas_t();
  ctxt.PutUserData((CValue::siPtrType)data);
 
  return CStatus::OK;
}

SICALLBACK UsdStageNode_Term(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  U2XStageNodeDatas_t* data = (U2XStageNodeDatas_t*)(CValue::siPtrType)ctxt.GetUserData();
  if (data)delete data;
  ctxt.PutUserData((CValue)NULL);
  return CStatus::OK;
}

// ================================================================
// ADD SPHERE
// ================================================================

CStatus RegisterUsdSphereNode(PluginRegistrar& in_reg)
{
  ICENodeDef nodeDef;
  nodeDef = Application().GetFactory().CreateICENodeDef(L"UsdSphereNode", L"UsdSphereNode");

  CStatus st;
  st = nodeDef.PutColor(U2XNodeR, U2XNodeG, U2XNodeB);
  st.AssertSucceeded();

  st = nodeDef.PutThreadingModel(XSI::siICENodeSingleThreading);
  st.AssertSucceeded();

  st = nodeDef.DefineCustomType("UsdStage", "UsdStage", "Smart Pointer to UsdStage",
    U2XLayerDataR, U2XLayerDataG, U2XLayerDataB);

  // Add input ports and groups.
  st = nodeDef.AddPortGroup(ID_G_100);
  st.AssertSucceeded();

  CStringArray U2XStageNodeDataType(1);
  U2XStageNodeDataType[0] = L"UsdStage";

  st = nodeDef.AddInputPort(
    ID_IN_Stage, ID_G_100, U2XStageNodeDataType, siICENodeStructureSingle, siICENodeContextAny,
    L"Stage", L"Stage", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
  st.AssertSucceeded();

  st = nodeDef.AddInputPort(
    ID_IN_Name, ID_G_100, siICENodeDataString, siICENodeStructureSingle, siICENodeContextAny,
    L"Name", L"Name", CValue(), CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  st = nodeDef.AddInputPort(
    ID_IN_Xform, ID_G_100, siICENodeDataMatrix44, siICENodeStructureAny, siICENodeContextAny,
    L"Xform", L"Xform", CValue(), CValue(), CValue(), ID_UNDEF, ID_STRUCT_CNS, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  st = nodeDef.AddInputPort(
    ID_IN_Radius, ID_G_100, siICENodeDataFloat, siICENodeStructureAny, siICENodeContextAny,
    L"Radius", L"Radius", 1.0, CValue(), CValue(), ID_UNDEF, ID_STRUCT_CNS, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  // Add output ports.
  st = nodeDef.AddOutputPort(
    ID_OUT_Stage, U2XStageNodeDataType, siICENodeStructureSingle, siICENodeContextAny,
    L"Result", L"Result", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
  nodeItem.PutCategories(L"Usd");

  return CStatus::OK;
}


SICALLBACK UsdSphereNode_Evaluate(ICENodeContext& in_ctxt)
{

  Application().LogMessage("USD ADD SPHERE EVALUATE :D");
  return CStatus::OK;
}

SICALLBACK UsdSphereNode_Init(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  U2XStageNodeDatas_t* data = new U2XStageNodeDatas_t();
  ctxt.PutUserData((CValue::siPtrType)data);

  return CStatus::OK;
}

SICALLBACK UsdSphereNode_Term(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  U2XStageNodeDatas_t* data = (U2XStageNodeDatas_t*)(CValue::siPtrType)ctxt.GetUserData();
  if (data)delete data;
  ctxt.PutUserData((CValue)NULL);
  return CStatus::OK;
}

// ================================================================
// ADD CUBE
// ================================================================

CStatus RegisterUsdCubeNode(PluginRegistrar& in_reg)
{
  ICENodeDef nodeDef;
  nodeDef = Application().GetFactory().CreateICENodeDef(L"UsdCubeNode", L"UsdCubeNode");

  CStatus st;
  st = nodeDef.PutColor(U2XNodeR, U2XNodeG, U2XNodeB);
  st.AssertSucceeded();

  st = nodeDef.PutThreadingModel(XSI::siICENodeSingleThreading);
  st.AssertSucceeded();

  st = nodeDef.DefineCustomType("UsdStage", "UsdStage", "Smart Pointer to UsdStage",
    U2XLayerDataR, U2XLayerDataG, U2XLayerDataB);

  // Add input ports and groups.
  st = nodeDef.AddPortGroup(ID_G_100);
  st.AssertSucceeded();

  CStringArray U2XStageNodeDataType(1);
  U2XStageNodeDataType[0] = L"UsdStage";

  st = nodeDef.AddInputPort(
    ID_IN_Stage, ID_G_100, U2XStageNodeDataType, siICENodeStructureSingle, siICENodeContextAny,
    L"Stage", L"Stage", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
  st.AssertSucceeded();

  st = nodeDef.AddInputPort(
    ID_IN_Name, ID_G_100, siICENodeDataString, siICENodeStructureSingle, siICENodeContextAny,
    L"Name", L"Name", CValue(), CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  st = nodeDef.AddInputPort(
    ID_IN_Xform, ID_G_100, siICENodeDataMatrix44, siICENodeStructureAny, siICENodeContextAny,
    L"Xform", L"Xform", CValue(), CValue(), CValue(), ID_UNDEF, ID_STRUCT_CNS, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  st = nodeDef.AddInputPort(
    ID_IN_Width, ID_G_100, siICENodeDataFloat, siICENodeStructureAny, siICENodeContextAny,
    L"Width", L"Width", 1.0, CValue(), CValue(), ID_UNDEF, ID_STRUCT_CNS, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  st = nodeDef.AddInputPort(
    ID_IN_Depth, ID_G_100, siICENodeDataFloat, siICENodeStructureAny, siICENodeContextAny,
    L"Depth", L"Depth", 1.0, CValue(), CValue(), ID_UNDEF, ID_STRUCT_CNS, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  st = nodeDef.AddInputPort(
    ID_IN_Height, ID_G_100, siICENodeDataFloat, siICENodeStructureAny, siICENodeContextAny,
    L"Height", L"Height", 1.0, CValue(), CValue(), ID_UNDEF, ID_STRUCT_CNS, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  // Add output ports.
  st = nodeDef.AddOutputPort(
    ID_OUT_Stage, U2XStageNodeDataType, siICENodeStructureSingle, siICENodeContextAny,
    L"Result", L"Result", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
  nodeItem.PutCategories(L"Usd");

  return CStatus::OK;
}


SICALLBACK UsdCubeNode_Evaluate(ICENodeContext& in_ctxt)
{

  Application().LogMessage("USD ADD SPHERE EVALUATE :D");
  return CStatus::OK;
}

SICALLBACK UsdCubeNode_Init(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  U2XStageNodeDatas_t* data = new U2XStageNodeDatas_t();
  ctxt.PutUserData((CValue::siPtrType)data);

  return CStatus::OK;
}

SICALLBACK UsdCubeeNode_Term(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  U2XStageNodeDatas_t* data = (U2XStageNodeDatas_t*)(CValue::siPtrType)ctxt.GetUserData();
  if (data)delete data;
  ctxt.PutUserData((CValue)NULL);
  return CStatus::OK;
}