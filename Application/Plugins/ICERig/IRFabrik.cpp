#include "IRRegister.h"


// Defines port, group and map identifiers used for registering the ICENode
enum IRFabrikIDs
{
  ID_IN_Ts = 0,
  ID_IN_Pos = 1,
  ID_IN_Iteration = 2,
  ID_G_100 = 100,
  ID_OUT_T = 200,
  ID_TYPE_CNS = 400,
  ID_STRUCT_CNS,
  ID_CTXT_CNS,
  ID_UNDEF = ULONG_MAX
};

CStatus RegisterIRFabrik(PluginRegistrar& in_reg)
{
  ICENodeDef nodeDef;
  nodeDef = Application().GetFactory().CreateICENodeDef(L"IRFabrik", L"IRFabrik");

  CStatus st;
  st = nodeDef.PutColor(ICERIG::ICERigNodeR, ICERIG::ICERigNodeG, ICERIG::ICERigNodeB);
  st.AssertSucceeded();

  // Add input ports and groups.
  st = nodeDef.AddPortGroup(ID_G_100);
  st.AssertSucceeded();

  st = nodeDef.AddInputPort(
    ID_IN_Ts, 
    ID_G_100, 
    siICENodeDataMatrix44, 
    siICENodeStructureArray, 
    siICENodeContextSingleton,
    L"Ts",
    L"Ts", 
    CMatrix4f(),
    CMatrix4f(),
    CMatrix4f(),
    ID_UNDEF,
    ID_UNDEF, 
    ID_CTXT_CNS
  );
  st.AssertSucceeded();

  st = nodeDef.AddInputPort(
    ID_IN_Pos, 
    ID_G_100, 
    siICENodeDataVector3, 
    siICENodeStructureSingle, 
    siICENodeContextSingleton, 
    L"Position", 
    L"Position", 
    CVector3f(), 
    CVector3f(), 
    CVector3f(), 
    ID_UNDEF, 
    ID_UNDEF, 
    ID_CTXT_CNS
  );
  st.AssertSucceeded();

  st = nodeDef.AddInputPort(
    ID_IN_Iteration, 
    ID_G_100, 
    siICENodeDataLong, 
    siICENodeStructureSingle, 
    siICENodeContextSingleton,
    L"Iteration", 
    L"Iteration",
    CValue(), 
    CValue(), 
    CValue(), 
    ID_UNDEF, 
    ID_UNDEF, 
    ID_CTXT_CNS
  );
  st.AssertSucceeded();

  // Add output ports.
  st = nodeDef.AddOutputPort(
    ID_OUT_T,
    siICENodeDataMatrix44, 
    siICENodeStructureArray,
    siICENodeContextSingleton,
    L"OutT", 
    L"OutT", 
    ID_UNDEF, 
    ID_UNDEF, 
    ID_CTXT_CNS
  );
  st.AssertSucceeded();


  PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
  nodeItem.PutCategories(L"ICERig");

  return CStatus::OK;
}

SICALLBACK IRFabrik_Evaluate(ICENodeContext& in_ctxt)
{

  CDataArrayVector3f InP(in_ctxt, ID_IN_Pos);
  CVector3f pos = InP[0];

  CDataArray2DMatrix4f InT(in_ctxt, ID_IN_Ts);
  CDataArray2DMatrix4f::Accessor accessor = InT[0];
  size_t nbt = accessor.GetCount();

  std::vector<CMatrix4f> ascT;
  std::vector<float> lengths;
  ascT.resize(nbt);
  lengths.resize(nbt - 1);

  CDataArray2DMatrix4f outData(in_ctxt);
  CDataArray2DMatrix4f::Accessor outAccessor = outData.Resize(0, nbt);

  bool reachable = false;

  CVector3f e, s, d;
  float tl = 0.0;
  for (size_t i = nbt; i-- > 0;)
  {
    ascT[i] = accessor[i];
    if (i > 0) {
      s.Set(
        accessor[i - 1].GetValue(3, 0), 
        accessor[i - 1].GetValue(3, 1), 
        accessor[i - 1].GetValue(3, 2)
      );
      e.Set(
        accessor[i].GetValue(3, 0), 
        accessor[i].GetValue(3, 1), 
        accessor[i].GetValue(3, 2)
      );
      d.Sub(e, s);
      lengths[i - 1] = d.GetLength();
      tl += lengths[i - 1];
    }
  }

  d.Sub(pos, s);
  if (d.GetLength() < tl)reachable = true;


  if (reachable)
  {
    ascT[nbt - 1].SetValue(3, 0, pos.GetX());
    ascT[nbt - 1].SetValue(3, 1, pos.GetY());
    ascT[nbt - 1].SetValue(3, 2, pos.GetZ());

    for (size_t i = nbt - 1; i-- > 0;)
    {
      s.Set(ascT[i].GetValue(3, 0), ascT[i].GetValue(3, 1), ascT[i].GetValue(3, 2));
      e.Set(ascT[i + 1].GetValue(3, 0), ascT[i + 1].GetValue(3, 1), ascT[i + 1].GetValue(3, 2));
      d.Sub(s, e);
      d.NormalizeInPlace();
      d.ScaleInPlace(lengths[i]);
      d.AddInPlace(e);
      ascT[i].SetValue(3, 0, d.GetX());
      ascT[i].SetValue(3, 1, d.GetY());
      ascT[i].SetValue(3, 2, d.GetZ());
    }

  }
  else
  {

  }

  for (int i = nbt; i-- > 0;)
  {
    outAccessor[i] = ascT[i];
  }


  //CDataArray2DMatrix44 InT( in_ctxt, ID_IN_T );
  /*
  CDataArray2DCustomType::Accessor accessor = ToonixLine[0];

  for(ULONG i=0;i<accessor.GetCount();i++)
  {
    accessor.GetData( i,(const CDataArrayCustomType::TData**)&data, sz );
    line = (TXLine*)data;
    out->Append(line);
  }

  CDataArrayCustomType::TData* pOutData = outData.Resize(0,sizeof(TXLine));
  ::memcpy( pOutData, out, sizeof(TXLine) );
      */
  return CStatus::OK;
}
