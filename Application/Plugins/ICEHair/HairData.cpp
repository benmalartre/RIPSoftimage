//-------------------------------------------------
//
// HairData.cpp
//
//-------------------------------------------------
/*
           ___
       .-'`   `'-.
   _,.'.===   ===.'.,_
  / /  .___. .___.  \ \
 / /   ( o ) ( o )   \ \                                            _
: /|    '-'___'-'    |\ ;                                          (_)
| |`\_,.-'`   `"-.,_/'| |                                          /|
| |  \             /  | |                                         /\;
| |   \           /   | | _                              ___     /\/
| |    \   __    /\   | |' `\-.-.-.-.-.-.-.-.-.-.-.-.-./`   `"-,/\/ 
| |     \ (__)  /\ `-'| |    `\ \ \ \ \ \ \ \ \ \ \ \ \`\       \/
| |      \-...-/  `-,_| |      \`\ \ \ \ \ \ \ \ \ \ \ \ \       \
| |       '---'    /  | |       | | | | | | | | | | | | | |       |
| |               |   | |       | | | | | | | | | | | | | |       |
\_/               |   \_/       | | | | | | | | | | | | | | .--.  ;Dog House
                  |       .--.  | | | | | | | | | | | | | | |  | /
                   \      |  | / / / / / / / / / / / / / /  |  |/
               jgs |`-.___|  |/-'-'-'-'-'-'-'-'-'-'-'-'-'`--|  |
            ,.-----'~~;   |  |                  (_(_(______)|  |
           (_(_(_______)  |  |                        ,-----`~~~\
                    ,-----`~~~\                      (_(_(_______)
                   (_(_(_______)

*/
//-------------------------------------------------
#include "HairData.h"

namespace ICEHAIR {
	//------------------------------------------
	//
	// HairMeshData object
	//
	//------------------------------------------
	HairMeshData::HairMeshData()
	{
		_geo = new IHGeometry();

	};

	HairMeshData::~HairMeshData()
	{
		ClearDatas();
	}

	void HairMeshData::ClearDatas()
	{
		ClearGuideData();
		ClearConnectedBranches();

		_tips.clear();
		_islands.clear();

		ClearLevelData();
		ClearBranchesData();
	}

	void HairMeshData::ClearLevelData()
	{
		for (std::vector<IHTip*>::iterator it = _levels.begin(); it < _levels.end(); it++)
		{
			delete (*it);
		}
		_levels.clear();
	}


	void HairMeshData::ClearTipData()
	{
		for (std::vector<IHTip*>::iterator it = _tips.begin(); it < _tips.end(); it++)
		{
			delete (*it);
		}
		_tips.clear();
	}

	void HairMeshData::ClearIslandData()
	{
		for (std::vector<IHTip*>::iterator it = _islands.begin(); it < _islands.end(); it++)
		{
			delete (*it);
		}
		_islands.clear();
	}

	void HairMeshData::ClearBranchesData()
	{
		_waitingbranches.clear();
		std::vector<IHBranch*>::iterator bi;
		for (bi = _branches.begin(); bi < _branches.end(); bi++)
		{
			delete (*bi);
		}
		_branches.clear();
	}

	void HairMeshData::ClearGuideData()
	{
		std::vector<IHGuide*>::iterator gi;
		for (gi = _guides.begin(); gi < _guides.end(); gi++)
		{
			delete (*gi);
		}
		_guides.clear();
	}

	bool HairMeshData::CheckGeometry()
	{
		//if(!_geo->_geocheck)return true;

		// this geo contains triangles or ngons
		// it's invalid if those polygons do not belongs to  tip islands
		bool check = true;
		std::vector<IHPolygon*>::iterator pi;
		for (pi = _geo->_polygons.begin(); pi < _geo->_polygons.end(); pi++)
		{
			if (!(*pi)->_tip)
			{
				if ((*pi)->_nbv != 4)
				{
					check = false;
					break;
				}
			}
		}
		return check;
	}

	void HairMeshData::GetPointPosition(CICEGeometry& geom)
	{
		_geo->GetPointPosition(geom);
	}

	void HairMeshData::GetTipDatas()
	{
		std::vector<IHEdge*>::iterator ei;
		std::vector<IHPolygon*>::iterator pi;
		for (ei = _geo->_edges.begin(); ei < _geo->_edges.end(); ei++)
		{
			bool check = false;
			for (pi = (*ei)->_polygons.begin(); pi < (*ei)->_polygons.end(); pi++)
			{
				if ((*pi)->_tip)check = true;
			}
			(*ei)->_tip = check;
			if ((*ei)->_tip)
			{
				(*ei)->_vertices[0]->_tip = true;
				(*ei)->_vertices[1]->_tip = true;
			}
		}
	}

	void HairMeshData::GetTipIslands()
	{
		_geo->UncheckPolygons();

		std::vector<IHPolygon*>::iterator pi;
		ULONG islandid = 0;
		for (pi = _tippolygons.begin(); pi < _tippolygons.end(); pi++)
		{
			if (!(*pi)->_checked)
			{
				GetTipIsland((*pi), islandid);
				islandid++;
			}
		}
		_nbtips = (ULONG)_tips.size();
	}

	void HairMeshData::GetTipIsland(IHPolygon* p, ULONG islandid)
	{
		IHTip* island = new IHTip();
		island->_id = islandid;
		island->_tip = true;
		std::vector<IHPolygon*> pv;
		std::vector<IHPolygon*> tpv;
		std::vector<IHPolygon*> npv;
		std::vector<IHPolygon*>::iterator pi;
		std::vector<IHPolygon*>::iterator pi2;
		std::vector<IHTriangle*>::iterator ti;

		p->_checked = true;
		island->_polygons.push_back(p);
		pv = p->GetTipNeighbors();

		// grow tip island
		while (pv.size() > 0)
		{
			tpv.clear();
			for (pi = pv.begin(); pi < pv.end(); pi++)
			{
				island->_polygons.push_back((*pi));
				npv = (*pi)->GetTipNeighbors();
				if (npv.size() > 0)
				{
					for (pi2 = npv.begin(); pi2 < npv.end(); pi2++)
					{
						tpv.push_back(*pi2);
					}
				}
			}
			pv = tpv;
		}

		// Get Border Edges and Triangles
		island->_borders.clear();
		IHVertex* v;

		std::vector<IHEdge*>::iterator ei;
		for (pi = island->_polygons.begin(); pi < island->_polygons.end(); pi++)
		{
			for (ei = (*pi)->_edges.begin(); ei < (*pi)->_edges.end(); ei++)
			{
				island->PushVertices((*ei)->_vertices[0]);
				island->PushVertices((*ei)->_vertices[1]);

				if ((*ei)->IsTipBorder())
				{
					island->_borders.push_back(*ei);
				}

				else
				{
					v = (*ei)->_vertices[0];
					v->IsSpecial();
					if (v->_special)
						island->PushSpecials(v);

					v = (*ei)->_vertices[1];
					v->IsSpecial();
					if (v->_special)
						island->PushSpecials(v);
				}
			}
		}

		_tips.push_back(island);
		_levels.push_back(island);
	}

	void HairMeshData::GetTipBranches()
	{
		_geo->UncheckEdges();
		_geo->UnbranchEdges();
		_geo->UncheckPolygons();

		GetBranches(_tips);
		_security = 0;
	}

	void HairMeshData::GetConnectedBranches()
	{
		std::vector<IHEdge*>::iterator ei;
		std::vector<IHBranch*>::iterator wi;
		//Get Connection infos
		for (wi = _waitingbranches.begin(); wi < _waitingbranches.end(); wi++)
		{
			(*wi)->GetConnections();
		}

		ClearConnectedBranches();
		std::vector<IHBranch*> waiting;

		for (wi = _waitingbranches.begin(); wi < _waitingbranches.end(); wi++)
		{
			if (!(*wi)->IsWaiting())
				PushConnectedBranches(*wi);
			else
				waiting.push_back(*wi);
		}
		_waitingbranches = waiting;
	}

	void HairMeshData::PushConnectedBranches(IHBranch* branch)
	{
		ULONG id = -1;
		// find if this branch is connected to an other already connected branch
		for (ULONG i = 0; i < _connectbranches.size(); i++)
		{
			for (ULONG j = 0; j < _connectbranches[i].size(); j++)
			{
				if (branch->Connected(_connectbranches[i][j])) { id = i; break; }
			}
		}
		//if not there we create a new container
		if (id == -1)
		{
			std::vector<IHBranch*> n;
			n.push_back(branch);
			_connectbranches.push_back(n);
		}

		else
		{
			_connectbranches[id].push_back(branch);
		}
	}

	void HairMeshData::ClearConnectedBranches()
	{
		std::vector<std::vector<IHBranch*>>::iterator bi;

		// clear old data
		for (bi = _connectbranches.begin(); bi < _connectbranches.end(); bi++)
		{
			(*bi).clear();
		}
		_connectbranches.clear();
	}

	void HairMeshData::ConnectBranches()
	{
		_islands.clear();

		std::vector<std::vector<IHBranch*>>::iterator it;
		std::vector<IHGuide*>::iterator g;
		for (it = _connectbranches.begin(); it < _connectbranches.end(); it++)
		{
			IHTip* out = new IHTip();
			out->_id = _nbtips;
			_nbtips++;

			std::vector<IHVertex*>::iterator vi;
			std::vector<IHEdge*>::iterator ei;
			std::vector<IHGuide*>::iterator gi;

			for (std::vector<IHBranch*>::iterator bi = (*it).begin(); bi < (*it).end(); bi++)
			{
				if ((*bi)->_island->_specials.size() > 0)
				{
					for (vi = (*bi)->_island->_specials.begin(); vi < (*bi)->_island->_specials.end(); vi++)
					{
						out->PushSpecials(*vi);
					}
				}

				for (gi = (*bi)->_guides.begin(); gi < (*bi)->_guides.end(); gi++)
				{
					out->_guides.push_back(*gi);
				}

				for (ei = (*bi)->_end.begin(); ei < (*bi)->_end.end(); ei++)
				{
					out->_borders.push_back(*ei);
					out->PushVertices((*ei)->_vertices[0]);
					out->PushVertices((*ei)->_vertices[1]);
				}
			}

			out->GetSpecials();
			_islands.push_back(out);
			_levels.push_back(out);
		}
	}

	void HairMeshData::GetWalkingGuides()
	{
		std::vector<IHGuide*>::iterator gi;
		for (gi = _guides.begin(); gi < _guides.end(); gi++)
		{
			if ((*gi)->_walking)
				(*gi)->_walking = !(*gi)->_vertices.back()->_special;
		}
	}

	void HairMeshData::GetBranches(std::vector<IHTip*>& islands)
	{
		/*
		if(_security>100)
		{
			UIToolkit toolkit = Application().GetUIToolkit();
			LONG out;
			toolkit.MsgBox(L"There is a problem with your mesh, walking aborted because of an infinite loop",siMsgExclamation ,"HairCore",out);
			_valid = false;
			return;
		}

		_security++;
		*/

		std::vector<IHTip*>::iterator ti;
		ULONG id = (ULONG)_branches.size();

		for (ti = islands.begin(); ti < islands.end(); ti++)
		{
			IHBranch* branch = new IHBranch(*ti);
			branch->_id = id;

			branch->WalkMesh();
			branch->GetEndEdges();

			_branches.push_back(branch);

			if (!branch->_reached || !branch->_started)
			{
				PushBranchingEdges(branch);
				PushWaitingBranches(branch);
			}

			id++;
		}

		GetConnectedBranches();
		ConnectBranches();

		// recursively call get branch
		if (_islands.size() > 0)
		{
			//Get driving guides for guide which walk INSIDE volume
			GetWalkingGuides();
			for (ti = _islands.begin(); ti < _islands.end(); ti++)
			{
				std::vector<IHGuide*>::iterator gi;
				for (gi = (*ti)->_guides.begin(); gi < (*ti)->_guides.end(); gi++)
				{
					if (!(*gi)->_walking)
					{
						(*gi)->GetDrivingGuides(*ti);
					}
				}
			}

			GetBranches(_islands);
		}
	}

	void HairMeshData::PushBranchingEdges(IHBranch* branch)
	{
		std::vector<IHEdge*>::iterator ei;
		for (ei = branch->_end.begin(); ei < branch->_end.end(); ei++)
		{
			(*ei)->PushBranches(branch->_id);
		}
	}

	void HairMeshData::PushWaitingBranches(IHBranch* branch)
	{
		std::vector<IHBranch*>::iterator bi;

		for (bi = _waitingbranches.begin(); bi < _waitingbranches.end(); bi++)
		{
			if ((*bi)->_id == branch->_id)return;
		}
		_waitingbranches.push_back(branch);
	}


	void HairMeshData::InitGuides()
	{
		std::vector<IHTip*>::iterator ti;
		std::vector<IHVertex*>::iterator vi;
		ULONG id = 0;
		for (ti = _tips.begin(); ti < _tips.end(); ti++)
		{
			for (vi = (*ti)->_vertices.begin(); vi < (*ti)->_vertices.end(); vi++)
			{
				IHGuide* guide = new IHGuide();
				guide->_id = id;
				guide->_walking = !(*vi)->_special;

				guide->_vertices.push_back(*vi);

				(*ti)->PushGuideData(guide);
				_guides.push_back(guide);
				id++;
			}

			std::vector<IHGuide*>::iterator gi;
			for (gi = (*ti)->_guides.begin(); gi < (*ti)->_guides.end(); gi++)
			{
				if (!(*gi)->_walking)
				{
					(*gi)->GetDrivingGuides(*ti);
				}
			}
		}
	}

	void HairMeshData::TermGuides()
	{
		std::vector<IHGuide*>::iterator gi;
		for (gi = _guides.begin(); gi < _guides.end(); gi++)
		{
			std::reverse((*gi)->_vertices.begin(), (*gi)->_vertices.end());
		}
	}

	void HairMeshData::GetVirtualVerticesPosition()
	{
		std::vector<IHGuide*>::iterator gi;
		std::vector<IHVirtualVertex*>::iterator vi;

		for (gi = _guides.begin(); gi < _guides.end(); gi++)
		{
			for (vi = (*gi)->_virtuals.begin(); vi < (*gi)->_virtuals.end(); vi++)
			{
				(*vi)->GetPosition();
			}
		}
	}

	void HairMeshData::GetGuideTriangles()
	{
		_guidetriangles.clear();
		_trianglesarea.clear();

		std::vector<IHTip*>::iterator ti;
		std::vector<IHVertex*>::iterator vi;
		std::vector<IHPolygon*>::iterator pi;
		std::vector<IHTriangle*>::iterator tri;

		std::map<ULONG, ULONG> vertextoguidemap;
		std::vector<IHGuide*>::iterator gi;
		for (gi = _guides.begin(); gi < _guides.end(); gi++)
		{
			vertextoguidemap[(*gi)->_vertices.front()->_id] = (*gi)->_id;
		}

		ULONG pid = 0;
		std::vector<IHGuide*> guides;
		guides.resize(3);

		for (ti = _tips.begin(); ti < _tips.end(); ti++)
		{
			for (pi = (*ti)->_polygons.begin(); pi < (*ti)->_polygons.end(); pi++)
			{
				for (tri = (*pi)->_triangles.begin(); tri < (*pi)->_triangles.end(); tri++)
				{
					for (ULONG i = 0; i < 3; i++)
					{
						pid = vertextoguidemap.find((*tri)->_vertices[i]->_id)->second;
						_guidetriangles.push_back(pid);
						guides[i] = _guides[pid];
					}
					float area = GetTriangleArea(guides);
					_trianglesarea.push_back(area);
				}
			}
		}

		guides.clear();
	}

	float HairMeshData::GetTriangleArea(std::vector<IHGuide*>& g)
	{
		CVector3f ab, ac, proj, height;
		ab.Sub(g[1]->_vertices.back()->_pos, g[0]->_vertices.back()->_pos);
		ac.Sub(g[2]->_vertices.back()->_pos, g[0]->_vertices.back()->_pos);

		//project vector to get triangle height
		float dot = ac.Dot(ab);
		proj.Scale(dot / ab.GetLengthSquared(), ab);
		height.Sub(ac, proj);

		float l1 = ab.GetLength();
		float l2 = height.GetLength();

		float area = (l1*l2) / 2;
		return area;
	}

}