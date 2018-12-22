/*--------------------------------------------
	Geometry.cpp

	Implementation of Geometry Class
	Implementation of Vertex Class
	Implementation of VirtualVertex Class
	Implementation of Edge Class
	Implementation of Polygon Class

														 $                         $$                    
														$$                       $$$                    
														$$$                     $$$$                    
														$$$$                   $$ $$                    
														$$$$$$eeeeePeee..   z"4-'P$                    
													   J$$$$$dP3"""*$*$$ee$$$$$ " $$F                   
													  z$*.P*$ "- .="         '\  $$$$                   
													.P    'F" *e"               \/$$$                   
													"      4  ^                  ^ $$b                  
														   4$%                    ^$$$r                 
															F                      4$$$                 
														   4                        $$$b                
												.    4$b   P           .ec          $$$$b               
													 $"               4$P""         $$$$$$c             
											   ^    4$$$r 4"          4$eec        4$$$$P"*$e           
									 \         .     $$$   4          ^$$$F        $$$$$E.zd$$$"        
									  ^\       F            r                     d$$P"  *$$$P .^       
										 -.    .        %   ^.                   J$P      $$F.      /   
											  "$\    .z.      -                 $P" ^    .P      .^     
												  ""3$$$$$$e    ".           .d"     ^   $F  -"         
												^   $$$$$$$$L       ^""=="""""      /   ^               
												 -  3$ee= "$"                      /  ."                
												  ^. ^****"                      z"  ^                  
																			   z"  /                    
													  -.                   .e$"  =                      
														^ ec..... ....ee$$*"  .^                        
														 ".  """**"""     .=^                           
															  "  === """                        

															
--------------------------------------------*/
#include "HairRegister.h"
#include "HairGeometry.h"
namespace ICEHAIR {

	IHGeometry::IHGeometry()
	{
		_init = false;
	}

	IHGeometry::~IHGeometry()
	{
		ClearDatas();
	}

	void IHGeometry::ClearDatas()
	{
		_geocheck = false;

		for (LONG a = 0; a < (LONG)_vertices.size(); a++)delete _vertices[a];
		_vertices.clear();
		_nbv = 0;

		for (LONG a = 0; a < (LONG)_edges.size(); a++)delete _edges[a];
		_edges.clear();
		_nbe = 0;

		for (LONG a = 0; a < (LONG)_polygons.size(); a++)delete _polygons[a];
		_polygons.clear();
		_nbp = 0;

		for (LONG a = 0; a < (LONG)_triangles.size(); a++)delete _triangles[a];
		_triangles.clear();
		_nbt = 0;

	}

	void IHGeometry::InitGuideData()
	{
		for (ULONG v = 0; v < _nbv; v++)
		{
			_vertices[v]->_tip = false;
			_vertices[v]->_flow = false;
			_vertices[v]->_checked = false;
			_vertices[v]->_special = false;
		}
	}

	void IHGeometry::GetCachedMeshData(CICEGeometry& geom)
	{
		ClearDatas();
		GetPointPosition(geom);

		//loop over all edges
		_nbe = geom.GetSegmentCount();
		CLongArray sIndices;
		geom.GetSegmentIndices(sIndices);
		_edges.resize(_nbe);

		for (ULONG i = 0; i < _nbe; i++)
		{
			_edges[i] = new IHEdge(i, _vertices[sIndices[i * 2]], _vertices[sIndices[i * 2 + 1]]);
			_vertices[sIndices[i * 2]]->_edges.push_back(_edges[i]);
			_vertices[sIndices[i * 2 + 1]]->_edges.push_back(_edges[i]);
		}

		// loop over triangles	
		_nbt = geom.GetTriangleCount();
		CLongArray tIndices;
		geom.GetTrianglePointIndices(tIndices);
		CVector3f a, b;
		CVector3f p;

		_triangles.resize(_nbt);

		for (ULONG i = 0; i < _nbt; i++)
		{
			_triangles[i] = new IHTriangle(i, _vertices[tIndices[i * 3]], _vertices[tIndices[i * 3 + 1]], _vertices[tIndices[i * 3 + 2]]);

			// Get Normal
			a.Sub(_vertices[tIndices[i * 3]]->_pos, _vertices[tIndices[i * 3 + 1]]->_pos);
			b.Sub(_vertices[tIndices[i * 3]]->_pos, _vertices[tIndices[i * 3 + 2]]->_pos);
			_triangles[i]->_norm.Cross(a, b);
			_triangles[i]->_norm.NormalizeInPlace();

			for (LONG j = 0; j < 3; j++)
			{
				// find connected triangles
				_vertices[tIndices[i * 3 + j]]->_triangles.push_back(_triangles[i]);
				_triangles[i]->_vertices.push_back(_vertices[tIndices[i * 3 + j]]);
			}
		}

		// loop over polygons
		_nbp = geom.GetPolygonCount();

		CLongArray pSizes;
		CLongArray pIndices;
		geom.GetPolygonIndices(pSizes, pIndices);
		CLongArray pid;
		IHPolygon* polygon;
		std::vector<IHEdge*> pe;
		std::vector<IHTriangle*> pt;

		LONG nOffset = 0;
		for (ULONG i = 0; i < _nbp; i++)
		{
			pid.Clear();
			for (LONG j = 0; j < (long)pSizes[i]; j++, nOffset++)
			{
				pid.Add(pIndices[nOffset]);
			}
			polygon = new IHPolygon(i, pid);
			polygon->_nbv = pid.GetCount();
			if (polygon->_nbv != 4)_geocheck = true;

			for (LONG j = 0; j < pid.GetCount(); j++)
			{
				polygon->PushVertexData(_vertices[pid[j]]);
				pe = _vertices[pid[j]]->_edges;
				for (LONG k = 0; k < (LONG)pe.size(); k++)
				{
					polygon->PushEdgeData(pe[k]);
				}

				pt = _vertices[pid[j]]->_triangles;
				for (LONG k = 0; k < (LONG)pt.size(); k++)
				{
					polygon->PushTriangleData(pt[k]);
				}
			}
			_polygons.push_back(polygon);
		}

		//reloop over all edges getting boundaries edge
		for (ULONG i = 0; i < _nbe; i++)
		{
			IHEdge* e = _edges[i];

			bool isboundary = e->_polygons.size() < 2;
			e->_boundary = isboundary;
			_vertices[e->_vertices[0]->_id]->_boundary = isboundary;
			_vertices[e->_vertices[1]->_id]->_boundary = isboundary;
		}

		//reloop over vertices getting neighbors;
		// we also set initial pos
		std::vector<IHVertex*>::iterator vi;
		for (vi = _vertices.begin(); vi < _vertices.end(); vi++)
		{
			(*vi)->GetNeighbors();
			(*vi)->_initpos = (*vi)->_pos;
			(*vi)->_initnorm = (*vi)->_norm;
		}

		// init mesh data flag
		_init = true;

	}

	void IHGeometry::GetCachedMeshData(X3DObject& obj)
	{
		PolygonMesh geom = obj.GetActivePrimitive().GetGeometry();
		KinematicState kine = obj.GetKinematics().GetGlobal();

		ClearDatas();
		GetPointPosition(geom, kine);

		CEdgeRefArray edges = geom.GetEdges();
		Edge e;
		CVertexRefArray vertices;
		Vertex v;
		ULONG a, b;

		//loop over all edges
		_nbe = edges.GetCount();

		_edges.resize(_nbe);

		for (ULONG i = 0; i < _nbe; i++)
		{
			e = edges[i];
			vertices = e.GetVertices();
			v = vertices.GetItem(0);
			a = v.GetIndex();
			v = vertices.GetItem(1);
			b = v.GetIndex();
			_edges[i] = new IHEdge(i, _vertices[a], _vertices[b]);
			_vertices[a]->_edges.push_back(_edges[i]);
			_vertices[b]->_edges.push_back(_edges[i]);
		}

		// loop over triangles	
		CTriangleRefArray triangles = geom.GetTriangles();
		_nbt = triangles.GetCount();
		Triangle t;
		LONG p1, p2, p3;
		Point pnt;

		CVector3f d1, d2;
		CVector3f p;

		_triangles.resize(_nbt);
		CTriangleVertexRefArray pnts;
		CLongArray pid;
		pid.Resize(3);

		for (ULONG i = 0; i < _nbt; i++)
		{
			t = triangles.GetItem(i);
			pnts = t.GetPoints();
			pnt = pnts.GetItem(0);
			p1 = pnt.GetIndex();
			pid[0] = p1;

			pnt = pnts.GetItem(1);
			p2 = pnt.GetIndex();
			pid[1] = p2;

			pnt = pnts.GetItem(2);
			p3 = pnt.GetIndex();
			pid[2] = p3;

			_triangles[i] = new IHTriangle(i, _vertices[p1], _vertices[p2], _vertices[p3]);

			// Get Normal
			d1.Sub(_vertices[p1]->_pos, _vertices[p2]->_pos);
			d2.Sub(_vertices[p1]->_pos, _vertices[p3]->_pos);
			_triangles[i]->_norm.Cross(d1, d2);
			_triangles[i]->_norm.NormalizeInPlace();

			for (LONG j = 0; j < 3; j++)
			{
				// find connected triangles
				_vertices[pid[j]]->_triangles.push_back(_triangles[i]);
				_triangles[i]->_vertices.push_back(_vertices[pid[j]]);
			}
		}
		/*
			// loop over polygons
			_nbp = geom.GetPolygonCount();

			CLongArray pSizes;
			CLongArray pIndices;
			geom.GetPolygonIndices( pSizes, pIndices );
			CLongArray pid;
			Polygon* polygon;
			std::vector<Edge*> pe;
			std::vector<Triangle*> pt;

			LONG nOffset = 0;
			for (ULONG i=0; i<_nbp; i++)
			{
				pid.Clear();
				for (LONG j=0; j<(long)pSizes[i]; j++, nOffset++)
				{
					pid.Add(pIndices[nOffset]);
				}
				polygon = new Polygon(i,pid);
				polygon->_nbv = pid.GetCount();
				if(polygon->_nbv!=4)_geocheck=true;

				for (LONG j=0; j<pid.GetCount(); j++)
				{
					polygon->PushVertexData(_vertices[pid[j]]);
					pe = _vertices[pid[j]]->_edges;
					for(LONG k=0;k<(LONG)pe.size();k++)
					{
						polygon->PushEdgeData(pe[k]);
					}

					pt = _vertices[pid[j]]->_triangles;
					for(LONG k=0;k<(LONG)pt.size();k++)
					{
						polygon->PushTriangleData(pt[k]);
					}
				}
				_polygons.push_back(polygon);
			}

			//reloop over all edges getting boundaries edge
			for (ULONG i=0; i<_nbe; i++)
			{
				Edge* e = _edges[i];

				bool isboundary = e->_polygons.size()<2;
				e->_boundary = isboundary;
				_vertices[e->_vertices[0]->_id]->_boundary = isboundary;
				_vertices[e->_vertices[1]->_id]->_boundary = isboundary;
			}

			//reloop over vertices getting neighbors;
			std::vector<Vertex*>::iterator vi;
			for(vi=_vertices.begin();vi<_vertices.end();vi++)
			{
				(*vi)->GetNeighbors();
			}

			// init mesh data flag
			_init = true;
			*/
	}

	void IHGeometry::GetPointPosition(CICEGeometry& geom)
	{
		CDoubleArray points;
		geom.GetPointPositions(points);
		LONG pCount = geom.GetPointPositionCount();
		CMatrix4f mat;
		geom.GetTransformation(mat);
		CVector3f p;

		if (_nbv != pCount)
		{
			// Clean old data
			for (ULONG v = 0; v < _vertices.size(); v++)delete _vertices[v];

			_nbv = pCount;
			_vertices.resize(_nbv);

			// loop over vertices
			for (ULONG i = 0; i < _nbv; i++)
			{
				p.Set((float)points[i * 3], (float)points[i * 3 + 1], (float)points[i * 3 + 2]);
				p.MulByMatrix4InPlace(mat);
				_vertices[i] = new IHVertex(i, p.GetX(), p.GetY(), p.GetZ());
			}
		}

		else
		{
			// loop over vertices
			for (ULONG i = 0; i < _nbv; i++)
			{
				p.Set((float)points[i * 3], (float)points[i * 3 + 1], (float)points[i * 3 + 2]);
				p.MulByMatrix4InPlace(mat);
				_vertices[i]->_pos.Set(p.GetX(), p.GetY(), p.GetZ());
			}
		}
	}

	void IHGeometry::GetPointPosition(PolygonMesh& geom, KinematicState& kine)
	{
		CPointRefArray points(geom.GetPoints());
		LONG pCount = points.GetCount();

		CTransformation tra = kine.GetTransform();
		Point pnt;
		CVector3 p;

		if (_nbv != pCount)
		{
			// Clean old data
			for (ULONG v = 0; v < _vertices.size(); v++)delete _vertices[v];

			_nbv = pCount;
			_vertices.resize(_nbv);

			// loop over vertices
			for (ULONG i = 0; i < _nbv; i++)
			{
				pnt = points[i];
				p = pnt.GetPosition();
				p.MulByTransformationInPlace(tra);
				_vertices[i] = new IHVertex(i, (float)p.GetX(), (float)p.GetY(), (float)p.GetZ());
			}
		}

		else
		{
			// loop over vertices
			for (ULONG i = 0; i < _nbv; i++)
			{
				pnt = points[i];
				p = pnt.GetPosition();
				p.MulByTransformationInPlace(tra);
				_vertices[i]->_pos.Set((float)p.GetX(), (float)p.GetY(), (float)p.GetZ());
			}
		}
	}

	void IHGeometry::UncheckVertices()
	{
		for (std::vector<IHVertex*>::iterator it = _vertices.begin(); it < _vertices.end(); it++)
		{
			(*it)->_checked = false;
		}
	}

	void IHGeometry::UncheckEdges()
	{
		for (std::vector<IHEdge*>::iterator it = _edges.begin(); it < _edges.end(); it++)
		{
			(*it)->_checked = false;
		}
	}

	void IHGeometry::UnbranchEdges()
	{
		// clear _branches array
		for (std::vector<IHEdge*>::iterator it = _edges.begin(); it < _edges.end(); it++)
		{
			(*it)->_branches.clear();
		}
	}

	void IHGeometry::UncheckPolygons()
	{
		for (std::vector<IHPolygon*>::iterator it = _polygons.begin(); it < _polygons.end(); it++)
		{
			(*it)->_checked = false;
		}
	}
	//------------------------------------
	// IHVertex Class
	//------------------------------------
	IHVertex::IHVertex(LONG id, float px, float py, float pz)
	{
		_id = id;
		_pos.Set(px, py, pz);
		_tip = false;
		_flow = false;
		_checked = false;
		_polygons.empty();
	}

	IHVertex::~IHVertex()
	{
		_edges.clear();
		_flowedges.clear();
		_segmentedges.clear();
		_polygons.clear();
		_neighbors.clear();
		_next = NULL;
		_triangles.clear();
	}

	void IHVertex::GetFlowEdges()
	{
		_flowedges.clear();
		_segmentedges.clear();
		_nbneighbors = (ULONG)_edges.size();

		for (ULONG ei = 0; ei < _nbneighbors; ei++)
		{
			if (_edges[ei]->_flow)
				_flowedges.push_back(_edges[ei]);

			if (_edges[ei]->_segment)
				_segmentedges.push_back(_edges[ei]);
		}
	}

	void IHVertex::GetNeighbors()
	{
		_neighbors.clear();
		for (ULONG e = 0; e < _edges.size(); e++)
		{
			if (_edges[e]->_vertices[0]->_id == _id)
				_neighbors.push_back(_edges[e]->_vertices[1]);
			else if (_edges[e]->_vertices[1]->_id == _id)
				_neighbors.push_back(_edges[e]->_vertices[0]);
		}
	}

	bool IHVertex::HasNext()
	{
		return(_next != NULL);
	}

	void IHVertex::GetNext()
	{
		// Vertex is Neighbor vertex connected by flow edge not already checked
		for (ULONG n = 0; n < _neighbors.size(); n++)
		{
			if (_neighbors[n]->_flow && !_neighbors[n]->_checked)
			{
				_next = _neighbors[n];
				return;
			}
		}
		_next = NULL;
	}

	bool IHVertex::IsNeighbor(IHVertex* other)
	{
		std::vector<IHEdge*>::iterator ei;
		for (ei = _edges.begin(); ei < _edges.end(); ei++)
		{
			if ((*ei)->_vertices[0]->_id == other->_id ||
				(*ei)->_vertices[1]->_id == other->_id)
				return true;
		}
		return false;
	}

	void IHVertex::IsSpecial()
	{
		std::vector<IHEdge*>::iterator ei;
		std::vector<IHPolygon*>::iterator pi;
		bool special = true;

		bool tip = true;

		for (pi = _polygons.begin(); pi < _polygons.end(); pi++)
		{
			if (!(*pi)->_tip) { tip = false; break; }
		}

		if (tip) { _special = true; return; }

		for (ei = _edges.begin(); ei < _edges.end(); ei++)
		{
			for (pi = (*ei)->_polygons.begin(); pi < (*ei)->_polygons.end(); pi++)
			{
				if (!(*pi)->_checked && !(*pi)->_tip)special = false;
				break;
			}
		}
		_special = special;
	}

	void IHVertex::PushPolygonData(IHPolygon* poly)
	{
		for (std::vector<IHPolygon*>::iterator it = _polygons.begin(); it < _polygons.end(); it++)
		{
			if ((*it)->_id == poly->_id)
			{
				return;
			}
		}
		_polygons.push_back(poly);
	}

	//------------------------------------
	// IHVirtualVertex Class
	//------------------------------------
	void IHVirtualVertex::GetPosition()
	{
		if (_previous.size() == _vertices.size() && _last != NULL)
		{
			CVector3f d, id;
			_pos = _last->_pos;
			_initpos = _last->_initpos;
			float tl = 0;
			for (ULONG vi = 0; vi < _vertices.size(); vi++)
			{
				d.Sub(_vertices[vi]->_pos, _previous[vi]->_pos);
				id.Sub(_vertices[vi]->_initpos, _previous[vi]->_initpos);
				d.ScaleInPlace(_weights[vi]);
				id.ScaleInPlace(_weights[vi]);
				_pos.AddInPlace(d);
				_initpos.AddInPlace(id);
			}
		}
		else
		{
			CVector3f v, iv;
			_pos.SetNull();
			_initpos.SetNull();
			for (ULONG vi = 0; vi < _vertices.size(); vi++)
			{
				v.Scale(_weights[vi], _vertices[vi]->_pos);
				iv.Scale(_weights[vi], _vertices[vi]->_initpos);
				_pos.AddInPlace(v);
				_initpos.AddInPlace(iv);
			}
		}
	}

	void IHVirtualVertex::GetWeights()
	{
		LONG nb = (LONG)_vertices.size();
		_weights.resize(nb);
		CFloatArray length;
		length.Resize(nb);
		float totallength = 0;
		CVector3f v;

		for (LONG vi = 0; vi < nb; vi++)
		{
			v.Sub(_pos, _vertices[vi]->_pos);
			length[vi] = 1 / v.GetLengthSquared();
			totallength += length[vi];
		}
		for (LONG vi = 0; vi < nb; vi++)
		{
			_weights[vi] = length[vi] / totallength;
		}
	}

	//------------------------------------
	// Edge Class
	//------------------------------------
	IHEdge::IHEdge(LONG id, IHVertex* p1, IHVertex* p2)
	{
		_id = id;
		_vertices.resize(2);
		_vertices[0] = p1;
		_vertices[1] = p2;
		_flow = false;
		_segment = false;
		_tip = false;
		_checked = false;
	}

	IHEdge::~IHEdge()
	{
		_polygons.clear();
		_vertices.clear();
		_branches.clear();
		_next = NULL;
	}

	bool IHEdge::SharePolygon(IHEdge* other)
	{
		if (_polygons[0]->_id == other->_polygons[0]->_id)return true;
		if (_polygons.size() > 1)
		{
			if (_polygons[1]->_id == other->_polygons[0]->_id)return true;
			if (other->_polygons.size() > 1)
			{
				if (_polygons[0]->_id == other->_polygons[1]->_id ||
					_polygons[1]->_id == other->_polygons[1]->_id)return true;
			}
		}
		return false;
	}

	bool IHEdge::ShareVertex(IHEdge* other)
	{
		if (_vertices[0]->_id == other->_vertices[0]->_id ||
			_vertices[0]->_id == other->_vertices[1]->_id ||
			_vertices[1]->_id == other->_vertices[0]->_id ||
			_vertices[1]->_id == other->_vertices[1]->_id)
			return true;

		return false;
	}

	bool IHEdge::BelongToPolygon(IHPolygon* poly)
	{
		std::vector<IHEdge*>::iterator it;
		for (it = poly->_edges.begin(); it < poly->_edges.end(); it++)
		{
			if ((*it)->_id == _id)return true;
		}
		return false;
	}

	bool IHEdge::IsTipBorder()
	{
		if (_polygons.size() > 1)
		{
			if (!(_polygons[0]->_tip && _polygons[1]->_tip))
				return true;
		}
		else
		{
			if (_polygons[0]->_tip)return true;
		}
		return false;
	}

	void IHEdge::PushBranches(ULONG branchid)
	{
		std::vector<LONG>::iterator li;
		for (li = _branches.begin(); li < _branches.end(); li++)
		{
			if ((*li) == branchid)return;
		}
		_branches.push_back(branchid);
	}

	//------------------------------------
	// Triangle Class
	//------------------------------------
	IHTriangle::IHTriangle(LONG id, IHVertex* p1, IHVertex* p2, IHVertex* p3)
	{
		_vertices.resize(3);
		_vertices[0] = p1;
		_vertices[1] = p2;
		_vertices[2] = p3;
		_id = id;
	}

	void IHTriangle::GetArea()
	{
		CVector3f ab, ac, proj, height;
		ab.Sub(_vertices[1]->_pos, _vertices[0]->_pos);
		ac.Sub(_vertices[2]->_pos, _vertices[0]->_pos);

		//project vector to get triangle height
		float dot = ac.Dot(ab);
		proj.Scale(dot / ab.GetLengthSquared(), ab);
		height.Sub(ac, proj);

		float l1 = ab.GetLength();
		float l2 = height.GetLength();

		_area = (l1*l2) / 2;
		//Application().LogMessage(L"Triangle ID "+(CString)_id +L" area : "+(CString)area);
	}

	//------------------------------------
	// Polygon Class
	//------------------------------------
	IHPolygon::IHPolygon(LONG id, XSI::CLongArray pid)
	{
		_id = id;
		_pid = pid;
		_root = true;
		_tip = false;
	}

	IHPolygon::~IHPolygon()
	{
		_guidesid.clear();
		_vertices.clear();
		_edges.clear();
		_triangles.clear();
	}

	void IHPolygon::PushGuideData(ULONG guideid)
	{
		for (ULONG i = 0; i < _guidesid.size(); i++)
		{
			if (_guidesid[i] == guideid)return;
		}
		_guidesid.push_back(guideid);
	}

	void IHPolygon::CleanGuideData()
	{
		_guidesid.clear();
	}

	void IHPolygon::PushVertexData(IHVertex*& vertex)
	{
		vertex->PushPolygonData(this);
		// if vertex already in _vertices return
		for (LONG v = 0; v < (LONG)_vertices.size(); v++)
		{
			if (_vertices[v]->_id == vertex->_id)
				return;
		}
		_vertices.push_back(vertex);
	}

	void IHPolygon::PushEdgeData(IHEdge*& edge)
	{
		// if edge already in _edges return
		for (LONG v = 0; v < (LONG)_edges.size(); v++)
		{
			if (_edges[v]->_id == edge->_id)return;
		}

		// check if edge belong to polygon
		LONG cnt = 0;
		for (LONG v = 0; v < (LONG)_pid.GetCount(); v++)
		{
			if (_pid[v] == edge->_vertices[0]->_id || _pid[v] == edge->_vertices[1]->_id)cnt++;
		}
		if (cnt == 2)
		{
			_edges.push_back(edge);
			edge->_polygons.push_back(this);
		}
	}

	void IHPolygon::PushTriangleData(IHTriangle*& tri)
	{
		// if triangle already in _tiangle return
		for (LONG v = 0; v < (LONG)_triangles.size(); v++)
		{
			if (_triangles[v]->_id == tri->_id)return;
		}

		// check if triangle belong to polygon
		LONG cnt = 0;
		for (LONG v = 0; v < 3; v++)
		{
			for (LONG p = 0; p < _pid.GetCount(); p++)
			{
				if (tri->_vertices[v]->_id == _pid[p])cnt++;
			}
		}
		if (cnt == 3)
		{
			_triangles.push_back(tri);
		}
	}

	void IHPolygon::GetFlow(IHEdge* start)
	{
		std::vector<IHEdge*>::iterator ei;
		ULONG id1 = start->_vertices[0]->_id;
		ULONG id2 = start->_vertices[1]->_id;

		for (ei = _edges.begin(); ei < _edges.end(); ei++)
		{
			// skip starting edge
			if ((*ei)->_id == start->_id)continue;

			if (start->ShareVertex(*ei))
			{
				(*ei)->_flow = true;
				(*ei)->_segment = false;
			}
			else
			{
				(*ei)->_flow = false;
				(*ei)->_segment = true;
			}
		}
	}

	std::vector<IHPolygon*> IHPolygon::GetTipNeighbors()
	{
		std::vector<IHPolygon*> out;
		out.clear();
		std::vector<IHEdge*>::iterator ei;
		std::vector<IHPolygon*>::iterator pi;
		for (ei = _edges.begin(); ei < _edges.end(); ei++)
		{
			for (pi = (*ei)->_polygons.begin(); pi < (*ei)->_polygons.end(); pi++)
			{
				if ((*pi)->_id == _id)continue;

				if ((*pi)->_tip && !(*pi)->_checked)
				{
					(*pi)->_checked = true;
					out.push_back(*pi);
				}
			}
		}
		return out;
	}

}