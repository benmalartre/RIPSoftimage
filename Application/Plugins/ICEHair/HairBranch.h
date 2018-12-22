//------------------------------------
//
//	Branch.h
//
/*
								 |
								/_____
							 _-~~     ~~-_//
						   /~             ~\
						  |              _  |_
						 |         _--~~~ )~~ )___
						\|        /   ___   _-~   ~-_
						\          _-~   ~-_         \
						|         /         \         |
						|        |           |     (O  |
						 |      |             |        |
						 |      |   O)        |       |
						 /|      |           |       /
						 / \ _--_ \         /-_   _-~)
						   /~    \ ~-_   _-~   ~~~__/
						  |   |\  ~-_ ~~~ _-~~---~  \
						  |   | |    ~--~~  / \      ~-_
						   |   \ |                      ~-_
							\   ~-|                        ~~--__ _-~~-,
							 ~-_   |                             /     |
								~~--|                                 /
								  |  |                               /
								  |   |              _            _-~
								  |  /~~--_   __---~~          _-~
								  |  \                   __--~~
								  |  |~~--__     ___---~~
								  |  |      ~~~~~
								  |  |
*/
//------------------------------------
#ifndef __LEVEL_H_
#define __LEVEL_H_

#include <vector>
#include "HairRegister.h"
#include "HairGeometry.h"
#include "HairData.h"
#include "HairGuide.h"

namespace ICEHAIR
{
	// forward declaration
	class IHLevel;
	class IHTip;
	class IHBranch;
	class IHPolygon;
	class IHVertex;
	class IHEdge;
	class IHGuide;
	class IHVirtualVertex;

	//------------------------------------
	// Tip Class
	// desciption of the tip of the branch
	//------------------------------------
	class IHTip
	{
	public:
		IHTip();
		~IHTip();
		std::vector<IHPolygon*> _polygons;				// array of Polygon objects, could be empty if branch not starting at tip
		std::vector<IHEdge*> _borders;					// edge loop describing start of the branch
		std::vector<IHVertex*> _vertices;				// vertices of the island
		std::vector<IHVertex*> _specials;				// vertices with no descending flow connection(will walk inside volume)

		bool _tip;										// 1 : real tip , 0 : branching tip
		bool _waiting;									// does this tip waiting for connection

		ULONG _id;										// index of this tip

		ULONG _nbpossibilities;
		ULONG GetNbPossibilities();						// get number of possible starting direction for walking
		std::vector<IHGuide*> _guides;					// array of Guide objects

		void PushVertices(IHVertex* v);				// push vertices data for this island
		void PushSpecials(IHVertex* v);				// push start virtual vertices for this island

		void GetSpecials();								// check if vertices of this island are special vertices

		void PushGuideData(IHGuide* guide);
		void ClearGuideData();

		IHGuide* GetGuideByVertexID(ULONG id);
	};

	//------------------------------------
	// Level Class
	//------------------------------------
	class IHLevel
	{
	public:
		IHLevel() {};
		~IHLevel() {};

		IHLevel(std::vector<IHEdge*>& edges);			// contructor(we feed it with a loop of edge: those are tagged a start edges)
		std::vector<IHEdge*> _start;					// loop of edge at level start(tagged as segment)
		std::vector<IHEdge*> _end;						// loop of edges at level end(tagged as segment) could be empty if we reach branching edges
		std::vector<IHEdge*> _flow;					// flowing edges array(tagged as flow) those edges drive hair generation
		std::vector<IHEdge*> _segments;				// edges perpendicular to flow edges

		std::vector<IHPolygon*> _polygons;				// polygons of this level
		std::vector<IHVertex*> _vertices;				// vertices at level start (vertices belonging to tip polygons)
		std::vector<IHVirtualVertex*> _virtualvertices;// virtual vertices(that doesn't exists on input mesh) for this level

		void CheckEdges();								// check flow edges
		void GetPolygons();								//
		void GetFlow();
		bool PolygonsLooping();

		IHLevel* _next;
		IHLevel* GetNext();

		bool IsRoot();									// a level is root if one of its start edge is a boundary

		bool _tip;
		bool _reached;

		void PushVertices(IHVertex*);
		void SetVirtualVertices(std::vector<IHVertex*>& _vertices);

	};

	//------------------------------------
	// Branch Class
	//------------------------------------
	class IHBranch
	{
	public:
		IHBranch(IHTip* tip);
		~IHBranch();

		std::vector<IHGuide*> _guides;			// array of Guide passing through this branch
		ULONG _id;									// index of this branch
		std::vector<IHLevel*> _levels;			// pointer to array of Level objects
		IHTip* _island;							// Tip object
		bool _tip;									// does this branch start at tip
		bool _reached;								// does this branch reached root
		bool _waiting;								// is this branch waiting for a connection
		bool _started;								// does this branch started walking succesfully
		std::vector<LONG> _connections;				// connected branches indices

		std::vector<IHEdge*> _end;					// end edges
		void GetEndEdges();							// get end edges

		std::vector<IHEdge*> _branchingedges;		// edges where two islands connect

		void WalkMesh();							// walk mesh for this branch pushing _levels array
		bool IsWaiting();							// does this branch have found a connection
		void GetConnections();						// push connection with other branches infos
		void PushConnection(LONG id);				// push ONE connection if not exists
		bool Connected(IHBranch* other);			// is this branch connected with another

		void PushGuidesData();

	};
} // end namespace ICEHAIR
#endif