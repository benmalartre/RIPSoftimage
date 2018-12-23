// HEMESH
#ifndef _HEMESH_H_
#define _HEMESH_H_

#include <vector>
#include <algorithm>
#include <cassert>
#include <vector>
#include <map>
#include <xsi_vector3.h>
#include <xsi_longarray.h>
#include <xsi_doublearray.h>

using namespace XSI;
using namespace MATH;
using namespace std;

class HEPlane
{
public:
	float m_A,m_B,m_C,m_D;

	void Set(const CVector3& p0, const CVector3& n);

	//returns 1 if p is on same side as normal, -1 if on opposite side, 0 if on the plane
	float Side(const CVector3& p);
};

class HEVertex;
struct HEFace;
struct HEHalfEdge;

struct HEHalfEdge
{
public:
	HEVertex* m_vert;		// vertex at the end of the half-edge
    HEHalfEdge* m_pair;		// oppositely oriented adjacent half-edge 
    HEFace* m_face;			// face the half-edge borders
    HEHalfEdge* m_next;		// next half-edge around the face
};


struct HEFace
{
public:
	HEHalfEdge* m_halfEdge; 
};

class HEVertex: public CVector3
{
public:
	ULONG m_id;
	bool m_d;
	float m_side;
	HEHalfEdge* m_halfEdge;

	HEVertex(float x, float y, float z, ULONG id);
	HEVertex(const CVector3& v, ULONG id);
	~HEVertex(){};
};

HEVertex* SplitEdge(HEHalfEdge* e, CVector3 p, ULONG index);

typedef std::pair<ULONG, HEVertex*> HESplitEdgeInfo_t;
typedef std::pair<ULONG, ULONG> HEUniqueEdgeKey_t;
typedef std::map<HEUniqueEdgeKey_t, HEHalfEdge*> HEUniqueEdgeMap;
typedef std::map<HEUniqueEdgeKey_t, HEHalfEdge*>::iterator HEUniqueEdgeMapIt;


// HEMesh
//-----------------------------------------
class HEMesh
{

public:
	vector<HEVertex*> m_vertices; 
	vector<HEFace*> m_faces;
	vector<HEHalfEdge*> m_halfEdges;
	vector<ULONG> m_uniqueEdges;

	CVector3Array m_verticedata;
	CLongArray m_polygondata;

	HEFace* getFace(ULONG i){return m_faces[i];}
	HEVertex* getVertex(ULONG i){return m_vertices[i];}
	HEHalfEdge* getHalfEdge(ULONG i){return m_halfEdges[i];}
	HEHalfEdge* getUniqueEdge(ULONG i) { return m_halfEdges[m_uniqueEdges[i]]; }

	HEMesh(){};
	~HEMesh();

	void Clean();
	void Set(HEMesh* fullMesh);
	void SetHEData(CVector3Array& vertices, CLongArray& polys);
	void SetHEData(CDoubleArray& vertices, CLongArray& polyCount, CLongArray& polyIndices);
	void PairHalfEdges(std::vector<HEHalfEdge*>& edges);
	void ResetInsertedFlag();
	void RetrieveSplitEdges(HEPlane& P, vector<HESplitEdgeInfo_t>& splitEdges);
	void SplitMesh(HEPlane& P,CVector3& center);
	void AddFragments();

	HEUniqueEdgeKey_t GetEdgeKey(HEHalfEdge* e);
	void GetUniqueEdges();
	bool EdgeMatch(HEHalfEdge* e, HESplitEdgeInfo_t& infos);
};

#endif /* _HEMESH_H_*/
