#include "IRRegister.h"

using namespace ICERIG;

// Defines port, group and map identifiers used for registering the ICENode
enum IRBuildElementIDs
{
  ID_IN_NbJoints    = 0,
  ID_IN_NbBones     = 1,
  ID_IN_Divisions   = 2,
  ID_G_100          = 100,
  ID_OUT_ElementID  = 200,
  ID_OUT_SubID      = 201,
  ID_OUT_BaseID     = 202,
  ID_OUT_JointID    = 203,
  ID_OUT_U          = 204,
  ID_TYPE_CNS       = 400,
  ID_STRUCT_CNS,
  ID_CTXT_CNS,
  ID_UNDEF = ULONG_MAX
};

CStatus RegisterIRBuildElementID( PluginRegistrar& in_reg )
{
  ICENodeDef nodeDef;
  nodeDef = Application().GetFactory().CreateICENodeDef(L"IRBuildElementID",L"IRBuildElementID");

  CStatus st;
  st = nodeDef.PutColor(IRNodeR, IRNodeG, IRNodeB);
  st.AssertSucceeded( ) ;

  st = nodeDef.PutThreadingModel(siICENodeSingleThreading);
  st.AssertSucceeded( ) ;

  // Add input ports and groups.
  st = nodeDef.AddPortGroup(ID_G_100);
  st.AssertSucceeded( ) ;

  st = nodeDef.AddInputPort(
    ID_IN_NbJoints, ID_G_100, siICENodeDataLong, siICENodeStructureArray, siICENodeContextSingleton,
    L"NbJoints", L"NbJoints", 0, CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded( ) ;

  st = nodeDef.AddInputPort(
    ID_IN_NbBones, ID_G_100, siICENodeDataLong, siICENodeStructureArray, siICENodeContextSingleton,
    L"NbBones", L"NbBones", 0, CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded( ) ;

  st = nodeDef.AddInputPort(
    ID_IN_Divisions, ID_G_100, siICENodeDataLong, siICENodeStructureArray, siICENodeContextSingleton, 
    L"Divisions", L"Divisions", 1, CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  // Add output ports.
  st = nodeDef.AddOutputPort(
    ID_OUT_ElementID, siICENodeDataLong, siICENodeStructureArray, siICENodeContextSingleton,
    L"ID", L"ID", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded( ) ;

  st = nodeDef.AddOutputPort(
    ID_OUT_SubID, siICENodeDataLong, siICENodeStructureArray, siICENodeContextSingleton,
    L"SubID", L"SubID", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded( ) ;

  st = nodeDef.AddOutputPort(
    ID_OUT_BaseID, siICENodeDataLong, siICENodeStructureArray, siICENodeContextSingleton,
    L"BaseID", L"BaseID", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded( ) ;

  st = nodeDef.AddOutputPort(
    ID_OUT_JointID, siICENodeDataLong, siICENodeStructureArray, siICENodeContextSingleton, 
    L"JointID", L"JointID", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  st = nodeDef.AddOutputPort(
    ID_OUT_U, siICENodeDataFloat, siICENodeStructureArray, siICENodeContextSingleton, 
    L"U", L"U", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
  nodeItem.PutCategories(L"ICERig");

  return CStatus::OK;
}

SICALLBACK IRBuildElementID_Evaluate( ICENodeContext& in_ctxt )
{
  // Get the input data buffers 
  CDataArray2DLong JointsData( in_ctxt, ID_IN_NbJoints );
  CDataArray2DLong::Accessor JointsSubArray = JointsData[0];
  CDataArray2DLong BonesData( in_ctxt, ID_IN_NbBones );
  CDataArray2DLong::Accessor BonesSubArray = BonesData[0];
  CDataArray2DLong DivisionsData(in_ctxt, ID_IN_Divisions);
  CDataArray2DLong::Accessor DivisionsSubArray = DivisionsData[0];

  LONG nbj = JointsSubArray.GetCount();
  ULONG numElements = JointsSubArray.GetCount();
  ULONG numJoints = 0;
  ULONG numBones = 0;
  for (int i = 0; i < nbj; i++)numJoints += JointsSubArray[i] - 1;
  for (int i = 0; i < nbj; i++)numBones += BonesSubArray[i];

  LONG nbb = BonesSubArray.GetCount();
  LONG nbd = DivisionsSubArray.GetCount();

  if (nbd != numJoints) {
  return CStatus::InvalidArgument;
  }

  ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );

  switch( out_portID )
  {     
    case ID_OUT_ElementID:
    {
      CDataArray2DLong outData(in_ctxt);
      CDataArray2DLong::Accessor outAccessor = outData.Resize(0, numBones);
      int offset = 0;
      for (int i = 0; i<nbb; i++) {
        int v = BonesSubArray[i];
        for (int j = 0; j<v; j++)outAccessor[offset++] = i;
      }

      return CStatus::OK;
    }break;

    case ID_OUT_SubID:
    {
      CDataArray2DLong outData(in_ctxt);
      CDataArray2DLong::Accessor outAccessor = outData.Resize(0, numBones);
      int offset = 0;
      for (int i = 0; i<nbb; i++) {
        int v = BonesSubArray[i];
        for (int j = 0; j<v; j++)outAccessor[offset++] = j;
      }
      return CStatus::OK;
    }break;

    case ID_OUT_BaseID:
    {
      CDataArray2DLong outData(in_ctxt);
      CDataArray2DLong::Accessor outAccessor = outData.Resize(0, numElements);
      ULONG base = 0;
      for (int i = 0; i<nbj; i++) {
        outAccessor[i] = base;
        base += JointsSubArray[i];
      }

      return CStatus::OK;
    }break;

    case ID_OUT_JointID:
    {
      CDataArray2DLong outData(in_ctxt);
      CDataArray2DLong::Accessor outAccessor = outData.Resize(0, numBones);
      int idx = 0;
      int offset = 0;
      int jnt = 0;
      for (int i = 0; i<nbj; i++) {
        int v = JointsSubArray[i] - 1;
        for (int j = 0; j < v; j++) {
          int w = DivisionsSubArray[offset + j];
          for (int k = 0; k < w; k++)outAccessor[idx++] = jnt;
          jnt ++;
        }
        offset += v;
        jnt ++;
      }
      return CStatus::OK;
    }break;

    case ID_OUT_U:
    {
      CDataArray2DFloat outData(in_ctxt);
      CDataArray2DFloat::Accessor outAccessor = outData.Resize(0, numBones);
      float u;
      int idx = 0;
      int offset = 0;
      for (int i = 0; i<nbj; i++) {
        int v = JointsSubArray[i] - 1;
        for (int j = 0; j < v; j++) {
          int w = DivisionsSubArray[offset+j];
          if (w == 1)outAccessor[idx++] = 0.5f;
          else {
            u = 1.0f / (float)(w);
            for (int k = 0; k < w; k++)outAccessor[idx++] = (k+0.5f)*u;
          }
        }
        offset += v;
      }
      return CStatus::OK;
    }break;
  }

}



