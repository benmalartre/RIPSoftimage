//------------------------------------
//
//	Geometry.h
//
//------------------------------------
/*
	Classes for storing Geometry Information
	As the ICE sdk provide only access to Point Position
	and Array indices for triangle, edges and polygons
	we have to rebuild all connections info for later analysis
	and walking over and inside the mesh
	We use these classes also for storing additional needed datas

								   _ ,___,-'",-=-. 
					   __,-- _ _,-'_)_  (""`'-._\ `. 
					_,'  __ |,' ,-' __)  ,-     /. | 
				  ,'_,--'   |     -'  _)/         `\ 
				,','      ,'       ,-'_,`           : 
				,'     ,-'       ,(,-(              : 
					 ,'       ,-' ,    _            ; 
					/        ,-._/`---'            / 
				   /        (____)(----. )       ,' 
				  /         (      `.__,     /\ /, 
				 :           ;-.___         /__\/| 
				 |         ,'      `--.      -,\ | 
				 :        /            \    .__/ 
				  \      (__            \    |_ 
				   \       ,`-, *       /   _|,\ 
					\    ,'   `-.     ,'_,-'    \ 
				   (_\,-'    ,'\")--,'-'       __\ 
					\       /  // ,'|      ,--'  `-. 
					 `-.    `-/ \'  |   _,'         `. 
						`-._ /      `--'/             \ 
				-hrr-      ,'           |              \ 
						  /             |               \ 
					   ,-'              |               / 
					  /                 |             -' 
*/
//-------------------------------------------------
#ifndef __GEOMETRY_H_
#define __GEOMETRY_H_

#include <vector>

namespace ICEHAIR
{
	// forward declaration
	class IHVertex;
	class IHVirtualVertex;
	class IHEdge;
	class IHTriangle;
	class IHPolygon;
	class IHGeometry;

	//------------------------------------
	// IHVertex Class
	//------------------------------------
	class IHVertex
	{
	public:
		IHVertex() {};
		IHVertex(LONG id, float px, float py, float pz);
		~IHVertex();

		ULONG _id;

		std::vector<IHEdge*> _edges;
		std::vector<IHEdge*> _flowedges;
		std::vector<IHEdge*> _segmentedges;
		std::vector<IHPolygon*> _polygons;
		std::vector<IHVertex*> _neighbors;
		std::vector<IHTriangle*> _triangles;

		IHVertex* _next;

		bool _boundary;												// is this vertex boundary
		bool _checked;												// has this vertex been visited
		bool _tip;													// is this vertex tip vertex(belongs to tip polygon)
		bool _root;													// is this vertex root(level = 0) 
		bool _special;												// vertex with no descending flow connection(will interpolate from other guides)
		bool _flow;

		ULONG _nbneighbors;											//	nb neigbhors vertices

		ULONG _level;												//	extrusion depth for this vertex
		ULONG _nbflowedges;											//	number of flowing edges attache to this vertex

		CVector3f _initpos;
		CVector3f _initnorm;

		CVector3f _pos;
		CVector3f _norm;

		void GetFlowEdges();
		void GetNeighbors();
		bool HasNeighbor(ULONG vertexid);							// does this vertex have an other vertex with this id
		void GetNext();
		bool HasNext();
		void IsSpecial();
		//void GetNormal();
		bool IsNeighbor(IHVertex* other);							// is this vertex connected to other vertex
		void PushPolygonData(IHPolygon* poly);						// push adjacent polygon data
	};

	class IHVirtualVertex : public IHVertex
	{
	public:
		IHVirtualVertex() {};
		~IHVirtualVertex() {};

		std::vector<IHVertex*> _vertices;
		std::vector<IHVertex*> _previous;
		IHVertex* _last;

		std::vector<float> _weights;

		void GetWeights();
		void GetPosition();
	};

	//------------------------------------
	// IHEdge Class
	//------------------------------------
	class IHEdge
	{
	public:
		IHEdge(LONG id, IHVertex* p1, IHVertex* p2);
		~IHEdge();
		ULONG _id;
		bool _checked;

		bool _flow;				// is this edge a flow edge
		bool _tip;				// is this edge a tip edge
		bool _segment;			// is this edge a segment edge
		bool _boundary;			// is this edge a boundary edge
		bool _branching;		// does this edge connect two islands

		IHEdge* _next;

		/*
			  -----
			 /|  /
			-----
			| |	||
			  -----
			|/	|/
			----- |
			|   | |
			| 	|
		*/

		std::vector<IHPolygon*> _polygons;
		std::vector<IHVertex*> _vertices;
		std::vector<LONG> _branches;

		void PushBranches(ULONG branchid);

		bool SharePolygon(IHEdge* other);
		bool ShareVertex(IHEdge* other);
		bool BelongToPolygon(IHPolygon* poly);
		bool IsTipBorder();
	};


	//------------------------------------
	// IHTriangle Class
	//------------------------------------
	class IHTriangle
	{
	public:
		IHTriangle(LONG id, IHVertex* p1, IHVertex* p2, IHVertex* p3);
		~IHTriangle() {};

		LONG _id;
		std::vector<IHVertex*> _vertices;
		CVector3f _norm;

		float _area;
		void GetArea();
	};


	//------------------------------------
	// IHPolygon Class
	//------------------------------------
	class IHPolygon
	{
	public:
		IHPolygon(LONG id, CLongArray pid);
		~IHPolygon();
		ULONG _id;					// index of this polygon
		CLongArray _pid;			// array of vertices index
		CVector3f _norm;			// normal of this polygon
		ULONG _nbv;					// actual number of vertices for this polygon

		bool _root;					// is this polygon a root polygon(pas sur que ca serve encore...)
		bool _tip;					// is this polygon a tip polygon
		bool _checked;				// does this polygon been visited by branch walking

		ULONG _level;				// depth in extrusion of this polygon

		std::vector<ULONG> _guidesid;			// array of guide id for this polygon
		void PushGuideData(ULONG guideid);		// push guide id array for this polygon
		void CleanGuideData();					// clean guide id array

		std::vector<IHVertex*> _vertices;		// vector of Vertex for this polygon
		void PushVertexData(IHVertex*& vertex);// push Vertex array for this polygon

		std::vector<IHEdge*> _edges;
		void PushEdgeData(IHEdge*& edge);

		void GetFlow(IHEdge* start);			// tag this polygon and it's edge for flow direction

		std::vector<IHTriangle*> _triangles;
		void PushTriangleData(IHTriangle*& tri);

		std::vector<IHPolygon*> GetTipNeighbors();		//grow selection until island complete
	};

	//------------------------------------
	//	IHGeometry Class
	//	this is the main class
	//	which store and manage all of the above
	//------------------------------------
	class IHGeometry
	{
	public:
		IHGeometry();
		~IHGeometry();

		std::vector<IHVertex*> _vertices;					// store vertex data
		std::vector<IHEdge*> _edges;						// store edge data
		std::vector<IHTriangle*> _triangles;				// store triangle data
		std::vector<IHPolygon*> _polygons;					// store polygon data

		//std::vector<Triangle*> _roottriangles;			// store virtuals root triangles
		CDoubleArray _points;								// point position from ICE attribute

		void GetPointPosition(CICEGeometry& geom);			// get point position from ICE sdk
		void GetPointPosition(PolygonMesh& mesh, KinematicState& kine);// get point position from regular sdk
		float GetAngleBetweenTwoEdges(ULONG id1, ULONG id2);// get angle between two edges
		void GetCachedMeshData(CICEGeometry& geom);			// get mesh description and connectivity from ICE sdk
		void GetCachedMeshData(X3DObject& obj);				// get mesh description and connectivity from regular sdk

		void ClearDatas();									// clear geometry data
		void InitGuideData();								// init geometry data

		void UncheckVertices();								// uncheck vertices
		void UncheckEdges();								// uncheck edges
		void UncheckPolygons();								// uncheck edges
		void UnbranchEdges();

		bool _init;				// is cached mesh data initialized
		ULONG _nbe;				// actual nb edges on geometry
		ULONG _nbv;				// actual nb vertices
		ULONG _nbp;				// actual nb polygons
		ULONG _nbt;				// actual nb triangles
		bool _geocheck;			// does this geo contains triangle or ngons
	};
}
#endif