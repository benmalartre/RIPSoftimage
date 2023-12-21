#include "nodes.h"
#include "stage.h"

CStatus RegisterUsdStageNode(PluginRegistrar& in_reg)
{
  ICENodeDef nodeDef;
  nodeDef = Application().GetFactory().CreateICENodeDef(L"UsdStageNode", L"UsdStageNode");

  CStatus st;
  st = nodeDef.PutColor(U2INodeR, U2INodeG, U2INodeB);
  st.AssertSucceeded();

  st = nodeDef.PutThreadingModel(XSI::siICENodeSingleThreading);
  st.AssertSucceeded();

  st = nodeDef.DefineCustomType("UsdStage", "UsdStage", "Smart Pointer to UsdStage",
    U2ILayerDataR, U2ILayerDataG, U2ILayerDataB);

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


  CStringArray U2IStageNodeDataType(1);
  U2IStageNodeDataType[0] = L"UsdStage";

  // Add output ports.
  st = nodeDef.AddOutputPort(
    ID_OUT_Stage, U2IStageNodeDataType, siICENodeStructureSingle, siICENodeContextAny,
    L"Stage", L"Stage", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  st = nodeDef.AddOutputPort(
    ID_OUT_Eval, siICENodeDataString, siICENodeStructureSingle, siICENodeContextSingleton,
    L"Eval", L"Eval", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
  nodeItem.PutCategories(L"Usd");

  return CStatus::OK;
}


SICALLBACK UsdStageNode_Evaluate(ICENodeContext& in_ctxt)
{
  U2IStageNodeDatas_t* data = (U2IStageNodeDatas_t*)(CValue::siPtrType)in_ctxt.GetUserData();
  if(data->_uri != data->_lastUri) {
    data->_stage = pxr::UsdStage::Open(data->_uri);
  }
  Application().LogMessage("StageNode EVALUATE :D");
  /*
  pxr::UsdStageWeakPtr _stage;
  std::string _uri;

  U2IStageNodeDatas_t() : _stage(nullptr), _uri("") {};
  2X_SCENE->Update();
  HYDRA_ENGINE->Render(U2I_SCENE->GetSceneStage()->GetPseudoRoot(), renderParams);
  glDisable(GL_DEPTH_TEST);
  */
Application().LogMessage("StageNode EVALUATE :D");
  return CStatus::OK;
}

SICALLBACK UsdStageNode_Init(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  U2IStageNodeDatas_t* data = new U2IStageNodeDatas_t();
  ctxt.PutUserData((CValue::siPtrType)data);
 
  return CStatus::OK;
}

SICALLBACK UsdStageNode_Term(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  U2IStageNodeDatas_t* data = (U2IStageNodeDatas_t*)(CValue::siPtrType)ctxt.GetUserData();
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
  st = nodeDef.PutColor(U2INodeR, U2INodeG, U2INodeB);
  st.AssertSucceeded();

  st = nodeDef.PutThreadingModel(XSI::siICENodeSingleThreading);
  st.AssertSucceeded();

  st = nodeDef.DefineCustomType("UsdStage", "UsdStage", "Smart Pointer to UsdStage",
    U2ILayerDataR, U2ILayerDataG, U2ILayerDataB);

  // Add input ports and groups.
  st = nodeDef.AddPortGroup(ID_G_100);
  st.AssertSucceeded();

  CStringArray U2IStageNodeDataType(1);
  U2IStageNodeDataType[0] = L"UsdStage";

  st = nodeDef.AddInputPort(
    ID_IN_Stage, ID_G_100, U2IStageNodeDataType, siICENodeStructureSingle, siICENodeContextAny,
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
    ID_OUT_Stage, U2IStageNodeDataType, siICENodeStructureSingle, siICENodeContextAny,
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
  U2IStageNodeDatas_t* data = new U2IStageNodeDatas_t();
  ctxt.PutUserData((CValue::siPtrType)data);

  return CStatus::OK;
}

SICALLBACK UsdSphereNode_Term(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  U2IStageNodeDatas_t* data = (U2IStageNodeDatas_t*)(CValue::siPtrType)ctxt.GetUserData();
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
  st = nodeDef.PutColor(U2INodeR, U2INodeG, U2INodeB);
  st.AssertSucceeded();

  st = nodeDef.PutThreadingModel(XSI::siICENodeSingleThreading);
  st.AssertSucceeded();

  st = nodeDef.DefineCustomType("UsdStage", "UsdStage", "Smart Pointer to UsdStage",
    U2ILayerDataR, U2ILayerDataG, U2ILayerDataB);

  // Add input ports and groups.
  st = nodeDef.AddPortGroup(ID_G_100);
  st.AssertSucceeded();

  CStringArray U2IStageNodeDataType(1);
  U2IStageNodeDataType[0] = L"UsdStage";

  st = nodeDef.AddInputPort(
    ID_IN_Stage, ID_G_100, U2IStageNodeDataType, siICENodeStructureSingle, siICENodeContextAny,
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
    ID_OUT_Stage, U2IStageNodeDataType, siICENodeStructureSingle, siICENodeContextAny,
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
  U2IStageNodeDatas_t* data = new U2IStageNodeDatas_t();
  ctxt.PutUserData((CValue::siPtrType)data);

  return CStatus::OK;
}

SICALLBACK UsdCubeeNode_Term(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  U2IStageNodeDatas_t* data = (U2IStageNodeDatas_t*)(CValue::siPtrType)ctxt.GetUserData();
  if (data)delete data;
  ctxt.PutUserData((CValue)NULL);
  return CStatus::OK;
}