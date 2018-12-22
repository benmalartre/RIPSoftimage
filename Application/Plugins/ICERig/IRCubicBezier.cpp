//---------------------------------------------------
#include "IRCubicBezier.h"

void IRCubicBezierDuplicateEndKnots(IRCubicBezier_t* crv)
{
	if (crv->m_position.size() > 1)
	{
		size_t last = crv->m_position.size() - 1;
		crv->m_start_knot.Sub(crv->m_position[0], crv->m_position[1]);
		crv->m_start_knot.AddInPlace(crv->m_position[0]);
		crv->m_end_knot.Sub(crv->m_position[last], crv->m_position[last-1]);
		crv->m_end_knot.AddInPlace(crv->m_position[last]);
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
		float u, u_clamped;
		for (size_t s = 0; s < numSamples; s++)
		{
			u = s * incr * numPatches;
			u_clamped = std::floorf(u);
			crv->m_samples[s].m_u = u - u_clamped;
			crv->m_samples[s].m_i = (size_t)u_clamped;
		}
	}
}

void Interpolate(const CVector3f& p1, const CVector3f&  p2, const CVector3f&  p3, const CVector3f&  p4, float u, CVector3f* result)
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

void IRComputeTangentOnBezierCurve(float s, const CVector3f& p0, const CVector3f& p1, const CVector3f& p2, const CVector3f& p3, CVector3f* t)
{
	float sSquare = s * s;

	CVector3f p0Term;
	p0Term.Scale(-3.0f + 6.0f*s - 3.0f*sSquare, p0);

	CVector3f p1Term;
	p1Term.Scale(3.0f - 12.0f*s + 9.0f*sSquare, p1);

	CVector3f p2Term;
	p2Term.Scale(6.0f*s - 9.0f*sSquare, p2);

	CVector3f p3Term;
	p3Term.Scale(3.0f * sSquare, p3);

	t->Add(p0Term, p1Term);
	t->AddInPlace(p2Term);
	t->AddInPlace(p3Term);
}

CStatus RegisterIRCubicBezier( PluginRegistrar& in_reg )
{
        ICENodeDef nodeDef;
        nodeDef = Application().GetFactory().CreateICENodeDef(L"IRCubicBezier",L"IRCubicBezier");

        CStatus st;
        st = nodeDef.PutColor(ICERigNodeR,ICERigNodeG,ICERigNodeB);
        st.AssertSucceeded( ) ;

		//st = nodeDef.PutThreadingModel(XSI::siICENodeMultiEvaluationPhase);
		//st.AssertSucceeded();

        // Add input ports and groups.
        st = nodeDef.AddPortGroup(ID_G_100);
        st.AssertSucceeded( ) ;

		st = nodeDef.AddInputPort(ID_IN_Position, ID_G_100, siICENodeDataVector3, siICENodeStructureArray, siICENodeContextAny, L"InputPosition", L"InputPosition", MATH::CVector3f(1.0, 1.0, 1.0), CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
		st.AssertSucceeded();

		st = nodeDef.AddInputPort(ID_IN_Segment, ID_G_100, siICENodeDataLong, siICENodeStructureSingle, siICENodeContextAny, L"Segment", L"Segment", 5, CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
		st.AssertSucceeded();


        // Add output ports.
        st = nodeDef.AddOutputPort(ID_OUT_Position,siICENodeDataVector3, siICENodeStructureArray, siICENodeContextAny,L"OutputPosition",L"OutputPosition",ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
        st.AssertSucceeded( ) ;
		/*
        st = nodeDef.AddOutputPort(ID_OUT_Tangent,siICENodeDataVector3, siICENodeStructureArray, siICENodeContextSingletonOrElementGenerator,L"Tangent",L"Tangent",ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
        st.AssertSucceeded( ) ;

        st = nodeDef.AddOutputPort(ID_OUT_Length,siICENodeDataFloat, siICENodeStructureArray, siICENodeContextSingletonOrElementGenerator,L"Length",L"Length",ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
        st.AssertSucceeded( ) ;
		*/
		/*
		st = nodeDef.AddOutputPort(ID_OUT_Rotation, siICENodeDataRotation, siICENodeStructureSingle, siICENodeContextAny, L"Rotation", L"OutputRotation", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
		st.AssertSucceeded();

		st = nodeDef.AddOutputPort(ID_OUT_Scale, siICENodeDataVector3, siICENodeStructureSingle, siICENodeContextAny, L"Scale", L"OutputSCale", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
		st.AssertSucceeded();
		*/

        PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
        nodeItem.PutCategories(L"ICERig");

        return CStatus::OK;
}


bool IRCubicBezier_DirtyState(ICENodeContext& in_ctxt)
{
	/*
	CICEPortState GeoState(in_ctxt, ID_IN_Position);
	bool geoDirty = GeoState.IsDirty(CICEPortState::siAnyDirtyState);
	GeoState.ClearState();

	CICEPortState TipState(in_ctxt, ID_IN_TipPolygon);
	bool tipDirty = TipState.IsDirty(CICEPortState::siAnyDirtyState);
	TipState.ClearState();

	return (geoDirty || tipDirty);
	*/
	return false;
}

/*
SICALLBACK IRCubicBezier_SubmitEvaluationPhaseInfo(ICENodeContext& in_ctxt)
{
	LOG("SUBMIT EVALUATION PHASE INFO");
	ULONG nPhase = in_ctxt.GetEvaluationPhaseIndex();
	switch (nPhase)
	{
	case 0:
	{
		LOG("CASE 0");
		in_ctxt.AddEvaluationPhaseInputPort(ID_IN_Segment);
	}
	break;

	case 1:
	{
		LOG("CASE 1");
		in_ctxt.AddEvaluationPhaseInputPort(ID_IN_Position);
		in_ctxt.SetLastEvaluationPhase();
	}
	break;
	}
	return CStatus::OK;
}
*/

/*
SICALLBACK IRCubicBezier_BeginEvaluate(ICENodeContext& in_ctxt)
{
	IRCubicBezierData_t* data = (IRCubicBezierData_t*)(CValue::siPtrType)in_ctxt.GetUserData();
	in_ctxt.PutUserData((CValue::siPtrType)data);
	return CStatus::OK;

}
*/

SICALLBACK IRCubicBezier_Evaluate( ICENodeContext& in_ctxt )
{
	IRCubicBezierData_t* data = (IRCubicBezierData_t*)(CValue::siPtrType)in_ctxt.GetUserData();
	/*
	LONG nPhase = in_ctxt.GetEvaluationPhaseIndex();
	if (nPhase == 0)
	{
		XSI::siICENodeDataType inSegType;
		XSI::siICENodeStructureType inSegStruct;
		XSI::siICENodeContextType inSegCtxt;

		in_ctxt.GetPortInfo(ID_IN_Segment, inSegType, inSegStruct, inSegCtxt);

		CIndexSet indexSet(in_ctxt);
		CDataArrayLong segmentArray(in_ctxt, ID_IN_Segment);
		
		data->m_curves.clear();
		for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
		{
			IRCubicBezier_t seg;
			seg.m_segments = segmentArray[ it.GetAbsoluteIndex()];
			data->m_curves.push_back(seg);
		}
			
		return CStatus::OK;
	}
	*/

	// The current output port being evaluated...
	ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );

	switch( out_portID )
	{     
		case ID_OUT_Position:
		{
			CDataArray2DVector3f inputPositionArray(in_ctxt, ID_IN_Position);
			CDataArrayLong segmentArray(in_ctxt, ID_IN_Segment);
			CDataArray2DVector3f outputPositionArray(in_ctxt);
			
			CIndexSet indexSet(in_ctxt);
			IRCubicBezier_t* crv;
			for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
			{
				IRCubicBezier_t crv;
				crv.m_segments = segmentArray[it.GetIndex()];

				IRCubicBezierSetCurveKnots(&crv, inputPositionArray[it]);
				IRCubicBezierDuplicateEndKnots(&crv);
				if (crv.m_valid)
				{			
					IRCubicBezierGetSamples(&crv, crv.m_segments);

					// output
					CDataArray2DVector3f::Accessor outputPosition = outputPositionArray.Resize(it, crv.m_samples.size());

					for (ULONG i = 0; i < crv.m_samples.size(); i++)
					{
						size_t base = crv.m_samples[i].m_i;
						Interpolate(crv.m_position[base], crv.m_position[base + 1], crv.m_position[base + 2], crv.m_position[base + 3], crv.m_samples[i].m_u, &outputPosition[i]);
					}
				}
				
			}
		
		}
		break;

		/*
		case ID_OUT_U:
		{
			XSI::siICENodeDataType outPortType;
			XSI::siICENodeStructureType outPortStruct;
			XSI::siICENodeContextType outPortContext;
			in_ctxt.GetPortInfo(ID_IN_Segment, outPortType, outPortStruct, outPortContext);
			LONG* seg;
			if (outPortStruct == siICENodeStructureSingle)
			{
				CDataArrayLong segment(in_ctxt, ID_IN_Segment);
				seg = &segment[0];
			}
			else
			{
				CDataArray2DLong segmentArray(in_ctxt, ID_IN_Segment);
				CDataArray2DLong::Accessor segments = segmentArray[0];
				seg = &segments[0];
			}

			CDataArray2DVector3f inputPositionArray(in_ctxt, ID_IN_Position);
			CDataArray2DFloat outputUArray(in_ctxt);

			CIndexSet indexSet(in_ctxt);

			for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
			{
				IRCubicBezier_t* crv = &data->m_curves[it.GetAbsoluteIndex()];
				if (crv)
				{
					IRCubicBezierSetCurveKnots(crv, inputPositionArray[it]);
					IRCubicBezierDuplicateEndKnots(crv);
					IRCubicBezierGetSamples(crv, seg[it.GetIndex()]);

					// output
					CDataArray2DFloat::Accessor outputU = outputUArray.Resize(it, crv->m_samples.size());

					for (ULONG i = 0; i < crv->m_samples.size(); i++)
					{
						outputU[i] = crv->m_samples[i].m_u;
					}
				}
			}
		}
		break;
		*/
	}
				
	/*
        case ID_OUT_Position :
        {
				in_ctxt.GetArrayAttribute()
                // Get the output port array ...                        
                CDataArrayVector3f outData( in_ctxt );
                        
                // Get the input data buffers for each port
                CDataArrayFloat SData( in_ctxt, ID_IN_S );
                CDataArrayVector3f P0Data( in_ctxt, ID_IN_P0 );
                CDataArrayVector3f P1Data( in_ctxt, ID_IN_P1 );
                CDataArrayVector3f P2Data( in_ctxt, ID_IN_P2 );
                CDataArrayVector3f P3Data( in_ctxt, ID_IN_P3 );
                CDataArrayLong SegmentCountData( in_ctxt, ID_IN_SegmentCount);

                // We need a CIndexSet to iterate over the data                 
                CIndexSet indexSet( in_ctxt );
                for(CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
                {
                        // Add code to set output port...
                        ULONG index = it;
                                
                        float s = SData[index];
                        XSI::MATH::CVector3f p0 = P0Data[index];
                        XSI::MATH::CVector3f p1 = P1Data[index];
                        XSI::MATH::CVector3f p2 = P2Data[index];
                        XSI::MATH::CVector3f p3 = P3Data[index];
                        long segmentCount = SegmentCountData[index];
                        if (segmentCount < 1)
                                segmentCount = 1;

                        std::vector<float> lengths;
                        IRComputeLinearSegmentsLengths(segmentCount, p0, p1, p2, p3, &lengths);
                        float t = 0.0f;

                        for (int i = 1; i <= segmentCount; ++i)
                        {
                                float tnMinusOne = 1.0f * (i-1) / segmentCount;
                                float tn = 1.0f * i / segmentCount;

                                if (s >= (lengths[i-1]/lengths[segmentCount]) && s <= (lengths[i]/lengths[segmentCount]))
                                {
                                        t = tnMinusOne + (s - lengths[i-1]/lengths[segmentCount]) * (tn - tnMinusOne) / (lengths[i]/lengths[segmentCount] - lengths[i-1]/lengths[segmentCount]);
                                }
                        }

                        XSI::MATH::CVector3f p;

                        IRComputePositionOnBezierCurve(t, p0, p1, p2, p3, &p);                                                                                            
                        outData[index].Set(p.GetX(), p.GetY(), p.GetZ());
                }
        }
        break;

				
        case ID_OUT_Tangent:
        {
                // Get the output port array ...                        
                CDataArrayVector3f outData( in_ctxt );
                        
                // Get the input data buffers for each port
                CDataArrayFloat SData( in_ctxt, ID_IN_S );
                CDataArrayVector3f P0Data( in_ctxt, ID_IN_P0 );
                CDataArrayVector3f P1Data( in_ctxt, ID_IN_P1 );
                CDataArrayVector3f P2Data( in_ctxt, ID_IN_P2 );
                CDataArrayVector3f P3Data( in_ctxt, ID_IN_P3 );
                CDataArrayLong SegmentCountData( in_ctxt, ID_IN_SegmentCount);

                // We need a CIndexSet to iterate over the data                 
                CIndexSet indexSet( in_ctxt );
                for(CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
                {
                        // Add code to set output port...
                        ULONG index = it;
                                
                        float s = SData[index];
                        if (s < 0.0f)
                                s = 0.0f;
                        if (s > 1.0f)
                                s = 1.0f;

                        XSI::MATH::CVector3f p0 = P0Data[index];
                        XSI::MATH::CVector3f p1 = P1Data[index];
                        XSI::MATH::CVector3f p2 = P2Data[index];
                        XSI::MATH::CVector3f p3 = P3Data[index];
                        long segmentCount = SegmentCountData[index];
                        if (segmentCount < 1)
                                segmentCount = 1;

                        std::vector<float> lengths;
                        IRComputeLinearSegmentsLengths(segmentCount, p0, p1, p2, p3, &lengths);
                        float t = 0.0f;

                        for (int i = 1; i <= segmentCount; ++i)
                        {
                                float tnMinusOne = 1.0f * (i-1) / segmentCount;
                                float tn = 1.0f * i / segmentCount;

                                if (s >= (lengths[i-1]/lengths[segmentCount]) && s <= (lengths[i]/lengths[segmentCount]))
                                {
                                        t = tnMinusOne + (s - lengths[i-1]/lengths[segmentCount]) * (tn - tnMinusOne) / (lengths[i]/lengths[segmentCount] - lengths[i-1]/lengths[segmentCount]);
                                }
                        }

                        XSI::MATH::CVector3f tangent;
                        IRComputeTangentOnBezierCurve(t, p0, p1, p2, p3, &tangent);

                        outData[index].Set(tangent.GetX(), tangent.GetY(), tangent.GetZ());
                }
        }
        break;
                
        case ID_OUT_Length:
        {
                // Get the output port array ...                        
                CDataArrayFloat outData( in_ctxt );
                        
                // Get the input data buffers for each port
                CDataArrayFloat UData( in_ctxt, ID_IN_U );
                CDataArrayVector3f PositionData( in_ctxt, ID_IN_Position );
                CDataArrayLong SegmentCountData( in_ctxt, ID_IN_SegmentCount);

                // We need a CIndexSet to iterate over the data                 
                CIndexSet indexSet( in_ctxt );
                for(CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
                {
                        ULONG index = it;
                                
                        float u = UData[index];
                        if (u < 0.0f)u = 0.0f;
                        if (u > 1.0f)u = 1.0f;

                        XSI::MATH::CVector3f p0 = P0Data[index];
                        XSI::MATH::CVector3f p1 = P1Data[index];
                        XSI::MATH::CVector3f p2 = P2Data[index];
                        XSI::MATH::CVector3f p3 = P3Data[index];
                        long segmentCount = SegmentCountData[index];
                        if (segmentCount < 1)
                                segmentCount = 1;

                        std::vector<float> lengths;
                        IRComputeLinearSegmentsLengths(segmentCount, p0, p1, p2, p3, &lengths);
                                
                        float lengthAtS = u * lengths[segmentCount];

                        outData[index] = lengthAtS;
                }
        }
				
                
        // Other output ports...
};
*/
        
return CStatus::OK;
}


SICALLBACK IRCubicBezier_Init(CRef& in_ctxt)
{
	Context ctxt(in_ctxt);
	IRCubicBezierData_t* data = new IRCubicBezierData_t();
	LOG("MOTHERFUCKIN INIT!!!!");

	ctxt.PutUserData((CValue::siPtrType)data);
	return CStatus::OK;
}

SICALLBACK IRCubicBezier_Term(CRef& in_ctxt)
{
	Context ctxt(in_ctxt);
	IRCubicBezierData_t* data = (IRCubicBezierData_t*)(CValue::siPtrType)ctxt.GetUserData();
	if(data)delete data;
	ctxt.PutUserData((CValue)NULL);
	return CStatus::OK;
}