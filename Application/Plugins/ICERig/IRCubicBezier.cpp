//---------------------------------------------------
#include "IRCubicBezier.h"

void IRCubicBezierDuplicateEndKnots(IRCubicBezier_t* crv)
{
	if (crv->m_position.size() > 1)
	{
		size_t last = crv->m_position.size() - 1;
		crv->m_start_knot.Sub(crv->m_position[0], crv->m_position[1]);
		crv->m_start_knot.AddInPlace(crv->m_position[0]);
		crv->m_start_tangent.Sub(crv->m_position[0], crv->m_start_knot);
		crv->m_start_tangent.NormalizeInPlace();

		crv->m_end_knot.Sub(crv->m_position[last], crv->m_position[last-1]);
		crv->m_end_knot.AddInPlace(crv->m_position[last]);
		crv->m_end_tangent.Sub(crv->m_end_knot, crv->m_position[last]);
		crv->m_end_tangent.NormalizeInPlace();

		crv->m_position.insert(crv->m_position.begin(), crv->m_start_knot);
		crv->m_position.push_back(crv->m_end_knot);
		crv->m_valid = true;
	}
	else crv->m_valid = false;
}

void IRCubicBezierSetCurveKnots(IRCubicBezier_t* crv, CDataArray2DVector3f::Accessor& positions)
{
	crv->m_position.resize(positions.GetCount());
	//memcpy(&crv->m_position[0], &positions[0], sizeof(CVector3f)* positions.GetCount());
	
	for (size_t i = 0; i < positions.GetCount(); i++)
	{
		crv->m_position[i] = positions[i];
	}
}

void IRCubicBezierGetSamples(IRCubicBezier_t* crv, size_t numSamples)
{
	if (numSamples == 0)
	{
		crv->m_samples.resize(0);
		return;
	}
	else
	{
		crv->m_samples.resize(numSamples);
		float incr = 1.0 / (numSamples-1);
		size_t numPoints = crv->m_position.size();
		size_t numPatches = numPoints - 3;
		size_t lastPoint = numPoints - 4;
		float u,nu, u_clamped;
		for (size_t s = 0; s < numSamples; s++)
		{
			nu = s * incr;
			u = s * incr * numPatches;
			u_clamped = std::floorf(u);
			crv->m_samples[s].m_u = u - u_clamped;
			crv->m_samples[s].m_nu = nu;
			crv->m_samples[s].m_i = (size_t)u_clamped;
		}
	}
}

void IRCubicBezierGetSample(IRCubicBezier_t* crv, float u)
{
	crv->m_samples.resize(1);
	size_t numPoints = crv->m_position.size();
	size_t numPatches = numPoints - 3;
	size_t lastPoint = numPoints - 4;
	float u_proj, u_clamped;

	u_proj = u * numPatches;
	u_clamped = std::floorf(u_proj);
	crv->m_samples[0].m_u = u_proj - u_clamped;
	crv->m_samples[0].m_nu = u;
	crv->m_samples[0].m_i = (size_t)u_clamped;

}

void IRComputePointOnBezierCurve(const CVector3f& p1, const CVector3f&  p2, const CVector3f&  p3, const CVector3f&  p4, float u, CVector3f* result)
{

	float inv6 = 1.0 / 6.0;
	// (-p1 + 3 * p2 - 3 * p3 + p4) / 6.0
	CVector3f a, p2c, p3c;
	a.Negate(p1);
	p2c.Scale(3, p2);
	p3c.Scale(3, p3);
	a.AddInPlace(p2c);
	a.SubInPlace(p3c);
	a.AddInPlace(p4);
	a.ScaleInPlace(inv6);

	// ( 3*p1 - 6*p2 + 3*p3 )/6.0
	CVector3f b, p1c, p2c2;
	p1c.Scale(3, p1);
	p2c2.Scale(6, p2);
	b.Sub(p1c, p2c2);
	b.AddInPlace(p3c);
	b.ScaleInPlace(inv6);

	// (-3*p1 + 3*p3 )/6.0
	CVector3f c;
	c.Negate(p1c);
	c.AddInPlace(p3c);
	c.ScaleInPlace(inv6);

	// ( p1 + 4*p2 + p3 )/6.0
	CVector3f d;
	d.Scale(4, p2);
	d.AddInPlace(p1);
	d.AddInPlace(p3);
	d.ScaleInPlace(inv6);

	// result
	result->Scale(u, a);
	result->AddInPlace(b);
	result->ScaleInPlace(u);
	result->AddInPlace(c);
	result->ScaleInPlace(u);
	result->AddInPlace(d);

}

void IRComputeTangentOnBezierCurve(const CVector3f& p0, const CVector3f& p1, const CVector3f& p2, const CVector3f& p3, float u, CVector3f* t)
{
	CVector3f lookAt;
	IRComputePointOnBezierCurve(p0, p1, p2, p3, u, &lookAt);
	IRComputePointOnBezierCurve(p0, p1, p2, p3, u+0.01, t);
	t->SubInPlace(lookAt);
	t->NormalizeInPlace();
	
}

CStatus RegisterIRCubicBezier( PluginRegistrar& in_reg )
{
        ICENodeDef nodeDef;
        nodeDef = Application().GetFactory().CreateICENodeDef(L"IRCubicBezier",L"IRCubicBezier");

        CStatus st;
        st = nodeDef.PutColor(ICERigNodeR,ICERigNodeG,ICERigNodeB);
        st.AssertSucceeded( ) ;

		st = nodeDef.PutThreadingModel(XSI::siICENodeMultiEvaluationPhase);
		st.AssertSucceeded();

        // Add input ports and groups.
        st = nodeDef.AddPortGroup(ID_G_100);
        st.AssertSucceeded( ) ;

		st = nodeDef.AddInputPort(ID_IN_Position, ID_G_100, siICENodeDataVector3, siICENodeStructureArray, siICENodeContextAny, L"Points", L"Points", MATH::CVector3f(1.0, 1.0, 1.0), CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
		st.AssertSucceeded();

		st = nodeDef.AddInputPort(ID_IN_Segment, ID_G_100, siICENodeDataLong, siICENodeStructureSingle, siICENodeContextAny, L"Segment", L"Segment", 5, CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
		st.AssertSucceeded();


        // Add output ports.
        st = nodeDef.AddOutputPort(ID_OUT_Position,siICENodeDataVector3, siICENodeStructureArray, siICENodeContextAny,L"Position",L"Position", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
        st.AssertSucceeded( ) ;

		st = nodeDef.AddOutputPort(ID_OUT_Tangent, siICENodeDataVector3, siICENodeStructureArray, siICENodeContextAny, L"Tangent", L"Tangent", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
		st.AssertSucceeded();

		st = nodeDef.AddOutputPort(ID_OUT_U, siICENodeDataFloat, siICENodeStructureArray, siICENodeContextAny, L"U", L"U", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
		st.AssertSucceeded();

		st = nodeDef.AddOutputPort(ID_OUT_NormalizedU, siICENodeDataFloat, siICENodeStructureArray, siICENodeContextAny, L"NormalizedU", L"NormalizedU", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
		st.AssertSucceeded();

		st = nodeDef.AddOutputPort(ID_OUT_Segment, siICENodeDataLong, siICENodeStructureArray, siICENodeContextAny, L"S", L"S", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
		st.AssertSucceeded();

        PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
        nodeItem.PutCategories(L"ICERig");

        return CStatus::OK;
}



SICALLBACK IRCubicBezier_SubmitEvaluationPhaseInfo(ICENodeContext& in_ctxt)
{
	ULONG nPhase = in_ctxt.GetEvaluationPhaseIndex();
	switch (nPhase)
	{
	case 0:
	{
		in_ctxt.AddEvaluationPhaseInputPort(ID_IN_Segment);
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


SICALLBACK IRCubicBezier_Evaluate(ICENodeContext& in_ctxt)
{
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
		for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
		{
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
			CDataArray2DVector3f outputPositionArray(in_ctxt);

			CIndexSet indexSet(in_ctxt);

			for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
			{
				IRCubicBezier_t& crv = data->m_curves[it.GetAbsoluteIndex()];
				// output
				CDataArray2DVector3f::Accessor outputPosition = outputPositionArray.Resize(it, crv.m_samples.size());

				for (ULONG i = 0; i < crv.m_samples.size(); i++)
				{
					size_t base = crv.m_samples[i].m_i;
					IRComputePointOnBezierCurve(crv.m_position[base],
						crv.m_position[base + 1],
						crv.m_position[base + 2],
						crv.m_position[base + 3],
						crv.m_samples[i].m_u,
						&outputPosition[i]);
				}
			}
		}
		break;

		case ID_OUT_Tangent:
		{
			CDataArray2DVector3f outputTangentArray(in_ctxt);

			CIndexSet indexSet(in_ctxt);

			for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
			{
				IRCubicBezier_t& crv = data->m_curves[it.GetAbsoluteIndex()];
				// output
				CDataArray2DVector3f::Accessor outputTangent = outputTangentArray.Resize(it, crv.m_samples.size());

				outputTangent[0] = crv.m_start_tangent;
				for (ULONG i = 1; i < crv.m_samples.size()-1; i++)
				{
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
			
		}
		break;

		case ID_OUT_U:
		{
			CDataArray2DFloat outputUArray(in_ctxt);

			CIndexSet indexSet(in_ctxt);

			for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
			{
				IRCubicBezier_t& crv = data->m_curves[it.GetAbsoluteIndex()];
				// output
				CDataArray2DFloat::Accessor outputU = outputUArray.Resize(it, crv.m_samples.size());

				for (ULONG i = 0; i < crv.m_samples.size(); i++)
				{
					outputU[i] = crv.m_samples[i].m_u;
				}
			}
		}
		break;

		case ID_OUT_NormalizedU:
		{
			CDataArray2DFloat outputNormalizedUArray(in_ctxt);

			CIndexSet indexSet(in_ctxt);

			for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
			{
				IRCubicBezier_t& crv = data->m_curves[it.GetAbsoluteIndex()];
				// output
				CDataArray2DFloat::Accessor outputU = outputNormalizedUArray.Resize(it, crv.m_samples.size());

				for (ULONG i = 0; i < crv.m_samples.size(); i++)
				{
					outputU[i] = crv.m_samples[i].m_nu;
				}
			}
		}
		break;

		case ID_OUT_Segment:
		{
			CDataArray2DLong outputSArray(in_ctxt);

			CIndexSet indexSet(in_ctxt);

			for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
			{
				IRCubicBezier_t& crv = data->m_curves[it.GetAbsoluteIndex()];
				// output
				CDataArray2DLong ::Accessor outputS = outputSArray.Resize(it, crv.m_samples.size());

				for (ULONG i = 0; i < crv.m_samples.size(); i++)
				{
					outputS[i] = crv.m_samples[i].m_i;
				}
			}
		}
		break;
	}
	
	return CStatus::OK;
}


SICALLBACK IRCubicBezier_Init(CRef& in_ctxt)
{
	Context ctxt(in_ctxt);
	IRCubicBezierData_t* data = new IRCubicBezierData_t();
	LOG("MOTHERFUCKIN INIT CUBIC BEZIER!!!!");

	ctxt.PutUserData((CValue::siPtrType)data);
	return CStatus::OK;
}

SICALLBACK IRCubicBezier_Term(CRef& in_ctxt)
{
	Context ctxt(in_ctxt);
	IRCubicBezierData_t* data = (IRCubicBezierData_t*)(CValue::siPtrType)ctxt.GetUserData();
	if(data)delete data;
	ctxt.PutUserData((CValue)NULL);
	LOG("MOTHERFUCKIN KILL CUBIC BEZIER!!!!");
	return CStatus::OK;
}