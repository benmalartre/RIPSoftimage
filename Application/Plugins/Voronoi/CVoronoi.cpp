// CVoronoi.cpp
//-------------------------------------------
#include  "Voronoi.h"

void CVoronoi::InitVoronoi(LONG nb)
{
	CleanVoronoi();
	m_numPoints = nb;
	m_voronoiCells.resize(m_numPoints);
}

void CVoronoi::CleanVoronoi()
{
	for (std::vector<HEMesh>::iterator it = m_voronoiCells.begin(); it < m_voronoiCells.end(); it++)
		(*it).Clean();
	m_voronoiCells.clear();
}

CStatus CVoronoi::Update( OperatorContext& in_ctxt )
{
	CustomOperator op = in_ctxt.GetSource();
	Primitive inPrim(in_ctxt.GetInputValue(0,0,0));
	PolygonMesh inGeom(inPrim.GetGeometry());

	inGeom.Get(m_containerVertices,m_containerPolygons);

	Primitive inCloud(in_ctxt.GetInputValue(1,0,0));
	Geometry inCloudGeom(inCloud.GetGeometry());
	m_positions = inCloudGeom.GetPoints().GetPositionArray();

	m_offset = in_ctxt.GetParameterValue(L"Offset");

	LONG nb = m_positions.GetCount();

	InitVoronoi(nb);
	BuildVoronoi();
	BuildMesh();

	Primitive output = in_ctxt.GetOutputTarget();
	PolygonMesh outputMesh = output.GetGeometry();
	outputMesh.Set(m_outVertices,m_outPolygons);
	
	return CStatus::OK;
}

void CVoronoi::BuildVoronoi()
{
	for (size_t i = 0; i < m_voronoiCells.size(); i++)
	{
		m_voronoiCells[i].SetHEData(
			(double*)&m_containerVertices[0],
			m_containerVertices.GetCount(),
			(uint32_t*)&m_containerPolygons[0],
			m_containerPolygons.GetCount()
		);
	}

	for (LONG i=0;i<m_numPoints;i++)
	{
		for(LONG j=0;j<m_numPoints;j++)
		{
			  if(i!=j)
			  {
				  HEPoint P(
					  m_positions[i].GetX(),
					  m_positions[i].GetY(),
					  m_positions[i].GetZ()
				  );
				  HEPoint N(
					  m_positions[j].GetX() - P.m_x,
					  m_positions[j].GetY() - P.m_y,
					  m_positions[j].GetZ() - P.m_z
				  );
				  HEPointNormalize(N);
				  HEPoint O(
					  (m_positions[j].GetX() + P.m_x) * 0.5 + N.m_x * -m_offset,
					  (m_positions[j].GetY() + P.m_y) * 0.5 + N.m_x * -m_offset,
					  (m_positions[j].GetZ() + P.m_z) * 0.5 + N.m_x * -m_offset
				  );
				  m_P.Set(O,N);
				  m_voronoiCells[i].SplitMesh(m_P,P);
			  }
		}
		m_voronoiCells[i].AddFragments();
	} 
}

void CVoronoi::BuildMesh()
{
	m_outVertices.Clear();
	m_outPolygons.Clear();

	LONG offsetpolygonindex = 0;
	LONG polygoncount = 0;
	LONG nbp = 0;

	for(int i=0;i<m_numPoints;i++)
	{
		// there should be at least one tetrahedron that is four triangle 
		if (m_voronoiCells[i].m_polygondata.size() >= 16)
		{
			// Add Vertices
			for (LONG j = 0; j<m_voronoiCells[i].m_verticedata.size(); j++)
			{
				m_outVertices.Add(CVector3(
					m_voronoiCells[i].m_verticedata[j].m_x,
					m_voronoiCells[i].m_verticedata[j].m_y,
					m_voronoiCells[i].m_verticedata[j].m_z
				));
			}

			LONG nb = (LONG)m_voronoiCells[i].m_polygondata.size();
			LONG start, end;
			start = end = 0;
			do
			{
				nbp = m_voronoiCells[i].m_polygondata[start];
				start++;
				end = start + nbp;

				m_outPolygons.Add(nbp);
				for (LONG k = start; k<end; k++)
				{
					m_outPolygons.Add(m_voronoiCells[i].m_polygondata[k] + offsetpolygonindex);
				}
				nb -= (nbp + 1);
				start = end;
			} while (nb>0);

			offsetpolygonindex += (LONG)(m_voronoiCells[i].m_verticedata.size());
		}
	}
}