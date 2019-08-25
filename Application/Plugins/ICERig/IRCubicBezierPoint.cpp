//---------------------------------------------------
#include "IRCubicBezier.h"

CStatus RegisterIRCubicBezierPoint( PluginRegistrar& in_reg )
{
        ICENodeDef nodeDef;
        nodeDef = Application().GetFactory().CreateICENodeDef(L"IRCubicBezierPoint",L"IRCubicBezierPoint");

        CStatus st;
        st = nodeDef.PutColor(ICERigNodeR,ICERigNodeG,ICERigNodeB);
        st.AssertSucceeded( ) ;

		st = nodeDef.PutThreadingModel(XSI::siICENodeMultiThreading);
		st.AssertSucceeded();

        // Add input ports and groups.
        st = nodeDef.AddPortGroup(ID_G_100);
        st.AssertSucceeded( ) ;

		st = nodeDef.AddInputPort(ID_IN_Position, ID_G_100, siICENodeDataVector3, siICENodeStructureArray, siICENodeContextAny, L"Points", L"Points", MATH::CVector3f(1.0, 1.0, 1.0), CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
		st.AssertSucceeded();

		st = nodeDef.AddInputPort(ID_IN_U, ID_G_100, siICENodeDataFloat, siICENodeStructureSingle, siICENodeContextAny, L"U", L"U", 0, 0, 1, ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
		st.AssertSucceeded();

        // Add output ports.
        st = nodeDef.AddOutputPort(ID_OUT_Position,siICENodeDataVector3, siICENodeStructureSingle, siICENodeContextAny,L"Position",L"Position",ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
        st.AssertSucceeded( ) ;

		st = nodeDef.AddOutputPort(ID_OUT_Tangent, siICENodeDataVector3, siICENodeStructureSingle, siICENodeContextAny, L"Tangent", L"Tangent", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
		st.AssertSucceeded();


        PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
        nodeItem.PutCategories(L"ICERig");

        return CStatus::OK;
}


/*
SICALLBACK IRCubicBezierPoint_SubmitEvaluationPhaseInfo(ICENodeContext& in_ctxt)
{
	ULONG nPhase = in_ctxt.GetEvaluationPhaseIndex();
	switch (nPhase)
	{
	case 0:
	{
		in_ctxt.AddEvaluationPhaseInputPort(ID_IN_U);
		in_ctxt.AddEvaluationPhaseInputPort(ID_IN_Position);
	}
	break;

	case 1:
	{
		in_ctxt.SetLastEvaluationPhase();
	}
	break;
	}
	return CStatus::OK;
}
*/


/*
SICALLBACK IRCubicBezierPoint_Evaluate(ICENodeContext& in_ctxt)
{
	IRCubicBezierData_t* data = (IRCubicBezierData_t*)(CValue::siPtrType)in_ctxt.GetUserData();

	
	// Read the current phase. 
	ULONG nPhase = in_ctxt.GetEvaluationPhaseIndex();

	switch (nPhase)
	{
	case 0:
	{

		siICENodeDataType dataType;
		siICENodeStructureType structType;
		siICENodeContextType ctxtType;
		in_ctxt.GetPortInfo(ID_IN_U, dataType, structType, ctxtType);
		if (structType == siICENodeStructureSingle) {
			CDataArrayFloat uArray(in_ctxt, ID_IN_U);
		}
		else if (structType == siICENodeStructureArray) {
			CDataArray2DFloat uArray(in_ctxt, ID_IN_U);
		}
		
		CDataArrayFloat uArray(in_ctxt, ID_IN_U);
		
		CDataArray2DVector3f inputPositionArray(in_ctxt, ID_IN_Position);
		CIndexSet indexSet(in_ctxt, ID_IN_Position);
		data->m_curves.clear();
		data->m_valid = false;
		for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
		{
			IRCubicBezier_t crv;
			float u = uArray[it];
			crv.m_segments = 64;// segmentArray[it.GetIndex()];
			IRCubicBezierSetCurveKnots(&crv, inputPositionArray[it]);
			IRCubicBezierDuplicateEndKnots(&crv);
			if (crv.m_valid)
			{
				IRCubicBezierGetSample(&crv, u);
				data->m_valid = true;
			}
			data->m_curves.push_back(crv);
		}

		return CStatus::OK;
	}
	break;
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
		CDataArrayFloat uArray(in_ctxt, ID_IN_U);
		CDataArrayVector3f outputPositionArray(in_ctxt);

		CIndexSet indexSet(in_ctxt);

		for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
		{
			IRCubicBezier_t& crv = data->m_curves[it.GetAbsoluteIndex()];
			
			size_t base = crv.m_samples[0].m_i;
			IRComputePointOnBezierCurve(crv.m_position[base],
				crv.m_position[base + 1],
				crv.m_position[base + 2],
				crv.m_position[base + 3],
				crv.m_samples[0].m_u,
				&outputPositionArray[it.GetAbsoluteIndex()]);
		}
	}
	break;

	case ID_OUT_Tangent:
	{
		CDataArrayVector3f outputTangentArray(in_ctxt);

		CIndexSet indexSet(in_ctxt);

		for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
		{
			IRCubicBezier_t& crv = data->m_curves[it.GetAbsoluteIndex()];
			// output
			size_t base = crv.m_samples[0].m_i;
			IRComputeTangentOnBezierCurve(crv.m_position[base],
				crv.m_position[base + 1],
				crv.m_position[base + 2],
				crv.m_position[base + 3],
				crv.m_samples[0].m_u,
				&outputTangentArray[it.GetAbsoluteIndex()]);
		}

	}
	break;
	}

	return CStatus::OK;
}
*/

SICALLBACK IRCubicBezierPoint_BeginEvaluate(ICENodeContext& in_ctxt)
{
	IRCubicBezierData_t* data = (IRCubicBezierData_t*)(CValue::siPtrType)in_ctxt.GetUserData();

	CDataArrayFloat us(in_ctxt, ID_IN_U);
	data->m_curves.clear();
	for (ULONG i = 0; i < us.GetCount(); i++)
	{
		IRCubicBezier_t crv;
		crv.m_segments = 64;// us[i];
		crv.m_initialized = false;
		data->m_curves.push_back(crv);
	}

	in_ctxt.PutUserData((CValue::siPtrType)data);
	return CStatus::OK;

}

SICALLBACK IRCubicBezierPoint_Evaluate(ICENodeContext& in_ctxt)
{
	IRCubicBezierData_t* data = (IRCubicBezierData_t*)(CValue::siPtrType)in_ctxt.GetUserData();

	// The current output port being evaluated...
	ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID();
	switch (out_portID)
	{
	case ID_OUT_Position:
	{
		CDataArray2DVector3f inputPositionArray(in_ctxt, ID_IN_Position);
		CDataArrayFloat uArray(in_ctxt, ID_IN_U);
		CDataArrayVector3f outputPositionArray(in_ctxt);

		CIndexSet indexSet(in_ctxt);

		for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
		{
			IRCubicBezier_t crv;

			IRCubicBezierSetCurveKnots(&crv, inputPositionArray[it]);
			IRCubicBezierDuplicateEndKnots(&crv);
			if (crv.m_valid)IRCubicBezierGetSample(&crv, uArray[it]);
			crv.m_initialized = true;

			// output

			size_t base = crv.m_samples[0].m_i;
			IRComputePointOnBezierCurve(
				crv.m_position[base],
				crv.m_position[base + 1],
				crv.m_position[base + 2],
				crv.m_position[base + 3],
				crv.m_samples[0].m_u,
				&outputPositionArray[it]);
		}
	}
	break;

	case ID_OUT_Tangent:
	{
		CDataArray2DVector3f inputPositionArray(in_ctxt, ID_IN_Position);
		CDataArrayFloat segmentArray(in_ctxt, ID_IN_U);
		CDataArrayVector3f outputTangentArray(in_ctxt);

		CIndexSet indexSet(in_ctxt);

		for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
		{
			IRCubicBezier_t crv;
			IRCubicBezierSetCurveKnots(&crv, inputPositionArray[it]);
			IRCubicBezierDuplicateEndKnots(&crv);
			if (crv.m_valid)IRCubicBezierGetSamples(&crv, segmentArray[it]);
			crv.m_initialized = true;


			// output
			size_t base = crv.m_samples[0].m_i;
			IRComputeTangentOnBezierCurve(
				crv.m_position[base],
				crv.m_position[base + 1],
				crv.m_position[base + 2],
				crv.m_position[base + 3],
				crv.m_samples[0].m_u,
				&outputTangentArray[it]);

		}
	}
	break;
	}

	return CStatus::OK;
}

SICALLBACK IRCubicBezierPoint_Init(CRef& in_ctxt)
{
	Context ctxt(in_ctxt);
	IRCubicBezierData_t* data = new IRCubicBezierData_t();

	ctxt.PutUserData((CValue::siPtrType)data);
	return CStatus::OK;
}

SICALLBACK IRCubicBezierPoint_Term(CRef& in_ctxt)
{
	Context ctxt(in_ctxt);
	IRCubicBezierData_t* data = (IRCubicBezierData_t*)(CValue::siPtrType)ctxt.GetUserData();
	if (data)delete data;
	ctxt.PutUserData((CValue)NULL);
	return CStatus::OK;
}