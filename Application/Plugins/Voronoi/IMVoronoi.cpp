//---------------------------------------------------
#include "IMVoronoi.h"

struct IMVoronoiData_t {
	HEMesh* m_mesh;
};

CStatus RegisterIMVoronoi( PluginRegistrar& in_reg )
{
        ICENodeDef nodeDef;
        nodeDef = Application().GetFactory().CreateICENodeDef(L"IMVoronoi",L"IMVoronoi");

        CStatus st;
        st = nodeDef.PutColor(200,180,160);
        st.AssertSucceeded( ) ;

		st = nodeDef.PutThreadingModel(XSI::siICENodeSingleThreading);
		st.AssertSucceeded();

        // Add input ports and groups.
        st = nodeDef.AddPortGroup(ID_G_100);
        st.AssertSucceeded( ) ;

		st = nodeDef.AddInputPort(ID_IN_Mesh, ID_G_100, siICENodeDataGeometry, siICENodeStructureSingle, siICENodeContextSingleton, L"Mesh", L"Mesh", MATH::CVector3f(1.0, 1.0, 1.0), CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
		st.AssertSucceeded();

		st = nodeDef.AddInputPort(ID_IN_Points, ID_G_100, siICENodeDataVector3, siICENodeStructureSingle, siICENodeContextAny, L"Points", L"Points", MATH::CVector3f(1.0, 1.0, 1.0), CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_UNDEF);
		st.AssertSucceeded();


        // Add output ports.
        st = nodeDef.AddOutputPort(ID_OUT_Position,siICENodeDataVector3, siICENodeStructureArray, siICENodeContextSingleton,L"Position",L"Position", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
        st.AssertSucceeded( ) ;

		st = nodeDef.AddOutputPort(ID_OUT_Indices, siICENodeDataLong, siICENodeStructureArray, siICENodeContextSingleton, L"Indices", L"Indices", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
		st.AssertSucceeded();

        PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
        nodeItem.PutCategories(L"ICEModeling");

        return CStatus::OK;
}


/*
SICALLBACK IMVoronoi_SubmitEvaluationPhaseInfo(ICENodeContext& in_ctxt)
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
*/

SICALLBACK IMVoronoi_BeginEvaluate(ICENodeContext& in_ctxt)
{
	IMVoronoiData_t* data = (IMVoronoiData_t*)(CValue::siPtrType)in_ctxt.GetUserData();

	CICEGeometry geom(in_ctxt, ID_IN_Mesh);

	CDoubleArray points;
	geom.GetPointPositions(points);

	CLongArray polyCount;
	CLongArray polyIndices;

	geom.GetPolygonIndices(polyCount, polyIndices);
	data->m_mesh->Clean();
	data->m_mesh->SetHEData(points, polyCount, polyIndices);

	LOG("IMVORONOI : HE mESH NUM POINTS :" + CString(data->m_mesh->m_vertices.size()));;;
	
	in_ctxt.PutUserData((CValue::siPtrType)data);
	return CStatus::OK;
}

/*
XSIPLUGINCALLBACK CStatus Sample_BeginEvaluate(ICENodeContext& in_ctxt)
{
	CICEGeometry geom(in_ctxt, ID_IN_GeometryPortID);

	// Geometry data are provided as a flat list of values
	CDoubleArray points;
	geom.GetPointPositions(points);

	XSI::MATH::CMatrix4f transfo;
	geom.GetTransformation(transfo);

	std::vector< MATH::CVector3f >* pUserData = new std::vector< MATH::CVector3f >;

	ULONG nCount = geom.GetPolygonCount();

	CLongArray sizes;       // sizes holds the number of points per polygon
	CLongArray indices;     // indices holds the polygon indices and are used for indexing the point values
	geom.GetPolygonIndices(sizes, indices);

	// Compute the center point of all polygons and save the result in a user data.
	ULONG nOffset = 0;
	for (ULONG i = 0; i<nCount; i++)
	{
		MATH::CVector3f vTotal;
		for (ULONG j = 0; j<(ULONG)sizes[i]; j++, nOffset++)
		{
			MATH::CVector3f v((float)points[indices[nOffset] * 3],
				(float)points[indices[nOffset] * 3 + 1],
				(float)points[indices[nOffset] * 3 + 2]);
			vTotal += v;
		}

		vTotal *= 1.0f / sizes[i];

		// The points are relative to the geometry and must be converted to global coordinates.
		vTotal.MulByMatrix4InPlace(transfo);
		pUserData->push_back(vTotal);
	}

	return CStatus::OK;
}
*/


SICALLBACK IMVoronoi_Evaluate(ICENodeContext& in_ctxt)
{
	IMVoronoiData_t* data = (IMVoronoiData_t*)(CValue::siPtrType)in_ctxt.GetUserData();
	/*
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

	*/


	// The current output port being evaluated...
	ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID();

	switch (out_portID)
	{
	case ID_OUT_Position:
	{
		LOG("UPODATE POSITION");
		/*
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
		*/
	}
	break;

	case ID_OUT_Indices:
	{
		LOG("UPDATE INDICES");
		/*
		CDataArray2DVector3f outputTangentArray(in_ctxt);

		CIndexSet indexSet(in_ctxt);

		for (CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
		{
			IRCubicBezier_t& crv = data->m_curves[it.GetAbsoluteIndex()];
			// output
			CDataArray2DVector3f::Accessor outputTangent = outputTangentArray.Resize(it, crv.m_samples.size());

			outputTangent[0] = crv.m_start_tangent;
			for (ULONG i = 1; i < crv.m_samples.size() - 1; i++)
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
		*/

	}
	break;

	}
	return CStatus::OK;
}


SICALLBACK IMVoronoi_Init(CRef& in_ctxt)
{
	Context ctxt(in_ctxt);
	IMVoronoiData_t* data = new IMVoronoiData_t();
	data->m_mesh = new HEMesh();
	ctxt.PutUserData((CValue::siPtrType)data);
	return CStatus::OK;
}

SICALLBACK IMVoronoi_Term(CRef& in_ctxt)
{
	Context ctxt(in_ctxt);
	IMVoronoiData_t* data = (IMVoronoiData_t*)(CValue::siPtrType)ctxt.GetUserData();
	if (data)
	{
		if (data->m_mesh)delete data->m_mesh;
		delete data;
	}
	ctxt.PutUserData((CValue)NULL);
	return CStatus::OK;
}