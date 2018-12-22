// HEMesh.cpp
//----------------------------------------------
#include "HEMesh.h"
#include "Voronoi.h"

// HEPlane Methods
//------------------------------------------------
// constructor
//------------------------------------------------
void HEPlane::Set(const CVector3& p0, const CVector3& n)
{
	CVector3 norm;
	norm.Normalize(n);
	m_A = norm.GetX();
	m_B = norm.GetY();
	m_C = norm.GetZ();
	m_D = -m_A * p0.GetX() - m_B * p0.GetY() - m_C * p0.GetZ();
};

//returns 1 if p is on same side as normal, -1 if on opposite side, 0 if on the plane
float HEPlane::Side(const CVector3& p)
{
	float tmp = m_A * p.GetX() + m_B * p.GetY() + m_C * p.GetZ() + m_D;
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
HEVertex::HEVertex(float x, float y, float z, ULONG id)
{
	Set(x, y, z);
	m_id = id;
	m_d = false;
};

HEVertex::HEVertex(const CVector3& v, ULONG id)
{
	Set(v.GetX(), v.GetY(), v.GetZ());
	m_id = id;
	m_d = false;
};


// SplitEdge constructor
//------------------------------------------------
HEVertex* SplitEdge(HEHalfEdge* e, CVector3 p, ULONG index)
{
	HEVertex* v = new HEVertex(p.GetX(), p.GetY(), p.GetZ(), index);
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
bool PlaneEdgeIntersection(HEHalfEdge* e, HEPlane* P, CVector3& ioVector)
{
	HEVertex* p0 = e->m_vert;
	HEVertex* p1 = e->m_next->m_vert;

	double denom = P->m_A*(p0->GetX() - p1->GetX()) + P->m_B*(p0->GetY() - p1->GetY()) + P->m_C*(p0->GetZ() - p1->GetZ());
	if ((denom<0.001) && (denom>-0.001)) return false;
	double u = (P->m_A*p0->GetX() + P->m_B*p0->GetY() + P->m_C*p0->GetZ() + P->m_D) / denom;
	if ((u<0.00) || (u>1.0)) return false;
	ioVector.Set(p0->GetX() + u * (p1->GetX() - p0->GetX()), p0->GetY() + u * (p1->GetY() - p0->GetY()), p0->GetZ() + u * (p1->GetZ() - p0->GetZ()));
	return true;
};

// HEMesh Get Edge Key
//-----------------------------------------
HEUniqueEdgeKey_t HEMesh::GetEdgeKey(HEHalfEdge* e)
{
	ULONG a = e->m_vert->m_id;
	ULONG b = e->m_next->m_vert->m_id;
	if (a < b) return std::make_pair(a, b);
	else return std::make_pair(b, a);
}

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


//go through all the halfedges and find matching pairs
void HEMesh::PairHalfEdges(std::vector<HEHalfEdge*>& edges)
{
	LOG("PAIR HALF EDGE");
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

	LOG("NUM OPEN EDGES : " + CString(openEdges.size()));
}

void HEMesh::GetUniqueEdges()
{
	m_uniqueEdges.clear();
	HEUniqueEdgeMap uniqueEdges;
	HEUniqueEdgeMapIt it;
	for (ULONG i = 0; i < m_halfEdges.size(); i++)
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

	for (ULONG k = 0; k<(ULONG)m_uniqueEdges.size(); k++)
	{
		HEHalfEdge* edge = getUniqueEdge(k);
		CVector3 intersection;

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
	for (int k = 0; k < m_vertices.size(); k++)
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
void HEMesh::SplitMesh( HEPlane& P, CVector3& center)
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
		for (int i = 0; i<(int)m_vertices.size(); i++)
		{
			HEVertex* v = getVertex(i);
			v->m_side = P.Side(*v);
			if (i>0 && first->m_side * v->m_side <= 0) cut = true;
		}

		ULONG vid = 0;
		//plane cuts the mesh
		if (cut)
		{
		
			vector<HEVertex*> newVertices;
			vector<HEHalfEdge*> newHalfEdges;
			vector<HEFace*> newFaces;
			//loop through all faces.
			for (int i = 0; i<(int)m_faces.size(); i++)
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

					for (int j = 0; j<(int)splitEdgeInfos.size(); j++)
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
				if ((int)newFaceVertices.size()>2)
				{
					HEFace* newFace = new HEFace();
					newFaces.push_back(newFace);
					vector<HEHalfEdge*> faceEdges;
					for (int j = 0; j<(int)newFaceVertices.size(); j++)
					{
						HEVertex* v = newFaceVertices[j];
						HEVertex* nv;
						if(!v->m_d)
						{
							nv = new HEVertex(v->GetX(), v->GetY(), v->GetZ(), vid);
							newVertices.push_back(nv);
							v->m_d = true;
							vertexLookUp[v->m_id] = newVertices.size() - 1;
							vid++;
						}
						else
						{
							if (vertexLookUp[v->m_id] >= newVertices.size())LOG("CRASH : " + CString(vertexLookUp[v->m_id]));
							nv = newVertices[vertexLookUp[v->m_id]];
						}
						
						
						HEHalfEdge* newHalfEdge = new HEHalfEdge();
						faceEdges.push_back(newHalfEdge);
						newHalfEdge->m_vert = nv;
						if (nv->m_halfEdge == NULL) nv->m_halfEdge = newHalfEdge;
						newHalfEdge->m_face = newFace;
						if (newFace->m_halfEdge == NULL) newFace->m_halfEdge = newHalfEdge;
						
					}
					
					
					int n = (int)faceEdges.size();
					HEHalfEdge* he;
					for (int j = 0; j<n - 1; j++)
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
			int n = (int)newHalfEdges.size();
			PairHalfEdges(newHalfEdges);
			
			
			//Cutting the mesh not only cuts the faces, it also creates one new planar face looping through all new cutpoints(in a convex mesh).
			//This hole in the mesh is identified by unpaired halfedges remaining after the pairibg operation.
			//This part needs to rethought to extend to concave meshes!!!
			vector<HEHalfEdge*> unpairedEdges;
			for (int i = 0; i<n; i++)
			{
				HEHalfEdge* he = newHalfEdges[i];
				if (he->m_pair == NULL) unpairedEdges.push_back(he);
			}
			
			if ((int)unpairedEdges.size() > 0)
			{
				//Create a closed loop out of the collection of unpaired halfedges and associate a new face with this.
				//Easy to explain with a drawing, beyond my skill with words.
				HEFace* cutFace = new HEFace();
				vector<HEHalfEdge*> faceEdges;
				HEHalfEdge* he = unpairedEdges[0];
				HEHalfEdge* hen = he;
				do
				{
					hen = he->m_next->m_pair->m_next;
					while (std::find(unpairedEdges.begin(), unpairedEdges.end(), hen) == unpairedEdges.end())hen = hen->m_pair->m_next;
					HEHalfEdge* newhe = new HEHalfEdge();
					faceEdges.push_back(newhe);
					if (cutFace->m_halfEdge == NULL) cutFace->m_halfEdge = newhe;
					newhe->m_vert = hen->m_vert;
					newhe->m_pair = he;
					he->m_pair = newhe;
					newhe->m_face = cutFace;
					he = hen;
				} while (hen != unpairedEdges[0]);

				int m = (int)faceEdges.size();
				for (int j = 1; j < m; j++)
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

			
			Clean();
			
			m_vertices.resize(newVertices.size());
			for (ULONG i = 0; i < newVertices.size(); i++)m_vertices[i] = newVertices[i];
			
			m_halfEdges.resize(newHalfEdges.size());
			for (ULONG i = 0; i < newHalfEdges.size(); i++)m_halfEdges[i] = newHalfEdges[i];
			
			m_faces.resize(newFaces.size());
			for (ULONG i = 0; i < newFaces.size(); i++)m_faces[i] = newFaces[i];
			
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
}

// add fragment
void HEMesh::AddFragments()
{
	int basepolygon = 0;
	m_polygondata.Clear();
	m_verticedata.Clear();

	for (int a = 0; a<(int)m_vertices.size(); a++)m_vertices[a]->m_d = false;
	CVector3 pos;
	int newid = 0;

	for (int a = 0; a<(int)m_faces.size(); a++)
	{
		HEFace* face = m_faces[a];
		HEHalfEdge* halfEdge = face->m_halfEdge;

		int c = 0;
		m_polygondata.Add(c);
		do {
			halfEdge = halfEdge->m_next;
			HEVertex* v = halfEdge->m_vert;
			if (v->m_d == false)
			{
				pos.Set(v->GetX(), v->GetY(), v->GetZ());
				m_verticedata.Add(pos);
				v->m_d = true;
				v->m_id = newid;
				newid++;
			}
			m_polygondata.Add(v->m_id);
			c++;
		} while (halfEdge != face->m_halfEdge);

		m_polygondata[basepolygon] = c;
		basepolygon += c + 1;
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

void HEMesh::Set(HEMesh* fullMesh)
{
	m_vertices.resize(fullMesh->m_vertices.size());
	std::copy(fullMesh->m_vertices.begin(), fullMesh->m_vertices.end(), m_vertices.begin());
	m_halfEdges.resize(fullMesh->m_halfEdges.size());
	std::copy(fullMesh->m_halfEdges.begin(), fullMesh->m_halfEdges.end(), m_halfEdges.begin());
	m_faces.resize(fullMesh->m_faces.size());
	std::copy(fullMesh->m_faces.begin(), fullMesh->m_faces.end(), m_faces.begin());
}



