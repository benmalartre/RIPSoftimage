// HEMesh.cpp
//----------------------------------------------
#include "HEMesh.h"
#include "Voronoi.h"

//------------------------------------------------
// HEPlane Set
//------------------------------------------------
void HEPlane::Set(const HEPoint& p0, const HEPoint& n)
{
	HEPoint norm(n);
	HEPointNormalize(norm);
	m_A = norm.m_x;
	m_B = norm.m_y;
	m_C = norm.m_z;
	m_D = -m_A * p0.m_x - m_B * p0.m_y - m_C * p0.m_z;
};


//------------------------------------------------
// HEPlane Side
//------------------------------------------------
// returns 1 if p is on same side as normal, -1 if on opposite side, 0 if on the plane
float HEPlane::Side(const HEPoint& p)
{
	float tmp = m_A * p.m_x + m_B * p.m_y + m_C * p.m_z + m_D;
	if (tmp<-0.001) {
		tmp = -1;
	}
	else if (tmp>0.001) {
		tmp = 1;
	}
	else {
		tmp = 0;
	}
	return tmp;
};

// HEVertex constructor
//------------------------------------------------
HEVertex::HEVertex(float x, float y, float z, uint32_t id)
{
	m_x = x;
	m_y = y;
	m_z = z;
	m_id = id;
	m_d = false;
	m_halfEdge = NULL;
};

HEVertex::HEVertex(double x, double y, double z, uint32_t id)
{
	m_x = (float)x;
	m_y = (float)y;
	m_z = (float)z;
	m_id = id;
	m_d = false;
	m_halfEdge = NULL;
};

HEVertex::HEVertex(const HEPoint& v, uint32_t id)
{
	m_x = v.m_x;
	m_y = v.m_y;
	m_z = v.m_z;
	m_id = id;
	m_d = false;
	m_halfEdge = NULL;
};


// SplitEdge constructor
//------------------------------------------------
HEVertex* SplitEdge(HEHalfEdge* e, HEPoint p, uint32_t index)
{
	HEVertex* v = new HEVertex(p.m_x, p.m_y, p.m_z, index);
	if (e->m_vert->m_side > 0)
	{
		e->m_next->m_vert = v;
		if (e->m_next->m_pair)e->m_next->m_pair->m_vert = v;
	}
	else
	{
		e->m_vert = v;
		if (e->m_pair)e->m_pair->m_next->m_vert = v;
	}
	return v;
};


//------------------------------------------------
// Plane / Edge Intersection
//------------------------------------------------
// Paul Bourke, http://local.wasp.uwa.edu.au/~pbourke/geometry/planeline/
//------------------------------------------------
bool PlaneEdgeIntersection(HEHalfEdge* e, HEPlane* P, HEPoint& intersection)
{
	HEVertex* p0 = e->m_vert;
	HEVertex* p1 = e->m_next->m_vert;

	double denom = P->m_A*(p0->m_x - p1->m_x) + P->m_B*(p0->m_y - p1->m_y) + P->m_C*(p0->m_z - p1->m_z);
	if ((denom<0.001) && (denom>-0.001)) return false;
	double u = (P->m_A*p0->m_x + P->m_B*p0->m_y + P->m_C*p0->m_z + P->m_D) / denom;
	if ((u<0.00) || (u>1.0)) return false;

	intersection.m_x = p0->m_x + u * (p1->m_x - p0->m_x);
	intersection.m_y = p0->m_y + u * (p1->m_y - p0->m_y);
	intersection.m_z = p0->m_z + u * (p1->m_z - p0->m_z);

	return true;
};

// HEMesh Get Edge Key
//-----------------------------------------
HEUniqueEdgeKey_t HEMesh::GetEdgeKey(HEHalfEdge* e)
{
	uint32_t a = e->m_vert->m_id;
	uint32_t b = e->m_next->m_vert->m_id;
	if (a < b) return std::make_pair(a, b);
	else return std::make_pair(b, a);
}

/*
// HEMesh
//-----------------------------------------
void HEMesh::SetHEData(CVector3Array& vertices, CLongArray& polys)
{
	// Add all input vertices to the mesh, the original index of the vertex is stored as an id.
	for (int i = 0; i<vertices.GetCount(); i++)
	{
		HEVertex* vert = new HEVertex(vertices[i].GetX(), vertices[i].GetY(), vertices[i].GetZ(), i);
		m_vertices.push_back(vert);
	}
	ULONG numVertices = vertices.GetCount();

	// Create a loop of halfedges for each face
	int start, end, nbvertices;
	start = end = nbvertices = 0;
	int nbpolys = polys.GetCount();

	do
	{
		vector<HEHalfEdge*> faceEdges;

		HEFace* hef = new HEFace();
		m_faces.push_back(hef);

		nbvertices = polys[start];
		start++;
		end = start + nbvertices;

		for (int j = start; j<end; j++)
		{
			HEHalfEdge* he = new HEHalfEdge();
			he->m_face = hef;
			if (hef->m_halfEdge == NULL) hef->m_halfEdge = he;
			he->m_vert = m_vertices[polys[j]];
			if (he->m_vert->m_halfEdge == NULL) he->m_vert->m_halfEdge = he;
			faceEdges.push_back(he);
			m_halfEdges.push_back(he);
		}

		int n = (int)faceEdges.size();
		for (int j = 0; j<n - 1; j++)
		{
			HEHalfEdge* he = faceEdges[j];
			he->m_next = faceEdges[j + 1];
		}

		HEHalfEdge* he = faceEdges[n - 1];
		he->m_next = faceEdges[0];

		start = end;
		nbpolys -= (nbvertices + 1);
	} while (nbpolys>0);

	// associate each halfedge with its pair (belonging to adjacent face)
	PairHalfEdges(m_halfEdges);
}
*/

// HEMesh
//-----------------------------------------
void HEMesh::SetHEData(double* vertices, uint32_t numVertices, uint32_t* polys, uint32_t numPolys)
{
	// Add all input vertices to the mesh, the original index of the vertex is stored as an id.
	for (uint32_t i = 0; i<numVertices; i++)
	{
		HEVertex* vert = new HEVertex(vertices[i*3], vertices[i*3+1], vertices[i*3+2], i);
		m_vertices.push_back(vert);
	}

	// Create a loop of halfedges for each face
	uint32_t start, end, nbvertices;
	start = end = nbvertices = 0;
	uint32_t nbpolys = numPolys;
	uint32_t faceid = 0;
	uint32_t edgeid = 0;
	do
	{
		vector<HEHalfEdge*> faceEdges;

		HEFace* hef = new HEFace(faceid);
		faceid++;
		m_faces.push_back(hef);

		nbvertices = polys[start];
		start++;
		end = start + nbvertices;

		for (uint32_t j = start; j<end; j++)
		{
			HEHalfEdge* he = new HEHalfEdge(edgeid);
			edgeid++;
			he->m_face = hef;
			if (hef->m_halfEdge == NULL) hef->m_halfEdge = he;
			he->m_vert = m_vertices[polys[j]];
			if (he->m_vert->m_halfEdge == NULL) he->m_vert->m_halfEdge = he;
			faceEdges.push_back(he);
			m_halfEdges.push_back(he);
		}

		uint32_t n = (uint32_t)faceEdges.size();
		for (uint32_t j = 0; j<n - 1; j++)
		{
			HEHalfEdge* he = faceEdges[j];
			he->m_next = faceEdges[j + 1];
		}

		HEHalfEdge* he = faceEdges[n - 1];
		he->m_next = faceEdges[0];

		start = end;
		nbpolys -= (nbvertices + 1);
	} while (nbpolys>0);

	// associate each halfedge with its pair (belonging to adjacent face)
	PairHalfEdges(m_halfEdges);
}

// HEMesh
//-----------------------------------------
void HEMesh::SetHEData(double* vertices, uint32_t numVertices, uint32_t* polyCount, uint32_t numPolyCount, uint32_t* polyIndices)
{

	// Add all input vertices to the mesh, the original index of the vertex is stored as an id.
	for (uint32_t i = 0; i<numVertices; i++)
	{
		HEVertex* vert = new HEVertex(vertices[i*3], vertices[i*3+1], vertices[i*3+2], i);
		m_vertices.push_back(vert);
	}

	// Create a loop of halfedges for each face
	uint32_t base, nbvertices;
	base = nbvertices = 0;
	uint32_t nbpolys = numPolyCount;
	uint32_t edgeid = 0;
	for(uint32_t i=0;i<nbpolys;i++)
	{
		vector<HEHalfEdge*> faceEdges;

		HEFace* hef = new HEFace(i);
		m_faces.push_back(hef);

		nbvertices = polyCount[i];

		for (uint32_t j = 0; j<nbvertices; j++)
		{
			HEHalfEdge* he = new HEHalfEdge(edgeid);
			edgeid++;
			he->m_face = hef;
			if (hef->m_halfEdge == NULL) hef->m_halfEdge = he;
			he->m_vert = m_vertices[polyIndices[j+base]];
			if (he->m_vert->m_halfEdge == NULL) he->m_vert->m_halfEdge = he;
			faceEdges.push_back(he);
			m_halfEdges.push_back(he);
		}
		base += nbvertices;

		uint32_t n = faceEdges.size();
		for (uint32_t j = 0; j<n - 1; j++)
		{
			HEHalfEdge* he = faceEdges[j];
			he->m_next = faceEdges[j + 1];
		}

		HEHalfEdge* he = faceEdges[n - 1];
		he->m_next = faceEdges[0];
	}

	// associate each halfedge with its pair (belonging to adjacent face)
	PairHalfEdges(m_halfEdges);

}



//go through all the halfedges and find matching pairs
void HEMesh::PairHalfEdges(std::vector<HEHalfEdge*>& edges)
{
	HEUniqueEdgeMap openEdges;
	HEUniqueEdgeMapIt it;

	size_t n = edges.size();

	for (size_t i = 0; i<n; i++)
	{
		HEHalfEdge* he = edges[i];
		if (he->m_pair == NULL)
		{
			// get key
			HEUniqueEdgeKey_t key = GetEdgeKey(he);

			// look for it in map
			it = openEdges.find(key);

			// found open edge, attach them and remove from list
			if (it != openEdges.end())
			{
				it->second->m_pair = he;
				he->m_pair = it->second;
				openEdges.erase(it);
			}
			// add edge to open list
			else
			{
				openEdges.insert(std::pair<HEUniqueEdgeKey_t, HEHalfEdge*>(key, he));
			}
		}
	}
}

void HEMesh::GetUniqueEdges()
{
	m_uniqueEdges.clear();
	HEUniqueEdgeMap uniqueEdges;
	HEUniqueEdgeMapIt it;
	for (size_t i = 0; i < m_halfEdges.size(); i++)
	{
		HEUniqueEdgeKey_t key = GetEdgeKey(m_halfEdges[i]);
		it = uniqueEdges.find(key);
		if (it == uniqueEdges.end())
		{
			m_uniqueEdges.push_back(i);
			uniqueEdges[key] = m_halfEdges[i];
		}
	}
}

// check all edges for intersection with a plane
void HEMesh::RetrieveSplitEdges(HEPlane& P, vector<HESplitEdgeInfo_t>& splitEdges)
{
	size_t index = m_vertices.size();

	for (size_t k = 0; k<m_uniqueEdges.size(); k++)
	{
		HEHalfEdge* edge = getUniqueEdge(k);
		HEPoint intersection;

		if (PlaneEdgeIntersection(edge, &P, intersection) == true)
		{
			HEVertex* nv = new HEVertex(intersection, index);
			nv->m_d = false;
			splitEdges.push_back(std::make_pair(k, nv));
			index++;
		}
	}
}

// reset vertex is inserted flag
void HEMesh::ResetInsertedFlag()
{
	for (size_t k = 0; k < m_vertices.size(); k++)
		m_vertices[k]->m_d = false;
}

// does this half edge match with a unique splitted edge
bool HEMesh::EdgeMatch(HEHalfEdge* e, HESplitEdgeInfo_t& infos)
{
	HEHalfEdge* he = getUniqueEdge(infos.first);
	HEUniqueEdgeKey_t key = GetEdgeKey(he);
	return key == GetEdgeKey(e);
}

// split mesh
void HEMesh::SplitMesh( HEPlane& P, HEPoint& center)
{
	float centerside = P.Side(center);
	if (centerside != 0)
	{
		// get all split edges
		vector<HESplitEdgeInfo_t> splitEdgeInfos;
		
		// get array of unique physical edge
		GetUniqueEdges();

		RetrieveSplitEdges(P, splitEdgeInfos);
		ResetInsertedFlag();

		std::vector<ULONG> vertexLookUp;
		vertexLookUp.resize(m_vertices.size()+splitEdgeInfos.size());

		// check if the plane cuts the mesh at all, at least one point should be on the other side of the plane.
		// compared to the first point
		bool cut = false;
		HEVertex* first = getVertex(0);
		for (size_t i = 0; i<m_vertices.size(); i++)
		{
			HEVertex* v = getVertex(i);
			v->m_side = P.Side(*v);
			if (i>0 && first->m_side * v->m_side <= 0) cut = true;
		}

		size_t vertexid = 0;
		//plane cuts the mesh
		if (cut)
		{
		
			vector<HEVertex*> newVertices;
			vector<HEHalfEdge*> newHalfEdges;
			vector<HEFace*> newFaces;
			//loop through all faces.
			for (size_t i = 0; i<m_faces.size(); i++)
			{
				HEFace* face = getFace(i);
				HEHalfEdge* halfEdge = face->m_halfEdge;
				vector<HEVertex*> newFaceVertices;// vertices on the correct side.
				do
				{
					if (halfEdge->m_vert->m_side * centerside >= 0)
					{
						newFaceVertices.push_back(halfEdge->m_vert);
					}

					for (size_t j = 0; j<splitEdgeInfos.size(); j++)
					{
						if(EdgeMatch(halfEdge, splitEdgeInfos[j]))
						{
							newFaceVertices.push_back(splitEdgeInfos[j].second);
						}
					}
					halfEdge = halfEdge->m_next;
				} while (halfEdge != face->m_halfEdge);

				
				// Create a new face form the vertices we retained,ignore degenerate faces with less than 3 vertices.
				// Add all face-related information to the data-structure.
				if (newFaceVertices.size()>2)
				{
					HEFace* newFace = new HEFace(newFaces.size());
					newFaces.push_back(newFace);
					vector<HEHalfEdge*> faceEdges;
					for (size_t j = 0; j<newFaceVertices.size(); j++)
					{
						HEVertex* v = newFaceVertices[j];
						HEVertex* nv;
						if(!v->m_d)
						{
							nv = new HEVertex(v->m_x, v->m_y, v->m_z, vertexid);
							newVertices.push_back(nv);
							v->m_d = true;
							vertexLookUp[v->m_id] = newVertices.size() - 1;
							vertexid++;
						}
						else nv = newVertices[vertexLookUp[v->m_id]];
							
						HEHalfEdge* newHalfEdge = new HEHalfEdge(newHalfEdges.size());
						faceEdges.push_back(newHalfEdge);
						newHalfEdge->m_vert = nv;
						if (nv->m_halfEdge == NULL) nv->m_halfEdge = newHalfEdge;
						newHalfEdge->m_face = newFace;
						if (newFace->m_halfEdge == NULL) newFace->m_halfEdge = newHalfEdge;
						
					}
					
					
					size_t n = faceEdges.size();
					HEHalfEdge* he;
					for (size_t j = 0; j<n - 1; j++)
					{
						he = faceEdges[j];
						he->m_next = faceEdges[j + 1];
						newHalfEdges.push_back(he);
					}
					he = faceEdges[n - 1];
					he->m_next = faceEdges[0];
					newHalfEdges.push_back(he);
					
				}
			}
			
			//Add missing information to the datastructure
			size_t n = newHalfEdges.size();
			PairHalfEdges(newHalfEdges);

			// Cutting the mesh not only cuts the faces, it also creates one new planar face looping through all new cutpoints(in a convex mesh).
			// This hole in the mesh is identified by unpaired halfedges remaining after the pairing operation.
			// This part needs to rethought to extend to concave meshes!!!
			vector<HEHalfEdge*> unpairedEdges;
			for (size_t i = 0; i<n; i++)
			{
				HEHalfEdge* he = newHalfEdges[i];
				if (he->m_pair == NULL) unpairedEdges.push_back(he);
			}

			if (unpairedEdges.size() > 0)
			{
				//Create a closed loop out of the collection of unpaired halfedges and associate a new face with this.
				//Easy to explain with a drawing, beyond my skill with words.
				HEFace* cutFace = new HEFace(newFaces.size());
				vector<HEHalfEdge*> faceEdges;
				HEHalfEdge* he = unpairedEdges[0];
				HEHalfEdge* hen = he;
				bool valid = true;
				do
				{
					if (!he->m_next->m_pair) {
						valid = false;
					}
					else
					{
						hen = he->m_next->m_pair->m_next;
						while (std::find(unpairedEdges.begin(), unpairedEdges.end(), hen) == unpairedEdges.end())hen = hen->m_pair->m_next;
						HEHalfEdge* newhe = new HEHalfEdge(newHalfEdges.size());
						faceEdges.push_back(newhe);
						if (cutFace->m_halfEdge == NULL) cutFace->m_halfEdge = newhe;
						newhe->m_vert = hen->m_vert;
						newhe->m_pair = he;
						he->m_pair = newhe;
						newhe->m_face = cutFace;
						he = hen;
					}
				} while (valid && hen != unpairedEdges[0]);

				if (valid)
				{
					size_t m = faceEdges.size();
					for (size_t j = 1; j < m; j++)
					{
						he = faceEdges[j];
						he->m_next = faceEdges[j - 1];
						newHalfEdges.push_back(he);
					}
					he = faceEdges[0];
					he->m_next = faceEdges[m - 1];
					newHalfEdges.push_back(he);
					newFaces.push_back(cutFace);
				}
				else delete cutFace;

			}
			
			Clean();
			
			m_vertices.resize(newVertices.size());
			for (size_t i = 0; i < newVertices.size(); i++)m_vertices[i] = newVertices[i];
			
			m_halfEdges.resize(newHalfEdges.size());
			for (size_t i = 0; i < newHalfEdges.size(); i++)m_halfEdges[i] = newHalfEdges[i];
			
			m_faces.resize(newFaces.size());
			for (size_t i = 0; i < newFaces.size(); i++)m_faces[i] = newFaces[i];
			
		}
		else
		{
			
			// if the plane doesn't cut the mesh, keep the mesh if on the same side as "center", otherwise discard everything.
			if (getVertex(0)->m_side * centerside <= 0)
			{
				Clean();
			}
		}
	}

	/*
	uint32_t numOpenEdges = 0;
	for (std::vector<HEHalfEdge*>::iterator it = m_halfEdges.begin(); it < m_halfEdges.end(); it++)
	{
		if (!(*it)->m_pair)numOpenEdges++;
	}
	LOG("BUILD MESH LEAVES " + CString((LONG)numOpenEdges) + " Open Edges");
	*/
}


// add fragment
void HEMesh::AddFragments()
{
	size_t basepolygon = 0;
	m_polygondata.clear();
	m_verticedata.clear();

	for (size_t a = 0; a<m_vertices.size(); a++)m_vertices[a]->m_d = false;
	HEPoint pos;
	size_t newid = 0;

	for (size_t a = 0; a<m_faces.size(); a++)
	{
		HEFace* face = m_faces[a];
		HEHalfEdge* halfEdge = face->m_halfEdge;

		size_t c = 0;
		m_polygondata.push_back(c);
		do {
			halfEdge = halfEdge->m_next;
			HEVertex* v = halfEdge->m_vert;
			if (v->m_d == false)
			{
				m_verticedata.push_back(*v);
				v->m_d = true;
				v->m_id = newid;
				newid++;
			}
			m_polygondata.push_back(v->m_id);
			c++;
		} while (halfEdge != face->m_halfEdge);

		m_polygondata[basepolygon] = c;
		basepolygon += (c + 1);
	}
}

HEMesh::~HEMesh()
{
	Clean();
}

void HEMesh::Clean()
{
	for (std::vector<HEVertex*>::iterator it = m_vertices.begin(); it < m_vertices.end(); it++)
		if(*it)delete (*it);
	m_vertices.clear();

	for (std::vector<HEHalfEdge*>::iterator it = m_halfEdges.begin(); it < m_halfEdges.end(); it++)
		if (*it)delete (*it);
	m_halfEdges.clear();

	for (std::vector<HEFace*>::iterator it = m_faces.begin(); it < m_faces.end(); it++)
		if (*it)delete (*it);
	m_faces.clear();
}

HEMesh::HEMesh(HEMesh* fullMesh)
{
	m_vertices.resize(fullMesh->m_vertices.size());
	for (size_t j = 0; j < fullMesh->m_vertices.size(); j++)
		m_vertices[j] = new HEVertex(*fullMesh->m_vertices[j], j);

	m_faces.resize(fullMesh->m_faces.size());
	for (size_t j = 0; j < fullMesh->m_faces.size(); j++)
		m_faces[j] = new HEFace(j);

	m_halfEdges.resize(fullMesh->m_halfEdges.size());
	for (size_t j = 0; j < fullMesh->m_halfEdges.size(); j++)
		m_halfEdges[j] = new HEHalfEdge(j);

	for (size_t j = 0; j < m_halfEdges.size(); j++)
	{
		m_halfEdges[j]->m_face = m_faces[fullMesh->m_halfEdges[j]->m_face->m_id];
		m_halfEdges[j]->m_next = m_halfEdges[fullMesh->m_halfEdges[j]->m_next->m_id];
		m_halfEdges[j]->m_pair = m_halfEdges[fullMesh->m_halfEdges[j]->m_pair->m_id];
		m_halfEdges[j]->m_vert = m_vertices[fullMesh->m_halfEdges[j]->m_vert->m_id];
	}

	for (size_t j = 0; j < fullMesh->m_faces.size(); j++)
	{
		m_faces[j]->m_halfEdge = m_halfEdges[fullMesh->m_faces[j]->m_halfEdge->m_id];
	}

	for (size_t j = 0; j < fullMesh->m_vertices.size(); j++)
	{
		m_vertices[j]->m_halfEdge = m_halfEdges[fullMesh->m_vertices[j]->m_halfEdge->m_id];
	}
}



