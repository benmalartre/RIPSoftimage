// HEMESH
#ifndef HEMESH_H
#define HEMESH_H

#include <vector>
#include <algorithm>
#include <cassert>
#include <vector>
#include <map>

using namespace std;

struct HEPoint {
	float m_x;
	float m_y;
	float m_z;

	HEPoint() {};
	HEPoint(float x, float y, float z) {
		m_x = x;
		m_y = y;
		m_z = z;
	};
	HEPoint(double x, double y, double z) {
		m_x = (float)x;
		m_y = (float)y;
		m_z = (float)z;
	};

	HEPoint(const HEPoint& other) {
		m_x = other.m_x;
		m_y = other.m_y;
		m_z = other.m_z;
	};
};

static void HEPointNormalize(HEPoint& pnt) {
	float len = sqrtf(pnt.m_x * pnt.m_x + pnt.m_y * pnt.m_y + pnt.m_z * pnt.m_z);
	if (fabsf(len) > 0.0000001f)
	{
		float invLen = 1.0f / len;
		pnt.m_x *= invLen;
		pnt.m_y *= invLen;
		pnt.m_z *= invLen;
	}
};

class HEPlane
{
public:
	float m_A,m_B,m_C,m_D;

	void Set(const HEPoint& p0, const HEPoint& n);

	//returns 1 if p is on same side as normal, -1 if on opposite side, 0 if on the plane
	float Side(const HEPoint& p);
};

class HEVertex;
struct HEFace;
struct HEHalfEdge;

struct HEHalfEdge
{
public:
	uint32_t m_id;
	HEVertex* m_vert;		// vertex at the end of the half-edge
    HEHalfEdge* m_pair;		// oppositely oriented adjacent half-edge 
    HEFace* m_face;			// face the half-edge borders
    HEHalfEdge* m_next;		// next half-edge around the face

	HEHalfEdge(uint32_t id) { m_id = id; m_vert = NULL; m_pair = NULL; m_face = NULL, m_next = NULL; };
};


struct HEFace
{
public:
	uint32_t m_id;
	HEHalfEdge* m_halfEdge; 

	HEFace(uint32_t id) { m_id = id; m_halfEdge = NULL; };
};

class HEVertex: public HEPoint
{
public:
	uint32_t m_id;
	bool m_d;
	float m_side;
	HEHalfEdge* m_halfEdge;

	HEVertex(float x, float y, float z, uint32_t id);
	HEVertex(double x, double y, double z, uint32_t id);
	HEVertex(const HEPoint& p, uint32_t id);
	~HEVertex(){};
};

HEVertex* SplitEdge(HEHalfEdge* e, HEPoint p, uint32_t index);

typedef std::pair<uint32_t, HEVertex*> HESplitEdgeInfo_t;
typedef std::pair<uint32_t, uint32_t> HEUniqueEdgeKey_t;
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
	vector<uint32_t> m_uniqueEdges;

	std::vector<HEPoint> m_verticedata;
	std::vector<uint32_t> m_polygondata;

	HEFace* getFace(uint32_t i){return m_faces[i];}
	HEVertex* getVertex(uint32_t i){return m_vertices[i];}
	HEHalfEdge* getHalfEdge(uint32_t i){return m_halfEdges[i];}
	HEHalfEdge* getUniqueEdge(uint32_t i) { return m_halfEdges[m_uniqueEdges[i]]; }

	HEMesh(){};
	HEMesh(HEMesh* fullMesh);
	~HEMesh();

	void Clean();
	void SetHEData(double* vertices, uint32_t numVertices, uint32_t* polys, uint32_t numPolys);
	void SetHEData(double* vertices, uint32_t numVertices, uint32_t* polyCount, uint32_t numPolyCount, uint32_t* polyIndices);
	void PairHalfEdges(std::vector<HEHalfEdge*>& edges);
	void ResetInsertedFlag();
	void RetrieveSplitEdges(HEPlane& P, vector<HESplitEdgeInfo_t>& splitEdges);
	void SplitMesh(HEPlane& P,HEPoint& center);
	void AddFragments();

	HEUniqueEdgeKey_t GetEdgeKey(HEHalfEdge* e);
	void GetUniqueEdges();
	bool EdgeMatch(HEHalfEdge* e, HESplitEdgeInfo_t& infos);
};

#endif /* HEMESH_H*/
