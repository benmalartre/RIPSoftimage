#include "IRRegister.h"


enum IRFabrikFlags {
  START   = 1,
  MIDDLE  = 2,
  END     = 4,
};

struct IRFabrikJoint {
  CVector3f origin;
  CVector3f position;
  CVector3f limits;
  short     flags;
};

struct IRFabrikSegment {
  IRFabrikJoint* joint0;
  IRFabrikJoint* joint1;
  
  float Length() {
    return (joint1->position.SubInPlace(joint0->position)).GetLength();
  };
  float RestLength() {
    return (joint1->origin.SubInPlace(joint0->origin)).GetLength();
  }
};

struct IRFabrikControl {
  CMatrix4f staticKineSTate;
  CMatrix4f deformKineState;
};

struct IRFabrikChain {
  std::vector<IRFabrikSegment>  segments;
  IRFabrikControl*              startControl;
  IRFabrikControl*              endControl;
};

class IRFabrikSkeleton {
public:
  void InitPoints(
    size_t numPoints,
    const CVector3f* positions,
    const CVector3f* limits,
    const LONG* mappings
  );

private:
  std::vector<IRFabrikJoint>   _joints;
  std::vector<IRFabrikChain>   _chains;
  std::vector<IRFabrikControl> _controls;
};

void IRFabrikSkeleton::InitPoints(
  size_t numPoints,
  const CVector3f* positions, 
  const CVector3f* limits, 
  const LONG* mappings
)
{
  _joints.resize(numPoints);
  
  size_t lastChainIndex = 0;
  size_t jointCount = 1;
  std::vector<std::pair<size_t, size_t>> segmentJointDesc;
  for (size_t i = 1; i < numPoints; ++i) {
    if (lastChainIndex != mappings[i]) {
      lastChainIndex = mappings[i];
      segmentJointDesc.push_back(std::make_pair(i - jointCount, i - 1));
      LOG("SEGMENT DESC : " + CString(i - jointCount) + " --> " + CString(i - 1));
      jointCount = 1;
    } else if (i == (numPoints - 1)) {
      segmentJointDesc.push_back(std::make_pair(i - jointCount, i));
      LOG("SEGMENT DESC : " + CString(i - jointCount) + " --> " + CString(i));
    } else {
      jointCount++;
    }
  }

  size_t numChains = segmentJointDesc.size();
  _chains.resize(numChains);
  for (size_t i = 0; i < numChains; ++i) {
    const std::pair<size_t, size_t>& desc = segmentJointDesc[i];
    for (size_t j = desc.first; j <= desc.second; ++j) {
      LOG("SET JOINT :" + CString(j));
      _joints[j].origin = positions[j];
      _joints[j].position = positions[j];
      _joints[j].limits = limits[j];
      if (j == desc.first) {
        _joints[j].flags = START;
      } else if (j == desc.second) {
        _joints[j].flags = END;
      } else {
        _joints[j].flags = MIDDLE;
      }
    }
    size_t numSegments = desc.second - desc.first;
    IRFabrikChain* chain = &_chains[i];
    chain->segments.resize(numSegments);
    for (size_t j = 0; j < numSegments; ++j) {
      LOG("SET SEGMENT :" + CString(j));
      IRFabrikSegment* segment = &(chain->segments[j]);
      segment->joint0 = &_joints[desc.first + j];
      segment->joint1 = &_joints[desc.first + j + 1];
    }
  }
}


// Defines port, group and map identifiers used for registering the ICENode
enum IRFabrikIDs
{
  ID_IN_Position = 0,
  ID_IN_Limit = 1,
  ID_IN_Mapping = 2,
  ID_IN_StaticKineState = 3,
  ID_IN_DeformKineState = 4,
  ID_IN_Iteration = 5,
  ID_G_100 = 100,
  ID_OUT_SolveKineState = 200,
  ID_OUT_BindKineState = 201,
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
    ID_IN_Position,
    ID_G_100,
    siICENodeDataVector3,
    siICENodeStructureArray,
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
    ID_IN_Limit,
    ID_G_100,
    siICENodeDataVector3,
    siICENodeStructureArray,
    siICENodeContextSingleton,
    L"Limit",
    L"Limit",
    CVector3f(),
    CVector3f(),
    CVector3f(),
    ID_UNDEF,
    ID_UNDEF,
    ID_CTXT_CNS
  );
  st.AssertSucceeded();

  st = nodeDef.AddInputPort(
    ID_IN_Mapping,
    ID_G_100,
    siICENodeDataLong,
    siICENodeStructureArray,
    siICENodeContextSingleton,
    L"Mapping",
    L"Mapping",
    CVector3f(),
    CVector3f(),
    CVector3f(),
    ID_UNDEF,
    ID_UNDEF,
    ID_CTXT_CNS
  );
  st.AssertSucceeded();

  st = nodeDef.AddInputPort(
    ID_IN_StaticKineState, 
    ID_G_100, 
    siICENodeDataMatrix44, 
    siICENodeStructureArray, 
    siICENodeContextSingleton,
    L"StaticKineState",
    L"StaticKineState", 
    CMatrix4f(),
    CMatrix4f(),
    CMatrix4f(),
    ID_UNDEF,
    ID_UNDEF, 
    ID_CTXT_CNS
  );
  st.AssertSucceeded();

  st = nodeDef.AddInputPort(
    ID_IN_DeformKineState,
    ID_G_100,
    siICENodeDataMatrix44,
    siICENodeStructureArray,
    siICENodeContextSingleton,
    L"DeformKineState",
    L"DeformKineState",
    CMatrix4f(),
    CMatrix4f(),
    CMatrix4f(),
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
    ID_OUT_BindKineState,
    siICENodeDataMatrix44,
    siICENodeStructureArray,
    siICENodeContextSingleton,
    L"BindKineState",
    L"BindKineState",
    ID_UNDEF,
    ID_UNDEF,
    ID_CTXT_CNS
  );
  st.AssertSucceeded();

  st = nodeDef.AddOutputPort(
    ID_OUT_SolveKineState,
    siICENodeDataMatrix44, 
    siICENodeStructureArray,
    siICENodeContextSingleton,
    L"SolveKineState", 
    L"SolveKineState", 
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
  CDataArray2DVector3f positionsArray(in_ctxt, ID_IN_Position);
  CDataArray2DVector3f::Accessor positions = positionsArray[0];
  CDataArray2DVector3f limitsArray(in_ctxt, ID_IN_Limit);
  CDataArray2DVector3f::Accessor limits = limitsArray[0];
  CDataArray2DLong mappingsArray(in_ctxt, ID_IN_Mapping);
  CDataArray2DLong::Accessor mappings = mappingsArray[0];

  size_t numPoints = positions.GetCount();
  if (numPoints != limits.GetCount() || numPoints != mappings.GetCount()) {
    LOG("[IRFabrikSolver] mismatch in input array size");
    return CStatus::Fail;
  }

  IRFabrikSkeleton skeleton;
  skeleton.InitPoints(numPoints, &positions[0], &limits[0], &mappings[0]);

  /*
  CDataArray2DMatrix4f staticKineStates(in_ctxt, ID_IN_StaticKineState);
  CDataArray2DMatrix4f::Accessor itStaticKineStates = staticKineStates[0];
  size_t numStaticKineStates = itStaticKineStates.GetCount();

  CDataArray2DMatrix4f deformKineStates(in_ctxt, ID_IN_DeformKineState);
  CDataArray2DMatrix4f::Accessor itDeformKineStates = deformKineStates[0];
  size_t numDeformKineStates = itDeformKineStates.GetCount();

  std::vector<CMatrix4f> ascT;
  std::vector<float> lengths;
  ascT.resize(numStaticKineStates);
  lengths.resize(numStaticKineStates - 1);

  
  CDataArray2DMatrix4f outData(in_ctxt);
  CDataArray2DMatrix4f::Accessor outAccessor = outData.Resize(0, numStaticKineStates);

  bool reachable = false;

  CVector3f e, s, d;
  float tl = 0.0;
  for (size_t i = numStaticKineStates; i-- > 0;)
  {
    ascT[i] = itStaticKineStates[i];
    if (i > 0) {
      s.Set(
        itStaticKineStates[i - 1].GetValue(3, 0),
        itStaticKineStates[i - 1].GetValue(3, 1),
        itStaticKineStates[i - 1].GetValue(3, 2)
      );
      e.Set(
        itStaticKineStates[i].GetValue(3, 0),
        itStaticKineStates[i].GetValue(3, 1),
        itStaticKineStates[i].GetValue(3, 2)
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
    ascT[numStaticKineStates - 1].SetValue(3, 0, pos.GetX());
    ascT[numStaticKineStates - 1].SetValue(3, 1, pos.GetY());
    ascT[numStaticKineStates - 1].SetValue(3, 2, pos.GetZ());

    for (size_t i = numStaticKineStates - 1; i-- > 0;)
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

  for (int i = numStaticKineStates; i-- > 0;)
  {
    outAccessor[i] = ascT[i];
  }
  */

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
