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
	//if(m_numPoints != nb)
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
	for (ULONG i = 0; i < m_voronoiCells.size(); i++)
	{
		m_voronoiCells[i].SetHEData(m_containerVertices, m_containerPolygons);
	}

	m_outVertices = m_containerVertices;
	m_outPolygons = m_containerPolygons;
	int base = m_outVertices.GetCount();

	for (int i=0;i<m_numPoints;i++)
	{
		for(int j=0;j<m_numPoints;j++)
		{
			  if(i!=j)
			  {
					CVector3 N;
					CVector3 Offset;
					N.Sub(m_positions[j],m_positions[i]); // plane normal=normalized vector pinting from point i to point j
					N.NormalizeInPlace();
					CVector3 O;
					O.Add(m_positions[j],m_positions[i]); // plane origin=point halfway between point i and point j
					O.ScaleInPlace(0.5);
					Offset.Scale(-m_offset,N);
					O.AddInPlace(Offset);
					m_P.Set(O,N);

					m_voronoiCells[i].SplitMesh(m_P,m_positions[i]);
			  }
		}
		m_voronoiCells[i].AddFragments();
	} 
}

void CVoronoi::BuildMesh()
{
	
	m_outVertices.Clear();
	m_outPolygons.Clear();

	int offsetpolygonindex = 0;
	int polygoncount = 0;
	int nbp = 0;

	for(int i=0;i<m_numPoints;i++)
	{
		// Add Vertices
		for(int j=0;j<m_voronoiCells[i].m_verticedata.GetCount();j++)
		{  
			m_outVertices.Add(m_voronoiCells[i].m_verticedata[j]);
		}
		
		int nb = m_voronoiCells[i].m_polygondata.GetCount();
		int start, end;
		start = end = 0;
		do
		{
			nbp = m_voronoiCells[i].m_polygondata[start];
			start++;
			end = start+nbp;

			m_outPolygons.Add(nbp);
			for(int k=start;k<end;k++)
			{
				m_outPolygons.Add(m_voronoiCells[i].m_polygondata[k]+offsetpolygonindex);
			}
			nb -= (nbp+1);
			start = end;
		}while(nb>0);

		offsetpolygonindex+=(m_voronoiCells[i].m_verticedata.GetCount());
	}
}