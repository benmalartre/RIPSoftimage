//---------------------------------------------------
#include "IRCubicBezier.h"

CStatus RegisterIRCubicBezierGenerator( PluginRegistrar& in_reg )
{
        ICENodeDef nodeDef;
        nodeDef = Application().GetFactory().CreateICENodeDef(L"IRCubicBezierGenerator",L"IRCubicBezierGenerator");

        CStatus st;
        st = nodeDef.PutColor(ICERigNodeR,ICERigNodeG,ICERigNodeB);
        st.AssertSucceeded( ) ;

		//st = nodeDef.PutThreadingModel(XSI::siICENodeMultiEvaluationPhase);
		st = nodeDef.PutThreadingModel(XSI::siICENodeSingleThreading);
		st.AssertSucceeded();

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


SICALLBACK IRCubicBezierGenerator_BeginEvaluate(ICENodeContext& in_ctxt)
{
	IRCubicBezierData_t* data = (IRCubicBezierData_t*)(CValue::siPtrType)in_ctxt.GetUserData();

	CDataArrayLong segments(in_ctxt, ID_IN_Segment);
	data->m_numElements = 0;
	data->m_curves.clear();
	for (ULONG i = 0; i < segments.GetCount(); i++)
	{
		IRCubicBezier_t crv;
		crv.m_segments = segments[i];
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

	// The current output port being evaluated...
	ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );
	switch( out_portID )
	{     
		case ID_OUT_Position:
		{
			CDataArray2DVector3f inputPositionArray(in_ctxt, ID_IN_Position);
			CDataArrayLong segmentArray(in_ctxt, ID_IN_Segment);
			//CDataArrayLong indexArray(in_ctxt, ID_IN_Index);
			CDataArray2DVector3f outputPositionArray(in_ctxt);
			
			CIndexSet indexSet(in_ctxt);
					
			for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
			{
				IRCubicBezier_t* crv =& data->m_curves[it.GetIndex()];
				IRCubicBezierSetCurveKnots(crv, inputPositionArray[it]);
				IRCubicBezierDuplicateEndKnots(crv);
				if (crv->m_valid)
				{			
					IRCubicBezierGetSamples(crv, segmentArray[it]);

					// output
					CDataArray2DVector3f::Accessor outputPosition = outputPositionArray.Resize(it, crv->m_samples.size());

					for (ULONG i = 0; i < crv->m_samples.size(); i++)
					{
						size_t base = crv->m_samples[i].m_i;
						Interpolate(crv->m_position[base], crv->m_position[base + 1], crv->m_position[base + 2], crv->m_position[base + 3], crv->m_samples[i].m_u, &outputPosition[i]);
					}
					LOG("BUILD CURVE : " + CString(it.GetIndex()));
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


SICALLBACK IRCubicBezierGenerator_Init(CRef& in_ctxt)
{
	Context ctxt(in_ctxt);
	IRCubicBezierData_t* data = new IRCubicBezierData_t();
	LOG("MOTHERFUCKIN INIT!!!!");

	ctxt.PutUserData((CValue::siPtrType)data);
	return CStatus::OK;
}

SICALLBACK IRCubicBezierGenerator_Term(CRef& in_ctxt)
{
	Context ctxt(in_ctxt);
	IRCubicBezierData_t* data = (IRCubicBezierData_t*)(CValue::siPtrType)ctxt.GetUserData();
	if(data)delete data;
	LOG("MOTHERFUCKIN KILL!!!!");
	ctxt.PutUserData((CValue)NULL);
	return CStatus::OK;
}