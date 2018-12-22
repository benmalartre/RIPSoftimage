/*-------------------------------------------------
// HairData.h
//
  $$$$$$$$$$$
        $$$$$$$$$$$$$$$                           $$
      $$$$$$$$$$$$$$$$$$    @@@`````````",       $$$$
      $$$$$$$$$$$$$$$$$$ @@@@ @           `,     $ $$
       $$$$$$$$$$$$$$$$@@@@            \~~\ ",   ,$$",
        $$$$$$$$$$$$$$$@@       /~~\    \@@ \ ","     :
          $$$$$$$$$$$@@@@       \    \   \@@","       :
                    @@@@@@@       \@@@ \   ,"         :
                    @@@@@@@@@@@@@  \@@@//  "         :
                     @@@@@@@@                       :
                      @@@@@@   __                  :
                       @@@@   /\                  /
            $$$$$$$$$$$$ @@@    \\______________/|
          $$$$$$$$$$$$$$$$@@     \ __         / /
        $$$$$$$$$$$$$$$$$$$@\,     \  \---\,/ /
        $$$$$$$$$$$$$$$$$$$   ~",    \_____/ /  Glo Pearl
         $$$$$$$$$$$$$$$$$       ",,,,,,,,,/
          $$$$$$$$$$$$$$$
            $$$$$$$$$$$
 */
//-------------------------------------------------
#ifndef __HAIRDATA_H_
#define __HAIRDATA_H_

#include "HairRegister.h"
#include "HairGeometry.h"
#include "HairBranch.h"
#include "HairGuide.h"

namespace ICEHAIR {
	class IHVertex;
	class IHBranch;
	class IHTip;

	//-------------------------------------------------------------
	//
	//	Hair Mesh Data Object
	//
	//	custom data for passing through the ICE tree
	//	this is where all datas are computed
	//
	//	contains methods for getting data from guide geometries
	//-------------------------------------------------------------
	class HairMeshData
	{
	public:
		HairMeshData();
		~HairMeshData();
		IHGeometry* _geo;

		bool _init;
		bool _valid;
		ULONG _nbtips;
		ULONG _security;

		std::vector<IHPolygon*> _tippolygons;
		std::vector<IHTip*> _tips;
		std::vector<IHTip*> _sortedtips;
		std::vector<IHTip*> _islands;
		std::vector<IHTip*> _levels;
		std::vector<IHBranch*> _branches;
		std::vector<IHBranch*> _waitingbranches;
		std::vector<std::vector<IHBranch*>> _connectbranches;
		std::vector<IHVertex*> _tipvertices;
		std::vector<ULONG> _guidetriangles;
		std::vector<float> _trianglesarea;

		void GetTipDatas();
		void GetTipIslands();
		void GetTipIsland(IHPolygon* start, ULONG islandid);
		void GetTipBranches();

		void GetPointPosition(CICEGeometry& geom);
		void GetPointPosition(CICEGeometry& geom, bool update);
		bool CheckGeometry();
		void GetBranches(std::vector<IHTip*>& islands);
		void GetConnectedBranches();
		void ConnectBranches();
		void PushBranchingEdges(IHBranch* branch);
		void ClearConnectedBranches();
		void PushConnectedBranches(IHBranch*);
		void PushWaitingBranches(IHBranch*);
		void GetGuideTriangles();
		void GetWalkingGuides();

		void ClearEmitMeshData();					// clear emit mesh data
		void ClearBranchesData();					// clear branches data
		void ClearLevelData();						// clear level data(tips + islands data)
		void ClearTipData();						// clear tip data
		void ClearIslandData();						// clear islands data
		void ClearGuideData();						// clear hair guides data
		void ClearDatas();							// clear all datas

		std::vector<IHGuide*> _guides;

		void GetVirtualVerticesPosition();

		void InitGuides();
		void TermGuides();

		float GetTriangleArea(std::vector<IHGuide*>& guides);

	};
}
#endif