//---------------------------------------------------
#include "IMVoronoi.h"

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

		st = nodeDef.AddInputPort(ID_IN_Offset, ID_G_100, siICENodeDataFloat, siICENodeStructureSingle, siICENodeContextSingleton, L"Offset", L"Offset", 0.0f, CValue(), CValue(), ID_UNDEF, ID_UNDEF, ID_UNDEF);
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

SICALLBACK IMVoronoi_BeginEvaluate(ICENodeContext& in_ctxt)
{
	IMVoronoiData_t* data = (IMVoronoiData_t*)(CValue::siPtrType)in_ctxt.GetUserData();

	for (std::vector<HEMesh*>::iterator it = data->m_voronoiCells.begin(); it < data->m_voronoiCells.end(); it++)
		(*it)->Clean();
	data->m_voronoiCells.clear();

	CICEGeometry geom(in_ctxt, ID_IN_Mesh);

	CDoubleArray vertices;
	geom.GetPointPositions(vertices);

	CLongArray polyCount;
	CLongArray polyIndices;
	geom.GetPolygonIndices(polyCount, polyIndices);

	data->m_mesh->Clean();
	data->m_mesh->SetHEData(
		(double*)&vertices[0],
		vertices.GetCount() / 3,
		(uint32_t*)&polyCount[0],
		polyCount.GetCount(),
		(uint32_t*)&polyIndices[0]
	);
	
	CDataArrayFloat offsetArray(in_ctxt, ID_IN_Offset);
	data->m_offset = offsetArray[0];

	CDataArrayVector3f pointsArray(in_ctxt, ID_IN_Points);
	ULONG numPoints = pointsArray.GetCount();
	data->m_voronoiCells.resize(numPoints);
	for (size_t i = 0; i < data->m_voronoiCells.size(); i++)
	{
		data->m_voronoiCells[i] = new HEMesh(data->m_mesh);
	}

	HEPlane plane;
	for (LONG i = 0; i<numPoints; i++)
	{
		for (LONG j = 0; j<numPoints; j++)
		{
			if (i != j)
			{
				HEPoint P(
					pointsArray[i].GetX(),
					pointsArray[i].GetY(),
					pointsArray[i].GetZ()
				);
				HEPoint N(
					pointsArray[j].GetX() - P.m_x,
					pointsArray[j].GetY() - P.m_y,
					pointsArray[j].GetZ() - P.m_z
				);
				HEPointNormalize(N);
				HEPoint O(
					(pointsArray[j].GetX() + P.m_x) * 0.5 + N.m_x * - data->m_offset,
					(pointsArray[j].GetY() + P.m_y) * 0.5 + N.m_x * - data->m_offset,
					(pointsArray[j].GetZ() + P.m_z) * 0.5 + N.m_x * - data->m_offset
				);
				plane.Set(O, N);
				data->m_voronoiCells[i]->SplitMesh(plane, P);
			}
		}
		data->m_voronoiCells[i]->AddFragments();
	}
	
	// build mesh
	data->m_outVertices.clear();
	data->m_outIndices.clear();

	LONG offsetpolygonindex = 0;
	LONG polygoncount = 0;
	LONG nbp = 0;

	for (int i = 0; i<numPoints; i++)
	{
		// there should be at least one tetrahedron that is four triangle 
		if (data->m_voronoiCells[i]->m_polygondata.size() >= 16)
		{
			// Add Vertices
			for (LONG j = 0; j<data->m_voronoiCells[i]->m_verticedata.size(); j++)
			{
				data->m_outVertices.push_back(CVector3f(
					data->m_voronoiCells[i]->m_verticedata[j].m_x,
					data->m_voronoiCells[i]->m_verticedata[j].m_y,
					data->m_voronoiCells[i]->m_verticedata[j].m_z
				));
			}

			LONG nb = (LONG)data->m_voronoiCells[i]->m_polygondata.size();
			LONG start, end;
			start = end = 0;
			do
			{
				nbp = data->m_voronoiCells[i]->m_polygondata[start];
				start++;
				end = start + nbp;
				for (LONG k = start; k<end; k++)
				{
					data->m_outIndices.push_back(data->m_voronoiCells[i]->m_polygondata[k] + offsetpolygonindex);
				}
				data->m_outIndices.push_back(-2);
				nb -= (nbp + 1);
				start = end;
			} while (nb>0);

			offsetpolygonindex += (LONG)(data->m_voronoiCells[i]->m_verticedata.size());
		}
	}
	
	in_ctxt.PutUserData((CValue::siPtrType)data);
	return CStatus::OK;
}

SICALLBACK IMVoronoi_Evaluate(ICENodeContext& in_ctxt)
{
	IMVoronoiData_t* data = (IMVoronoiData_t*)(CValue::siPtrType)in_ctxt.GetUserData();

	ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID();

	switch (out_portID)
	{
	case ID_OUT_Position:
	{
		CDataArray2DVector3f outputPositionArray(in_ctxt);
		CDataArray2DVector3f::Accessor outputPosition = outputPositionArray.Resize(0, data->m_outVertices.size());
		for (size_t i=0;i<data->m_outVertices.size();i++)
		{
			outputPosition[i] = data->m_outVertices[i];
		}
	}
	break;

	case ID_OUT_Indices:
	{
		CDataArray2DLong outputIndicesArray(in_ctxt);
		CDataArray2DLong::Accessor outputIndices = outputIndicesArray.Resize(0, data->m_outIndices.size());
		for (size_t i = 0; i<data->m_outIndices.size(); i++)
		{
			outputIndices[i] = data->m_outIndices[i];
		}

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