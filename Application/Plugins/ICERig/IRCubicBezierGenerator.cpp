//---------------------------------------------------
#include "IRCubicBezier.h"

CStatus RegisterIRCubicBezierGenerator( PluginRegistrar& in_reg )
{
  ICENodeDef nodeDef;
  nodeDef = Application().GetFactory().CreateICENodeDef(L"IRCubicBezierGenerator",L"IRCubicBezierGenerator");

  CStatus st;
  st = nodeDef.PutColor(ICERigNodeR,ICERigNodeG,ICERigNodeB);
  st.AssertSucceeded( ) ;

  st = nodeDef.PutThreadingModel(XSI::siICENodeSingleThreading);
  st.AssertSucceeded();

  // Add input ports and groups.
  st = nodeDef.AddPortGroup(ID_G_100);
  st.AssertSucceeded( ) ;

  st = nodeDef.AddInputPort(
    ID_IN_Position, ID_G_100, siICENodeDataVector3, siICENodeStructureArray, siICENodeContextAny, 
    L"Points", L"Points", MATH::CVector3f(1.0, 1.0, 1.0), CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  st = nodeDef.AddInputPort(
    ID_IN_Segment, ID_G_100, siICENodeDataLong, siICENodeStructureSingle, siICENodeContextAny, 
    L"Segment", L"Segment", 5, CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded();


  // Add output ports.
  st = nodeDef.AddOutputPort(
    ID_OUT_Position,siICENodeDataVector3, siICENodeStructureArray, siICENodeContextAny,
    L"Position", L"Position", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded( ) ;

  st = nodeDef.AddOutputPort(
    ID_OUT_Tangent, siICENodeDataVector3, siICENodeStructureArray, siICENodeContextAny, 
    L"Tangent", L"Tangent", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  st = nodeDef.AddOutputPort(
    ID_OUT_U, siICENodeDataFloat, siICENodeStructureArray, siICENodeContextAny, 
    L"U", L"U", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  st = nodeDef.AddOutputPort(
    ID_OUT_Segment, siICENodeDataLong, siICENodeStructureArray, siICENodeContextAny,
    L"S", L"S", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS
  );
  st.AssertSucceeded();

  PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
  nodeItem.PutCategories(L"ICERig");

  return CStatus::OK;
}


SICALLBACK IRCubicBezierGenerator_BeginEvaluate(ICENodeContext& in_ctxt)
{
  IRCubicBezierData_t* data = (IRCubicBezierData_t*)(CValue::siPtrType)in_ctxt.GetUserData();

  CDataArrayLong segments(in_ctxt, ID_IN_Segment);
  data->m_numElements = 0;
  data->m_curves.clear();
  for (ULONG i = 0; i < segments.GetCount(); i++) {
    IRCubicBezier_t crv;
    crv.m_segments = segments[i];
    crv.m_initialized = false;
    data->m_curves.push_back(crv);
    data->m_numElements++;
  }
  in_ctxt.PutNumberOfElementsToProcess(data->m_numElements);

  in_ctxt.PutUserData((CValue::siPtrType)data);
  return CStatus::OK;

}

SICALLBACK IRCubicBezierGenerator_Evaluate( ICENodeContext& in_ctxt )
{
  IRCubicBezierData_t* data = (IRCubicBezierData_t*)(CValue::siPtrType)in_ctxt.GetUserData();

  ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );
  switch( out_portID )
  {     
    case ID_OUT_Position:
    {
      CDataArray2DVector3f inputPositionArray(in_ctxt, ID_IN_Position);
      CDataArrayLong segmentArray(in_ctxt, ID_IN_Segment);
      CDataArray2DVector3f outputPositionArray(in_ctxt);

      CIndexSet indexSet(in_ctxt);

      for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next()) {
        IRCubicBezier_t* crv =& data->m_curves[it.GetIndex()];
        if (!crv->m_initialized) {
          IRCubicBezierSetCurveKnots(crv, inputPositionArray[it]);
          IRCubicBezierDuplicateEndKnots(crv);
          if (crv->m_valid)IRCubicBezierGetSamples(crv, segmentArray[it]);
          crv->m_initialized = true;
        }

        CDataArray2DVector3f::Accessor outputPosition = outputPositionArray.Resize(it, crv->m_samples.size());

        for (ULONG i = 0; i < crv->m_samples.size(); i++) {
          size_t base = crv->m_samples[i].m_i;
          IRComputePointOnBezierCurve(
          crv->m_position[base], 
          crv->m_position[base + 1], 
          crv->m_position[base + 2],
          crv->m_position[base + 3],
          crv->m_samples[i].m_u,
          &outputPosition[i]);
        }
      }
    } break;

    case ID_OUT_Tangent:
    {
      CDataArray2DVector3f inputPositionArray(in_ctxt, ID_IN_Position);
      CDataArrayLong segmentArray(in_ctxt, ID_IN_Segment);
      CDataArray2DVector3f outputTangentArray(in_ctxt);

      CIndexSet indexSet(in_ctxt);

      for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next()) {
        IRCubicBezier_t* crv = &data->m_curves[it.GetIndex()];
        if (!crv->m_initialized) {
          IRCubicBezierSetCurveKnots(crv, inputPositionArray[it]);
          IRCubicBezierDuplicateEndKnots(crv);
          if (crv->m_valid)IRCubicBezierGetSamples(crv, segmentArray[it]);
          crv->m_initialized = true;
        }

        CDataArray2DVector3f::Accessor outputTangent = outputTangentArray.Resize(it, crv->m_samples.size());
        outputTangent[0] = crv->m_start_tangent;
        for (ULONG i = 1; i < crv->m_samples.size()-1; i++) {
          size_t base = crv->m_samples[i].m_i;
          IRComputeTangentOnBezierCurve(
          crv->m_position[base],
          crv->m_position[base + 1],
          crv->m_position[base + 2],
          crv->m_position[base + 3],
          crv->m_samples[i].m_u,
          &outputTangent[i]);
        }
        outputTangent[crv->m_samples.size() - 1] = crv->m_end_tangent;
      }
    } break;

    case ID_OUT_U:
    {
      CDataArray2DVector3f inputPositionArray(in_ctxt, ID_IN_Position);
      CDataArrayLong segmentArray(in_ctxt, ID_IN_Segment);
      CDataArray2DFloat outputUArray(in_ctxt);

      CIndexSet indexSet(in_ctxt);

      for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next()) {
        IRCubicBezier_t* crv = &data->m_curves[it.GetIndex()];
        if (!crv->m_initialized) {
          IRCubicBezierSetCurveKnots(crv, inputPositionArray[it]);
          IRCubicBezierDuplicateEndKnots(crv);
          if (crv->m_valid)IRCubicBezierGetSamples(crv, segmentArray[it]);
          crv->m_initialized = true;
        }

        CDataArray2DFloat::Accessor outputU = outputUArray.Resize(it, crv->m_samples.size());

        for (ULONG i = 0; i < crv->m_samples.size(); i++) {
          outputU[i] = crv->m_samples[i].m_u;
        }
      }
    } break;

    case ID_OUT_Segment:
    {
      CDataArray2DVector3f inputPositionArray(in_ctxt, ID_IN_Position);
      CDataArrayLong segmentArray(in_ctxt, ID_IN_Segment);
      CDataArray2DLong outputSegmentArray(in_ctxt);

      CIndexSet indexSet(in_ctxt);

      for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next()) {
        IRCubicBezier_t* crv = &data->m_curves[it.GetIndex()];
        if (!crv->m_initialized) {
          IRCubicBezierSetCurveKnots(crv, inputPositionArray[it]);
          IRCubicBezierDuplicateEndKnots(crv);
          if (crv->m_valid)IRCubicBezierGetSamples(crv, segmentArray[it]);
          crv->m_initialized = true;
        }

        CDataArray2DLong::Accessor outputS = outputSegmentArray.Resize(it, crv->m_samples.size());

        for (ULONG i = 0; i < crv->m_samples.size(); i++) {
          outputS[i] = crv->m_samples[i].m_i;
        }
      }
    } break;
  }
return CStatus::OK;
}


SICALLBACK IRCubicBezierGenerator_Init(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  IRCubicBezierData_t* data = new IRCubicBezierData_t();
  ctxt.PutUserData((CValue::siPtrType)data);
  return CStatus::OK;
}

SICALLBACK IRCubicBezierGenerator_Term(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  IRCubicBezierData_t* data = (IRCubicBezierData_t*)(CValue::siPtrType)ctxt.GetUserData();
  if(data)delete data;
  ctxt.PutUserData((CValue)NULL);
  return CStatus::OK;
}