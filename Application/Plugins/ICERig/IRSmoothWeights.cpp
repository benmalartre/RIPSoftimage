#include "IRRegister.h"
#include "IRSmooth.h"

using namespace ICERIG;

// Defines port, group and map identifiers used for registering the ICENode
enum IRSmoothWeightsIDs
{
  ID_IN_PointWeight = 0,
  ID_IN_NeighborIDs = 1,
  ID_IN_Iterations = 2,
  ID_IN_SmoothMap = 3,
  ID_G_100 = 100,
  ID_OUT_OutWeights = 200,
  ID_TYPE_CNS = 400,
  ID_STRUCT_CNS,
  ID_CTXT_CNS,
  ID_UNDEF = ULONG_MAX
};

CStatus RegisterIRSmoothWeights( PluginRegistrar& in_reg )
{
  ICENodeDef nodeDef;
  nodeDef = Application().GetFactory().CreateICENodeDef(L"IRSmoothWeights", L"IRSmoothWeights");

  CStatus st;
  st = nodeDef.PutColor(ICERigNodeR,ICERigNodeG,ICERigNodeB);
  st.AssertSucceeded( );

  st = nodeDef.PutThreadingModel(siICENodeSingleThreading);
  st.AssertSucceeded( );

  // Add input ports and groups.
  st = nodeDef.AddPortGroup(ID_G_100);
  st.AssertSucceeded( );

  st = nodeDef.AddInputPort(
    ID_IN_PointWeight, ID_G_100, siICENodeDataFloat, siICENodeStructureArray, siICENodeContextComponent0D,
    L"PointWeights", L"PointWeights", 0, CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded( );

  st = nodeDef.AddInputPort(
    ID_IN_NeighborIDs, ID_G_100,siICENodeDataLong, siICENodeStructureArray, siICENodeContextComponent0D,
    L"NeighborIDs", L"NeighborIDs", 0, CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded( ) ;

  st = nodeDef.AddInputPort(
    ID_IN_Iterations, ID_G_100, siICENodeDataLong, siICENodeStructureSingle, siICENodeContextSingleton,
    L"Iterations", L"Iterations", 0, CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_UNDEF
  );
  st.AssertSucceeded( ) ;

  st = nodeDef.AddInputPort(
    ID_IN_SmoothMap, ID_G_100, siICENodeDataFloat, siICENodeStructureSingle, siICENodeContextComponent0D,
    L"SmoothMap", L"SmoothMap", 0, CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded( ) ;

  // Add output ports.
  st = nodeDef.AddOutputPort(
    ID_OUT_OutWeights, siICENodeDataFloat, siICENodeStructureArray, siICENodeContextComponent0D,
    L"OutWeights", L"OutWeights", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded( ) ;

  PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
  nodeItem.PutCategories(L"ICERig");

  return CStatus::OK;
}

SICALLBACK IRSmoothWeights_Evaluate( ICENodeContext& in_ctxt )
{
  // Get User Data
  Smooth* smooth = (Smooth*)(CValue::siPtrType)in_ctxt.GetUserData( );

  // Get the input data buffers for each port
  CDataArray2DFloat PointWeightData( in_ctxt, ID_IN_PointWeight );
  CDataArray2DLong NeighborIDsData( in_ctxt, ID_IN_NeighborIDs );
  CDataArrayLong IterationsData( in_ctxt, ID_IN_Iterations );
  CDataArrayFloat SmoothMapData( in_ctxt, ID_IN_SmoothMap );

  if(!smooth->_init)
  smooth->Init(SmoothMapData.GetCount(),IterationsData[0]);

  smooth->iterations = IterationsData[0];

  std::vector<float> tmp;

  // We Create Point Info Datas
  CIndexSet indexSet( in_ctxt );
  for(CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next()) {
    SmoothPoint* p = smooth->_points[it.GetAbsoluteIndex()];
    p->_weights.clear();
    p->_neighbors.clear();

    CDataArray2DFloat::Accessor WeightsSubArray = PointWeightData[it];
    ULONG nbWeights = WeightsSubArray.GetCount();
    for(ULONG w=0; w<nbWeights; w++) {
      p->PushWeights(WeightsSubArray[w]);
      //if(it.GetAbsoluteIndex() == 0)Application().LogMessage((CString)WeightsSubArray[w]);
    }
    CDataArray2DLong::Accessor NeighborIDsSubArray = NeighborIDsData[it];
    ULONG nbNeighbors = NeighborIDsSubArray.GetCount();
    for (ULONG i=0; i<nbNeighbors; i++) {
      p->PushNeighbors(NeighborIDsSubArray[i]);
    }

    p->_smooth = SmoothMapData[it];
  }

  smooth->Compute();

  SmoothPoint* p;
  ULONG u;

  CDataArray2DFloat outData( in_ctxt );

  for(CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next()) {
    p = smooth->_points[it.GetAbsoluteIndex()];
    u = (ULONG)p->_weights.size();

    CDataArray2DFloat::Accessor outAccessor = outData.Resize( it, u);
    for( ULONG i=0;i<u;i++) {
      outAccessor[i] = p->_weights[i];
    }
  }
  return CStatus::OK;
}


SICALLBACK IRSmoothWeights_Init( CRef& in_ctxt )
{
  Context ctxt(in_ctxt);
  Smooth* smooth = new Smooth();
  smooth->_init = false;
  ctxt.PutUserData( (CValue::siPtrType)smooth );
  return CStatus::OK;
}

SICALLBACK IRSmoothWeights_Term( CRef& in_ctxt )
{
  Context ctxt(in_ctxt);
  if(!ctxt.GetUserData().IsEmpty()) {
    Smooth* smooth = (Smooth*)(CValue::siPtrType)ctxt.GetUserData( );
    smooth->CleanUp();
    delete smooth;
    ctxt.PutUserData((CValue)NULL);
  }
  return CStatus::OK;
}



