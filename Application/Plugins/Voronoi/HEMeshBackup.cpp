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
HEVertex::HEVertex(double x, double y, double z, ULONG id)
{
	Set(x, y, z);
	m_id = id;
	m_d = false;
};

HEVertex::HEVertex(CVector3& v , ULONG id)
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

	//associate each halfedge with its pair (belonging to adjacent face)
	PairHalfEdges(m_halfEdges);
}

/*
//go through all the halfedges and find matching pairs
void HEMesh::PairHalfEdges(std::vector<HEHalfEdge*>& edges)
{
	LOG("PAIR HALF EDGE");
	UniqueEdgeMap openEdges;
	UniqueEdgeMapIt it;

	size_t n = edges.size();
	int a, b;
	for (size_t i = 0; i<n; i++)
	{
		HEHalfEdge* he = edges[i];
		if (he->m_pair == NULL)
		{
			// get key
			if (he->m_vert->m_id > he->m_next->m_vert->m_id)
			{
				a = he->m_next->m_vert->m_id;
				b = he->m_vert->m_id;
			}
			else
			{
				b = he->m_next->m_vert->m_id;
				a = he->m_vert->m_id;
			}
			UniqueEdgeKey key(a, b);
			LOG("KEY : " + CString(a) + "," + CString(b));

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
				openEdges.insert(std::pair<UniqueEdgeKey, HEHalfEdge*>(key, he));
			}
		}
	}

	LOG("NUM OPEN EDGES : " + CString(openEdges.size()));
}
*/

//go through all the halfedges and find matching pairs
void HEMesh::PairHalfEdges(std::vector<HEHalfEdge*>& halfEdges)
{
	int n = (int)halfEdges.size();
	for (int i = 0; i<n; i++)
	{
		HEHalfEdge* he = halfEdges[i];
		if (he->m_pair == NULL)
		{
			for (int j = 0; j<n; j++)
			{
				if (i != j)
				{
					HEHalfEdge* he2 = halfEdges[j];
					if ((he2->m_pair == NULL) && (he->m_vert == he2->m_next->m_vert) && (he2->m_vert == he->m_next->m_vert))
					{
						he->m_pair = he2;
						he2->m_pair = he;
						break;
					}
				}
			}
		}
	}

	ULONG numOpenEdges = 0;
	for (int i = 0; i < halfEdges.size(); i++) {
		if (!halfEdges[i]->m_pair)numOpenEdges++;
	}
	LOG("NUM OPEN EDGES : " + CString(numOpenEdges));
}

// check all edges for intersection with a plane
void HEMesh::RetrieveSplitEdges(HEPlane& P, vector<HESplitEdgeInfo_t>& splitEdges)
{
	size_t index = m_vertices.size();
	HEUniqueEdgeMap splittedEdges;
	for (ULONG k = 0; k<(ULONG)m_halfEdges.size(); k++)
	{
		HEHalfEdge* edge = getHalfEdge(k);
		HEUniqueEdgeKey_t key = GetEdgeKey(edge);
		if (splittedEdges.find(key) == splittedEdges.end())
		{
			CVector3 intersection;

			if (PlaneEdgeIntersection(edge, &P, intersection) == true)
			{
				HEVertex* nv = new HEVertex(intersection, index);
				splitEdges.push_back(std::make_pair(k, nv));
				index++;
				splittedEdges.insert(std::make_pair(key, edge));
			}
		}
		else
		{
			for (HEUniqueEdgeMapIt it = splittedEdges.begin(); it != splittedEdges.end(); it++)
			{

				if ((*it).second->m_vert == edge->m_next->m_vert) {
					HEVertex* nv = (*it).second->m_vert;
					splitEdges.push_back(std::make_pair(k, nv));
					splittedEdges.erase(it);
					break;
				}

			}
		}
		
	}
}

/*
SplitEdge(edge, intersection, numVertices++);
splitEdges.push_back(edge);
*/

void HEMesh::PrepareQuery()
{
	for (int iii = 0; iii < m_vertices.size(); iii++)
		m_vertices[iii]->m_d = false;
}

/*
void HEMesh::SplitMesh(HEPlane& P, CVector3& center)
{
	float centerside = P.Side(center);
	// if center is on the plane, we can't decide which part to keep, ignore.
	if (centerside != 0)
	{
		//check if the plane cuts the mesh at all, at least one point should be on the other side of the plane.
		//compared to the first point
		bool cut = false;
		HEVertex* first = getVertex(0);
		for (int i = 0; i<(int)m_vertices.size(); i++)
		{
			HEVertex* v = getVertex(i);
			v->m_side = P.Side(*v);
			if (i > 0 && first->m_side * v->m_side <= 0) {
				cut = true;
			}
		}

		//plane cuts the mesh
		if (cut)
		{
			PrepareQuery();
			vector<HEVertex*> newVertices;
			vector<HEFace*> newFaces;
			vector<HEHalfEdge*> newHalfEdges;
			vector<HEEdge*> newEdges;

			// get all split edges
			vector<SplitEdgeInfo> splitEdges;
			RetrieveSplitEdges(P, splitEdges);

			ULONG vid = m_vertices.size();

			//loop through all faces.
			for (int i = 0; i<(int)m_faces.size(); i++)
			{
				HEFace* face = getFace(i);
				HEHalfEdge* halfEdge = face->m_halfEdge;

				// three possibilities :
				// the face is cut
				// the face is over the plane, we keep it untouched
				// the face is below the plane, we discard it
				bool below = halfEdge->m_vert->m_side * centerside <= 0;
				bool intersect = false;
				halfEdge = halfEdge->m_next;
				ULONG numVertices = 0;
				//for each face, loop through all vertices and retain the vertices on the correct side. If the edge
				//is cut, insert the new point in the appropriate place.   
				do
				{
					if (below != (halfEdge->m_vert->m_side * centerside <= 0))
					{
						intersect = true;
					}
					halfEdge = halfEdge->m_next;
					numVertices++;
				} while (halfEdge != face->m_halfEdge);

				if (intersect)
				{
					do
					{
						for (std::vector<SplitEdgeInfo>::iterator it = splitEdges.begin(); it < splitEdges.end(); it++)
						{
							if (m_halfEdges[(*it).first] == halfEdge)
							{
								vid++;
								if (halfEdge->m_vert->m_side > 0)
								{
									if (!halfEdge->m_vert->m_d)
									{
										HEVertex* start = new HEVertex(*halfEdge->m_vert, vid);
										newVertices.push_back(start);
										vid++;
									}
									HEVertex* end = new HEVertex((*it).second, vid);
									newVertices.push_back(end);
									vid++;
								}
								else if(halfEdge->m_vert->m_side < 0)
								{
									if (!halfEdge->m_next->m_vert->m_d)
									{
										HEVertex* start = new HEVertex(*halfEdge->m_next->m_vert, vid);
										newVertices.push_back(start);
										vid++;
									}
									HEVertex* end = new HEVertex((*it).second, vid);
									newVertices.push_back(end);
									vid++;
								}
							}
							halfEdge = halfEdge->m_next;
						}
					} while (halfEdge != face->m_halfEdge);
				}
				else
				{
					if (below) continue;
					else
					{
						for (ULONG j = 0; j < numVertices; j++)
						{
							HEVertex* nv = new HEVertex()
							newVertices.push_back(nv);
						}
					}
				}

				// Create a new face from the vertices we retained,ignore degenerate faces with less than 3 vertices.
				// Add all face-related information to the data-structure.
				if ((int)newFaceVertices1.size()>2)
				{
					HEFace* newFace = new HEFace();
					newFaces.push_back(newFace);
					vector<HEHalfEdge*> faceEdges;
					for (int j = 0; j<(int)newFaceVertices1.size(); j++)
					{
						HEVertex* v = newFaceVertices1[j];
						if(!v->m_d)
						{
							newVertices.push_back(v);
							v->m_d = true;
						}

						HEHalfEdge* newHalfEdge = new HEHalfEdge();
						newHalfEdge->m_vert = v;
						if (v->m_halfEdge == NULL) v->m_halfEdge = newHalfEdge;
						newHalfEdge->m_face = newFace;
						if (newFace->m_halfEdge == NULL) newFace->m_halfEdge = newHalfEdge;
						faceEdges.push_back(newHalfEdge);
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
			//This hole in the mesh is identified by unpaired halfedges remaining after the pairing operation.
			//This part needs to rethought to extend to concave meshes!!!
			vector<HEHalfEdge*> unpairedEdges;
			for (int i = 0; i<n; i++)
			{
				HEHalfEdge* he = newHalfEdges[i];
				if (he->m_pair == NULL) unpairedEdges.push_back(he);
			}

			if ((int)unpairedEdges.size()>0)
			{
				LOG("NUM UNPAIRED EDGES : " + CString(unpairedEdges.size()));
				
				// Create a closed loop out of the collection of unpaired halfedges and associate a new face with this.
				// Easy to explain with a drawing, beyond my skill with words.
				HEFace* cutFace = new HEFace();
				vector<HEHalfEdge*> faceEdges;
				HEHalfEdge* he = unpairedEdges[0];
				HEHalfEdge* hen = he;

				do
				{
					hen = he->m_next->m_pair->m_next;
					while (std::find(unpairedEdges.begin(), unpairedEdges.end(), hen) == unpairedEdges.end())hen = hen->m_pair->m_next;
					HEHalfEdge* newhe = new HEHalfEdge;
					faceEdges.push_back(newhe);
					if (cutFace->m_halfEdge == NULL) cutFace->m_halfEdge = newhe;
					newhe->m_vert = hen->m_vert;
					newhe->m_pair = he;
					he->m_pair = newhe;

				} while (hen != unpairedEdges[0]);

				int m = (int)faceEdges.size();
				for (int j = 1; j<m; j++)
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

			//re-index all new datastructure
			for (int i = 0; i<(int)newVertices.size(); i++)
			{
				newVertices[i]->m_id = i;
			}
	
		}
		else
		{
			
			// if the plane doesn't cut the mesh, keep the mesh if on the same side as "center", otherwise discard everything.
			if(sides[0]*centerside>=0)
			{
			newVertices=m_vertices;
			newFaces=m_faces;
			newHalfEdges=m_halfEdges;
			}
			
		}

		
		// update the mesh
		m_vertices=newVertices;
		m_faces=newFaces;
		m_halfEdges=newHalfEdges;

		// delete split edges
		for (std::vector<HESplitEdge*>::iterator it = splitEdges.begin(); it < splitEdges.end(); it++)
		delete (*it);

		delete[]sides;
		
	}
}
*/

void HEMesh::SplitMesh(HEPlane& P, CVector3& center)
{
	float centerside = P.Side(center);
	if (centerside != 0)
	{
		// if center is on the plan, we can't decide which part to keep, ignore.
		vector<HEVertex*> newVertices;
		vector<HEFace*> newFaces;
		vector<HEHalfEdge*> newHalfEdges;

		// get all split edges
		vector<HESplitEdgeInfo_t> splitEdgeInfos;
		RetrieveSplitEdges(P, splitEdgeInfos);

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

		//plane cuts the mesh
		if (cut)
		{
			//loop through all faces.
			for (int i = 0; i<(int)m_faces.size(); i++)
			{
				HEFace* face = getFace(i);
				HEHalfEdge* halfEdge = face->m_halfEdge;
				vector<HEVertex*> newFaceVertices1;// vertices on the correct side.
				vector<HEVertex*> newFaceVertices2;// vertices on the wrong side, not used right now.
													//for each face, loop through all vertices and retain the vertices on the correct side. If the edge
													//is cut, insert the new point in the appropriate place.
				do
				{
					if (halfEdge->m_vert->m_side * centerside >= 0)
					{
						newFaceVertices1.push_back(halfEdge->m_vert);
					}
					if (halfEdge->m_vert->m_side * centerside <= 0)
					{
						newFaceVertices2.push_back(halfEdge->m_vert);
					}
					for (int j = 0; j<(int)splitEdgeInfos.size(); j++)
					{
						// loop through all split edges to check for the current edge.
						HEHalfEdge* se = m_halfEdges[splitEdgeInfos[j].first];
						if (halfEdge == se)
						{
							newFaceVertices1.push_back(splitEdgeInfos[j].second);
							newFaceVertices2.push_back(splitEdgeInfos[j].second);
							break;
						}
					}
					halfEdge = halfEdge->m_next;
				} while (halfEdge != face->m_halfEdge);
				//Create a new face form the vertices we retained,ignore degenerate faces with less than 3 vertices.
				//Add all face-related information to the data-structure.
				if ((int)newFaceVertices1.size()>2)
				{
					HEFace* newFace = new HEFace();
					newFaces.push_back(newFace);
					vector<HEHalfEdge*> faceEdges;
					for (int j = 0; j<(int)newFaceVertices1.size(); j++)
					{
						HEVertex* v = newFaceVertices1[j];
						if (std::find(newVertices.begin(), newVertices.end(), v) == newVertices.end())
						{
							newVertices.push_back(v);
						}
						HEHalfEdge* newHalfEdge = new HEHalfEdge();
						faceEdges.push_back(newHalfEdge);
						newHalfEdge->m_vert = v;
						if (v->m_halfEdge == NULL) v->m_halfEdge = newHalfEdge;
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

			/*
			//Cutting the mesh not only cuts the faces, it also creates one new planar face looping through all new cutpoints(in a convex mesh).
			//This hole in the mesh is identified by unpaired halfedges remaining after the pairibg operation.
			//This part needs to rethought to extend to concave meshes!!!
			vector<HEHalfEdge*> unpairedEdges;
			for (int i = 0; i<n; i++)
			{
				HEHalfEdge* he = newHalfEdges[i];
				if (he->m_pair == NULL) unpairedEdges.push_back(he);
			}
			if ((int)unpairedEdges.size()>0)
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
				for (int j = 1; j<m; j++)
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
			//re-index all new datastructure
			for (int i = 0; i<(int)newVertices.size(); i++)
			{
				newVertices[i]->m_id = i;
			}
			*/
		}
		else
		{// if the plane doesn't cut the mesh, keep the mesh if on the same side as "center", otherwise discard everything.
			/*if (getVertex(0)->m_side * centerside >= 0)
			{
				newVertices = m_vertices;
				newFaces = m_faces;
				newHalfEdges = m_halfEdges;
			}*/
		}
		/*
		// update the mesh
		m_vertices = newVertices;
		m_faces = newFaces;
		m_halfEdges = newHalfEdges;
		*/
	}
}

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

void HEMesh::Clean()
{
	for (std::vector<HEVertex*>::iterator it = m_vertices.begin(); it < m_vertices.end(); it++)
		delete (*it);
	m_vertices.clear();

	for (std::vector<HEHalfEdge*>::iterator it = m_halfEdges.begin(); it < m_halfEdges.end(); it++)
		delete (*it);
	m_halfEdges.clear();

	for (std::vector<HEFace*>::iterator it = m_faces.begin(); it < m_faces.end(); it++)
		delete (*it);
	m_faces.clear();
}



