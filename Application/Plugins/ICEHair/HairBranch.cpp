// Level.cpp
/*
	Implementation of Level class
	Implementation of Branch class
				      ,eedMMMMMMMMMNr
				  .zdMMMP"""",emeMMMMMMMMMMMMb
				.dMPP".,ddMMMMMMMMMMMMMMMMMMM"
			   dP,zmMMMMMMMMMMMMMMMMMMMMMMMMF
			  .dMMMMMMMMMMMMMMMMMMMMMMMMMMMM
			udMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
		   dMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM.                           .
		   MMMMMMMMMMMMMMMMMMMPPPP"""""""""""                ,zeeeeed$$$$
			"TMMMMMMMMMMP".,edd$$$$$$$$$$$$$$$$bbee..     ed$$$$$$$$$$$$$
			  `TMMMMP ued$$$$$$$$$$$$$$$$$$$$$$$$$$$$$P .$$$$$$P"...`"?$$
				`".u$$$$$$$$",,,.'"$$$$$$$$$$$$$$$$$$$ d$$$$P"ud$$$$$bc$$
			   .zP".,u,.$$$$$$$$$b.9$$$$$$$$$$$$$$$$$$$$$$$'z$$$$$$$$$$$"
			 .d$$d$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$"
			d$$$P".."?$$$$$P",,"?$$$$$$$$??$$$$$$$$$$$$$$$$$$$$$$$$$$F
		   d$$$$ d$$b $$$$$ $$$;?$$$$$$&zeb$$$$$$$$$$$$$$$$$$$$$$$$"
		  $$$$$$ ?F"" $$$$$ ""?'J$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$e$e.
		 d$$$$$$c_    """??     $$$$?$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$br
		 $$$$$$$" .,ebbee,.  -J$$$F d$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$.
		,cc"?$F .$$$$$$$$$$$u `$$P d$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$L
		d$$$b  x$$$$$$$$$$$$$b ?$  $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$.
		d$$$$  $  '$$$$"  9$$$  $  $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$k
		$$$$$  F  .$$$$L  d$$F  $L ?$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$F
		$$$$$. "?e$$$$$$$$$P"  '?"  `"?$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
		?$$$$$e.                      ,,. `""?$$$$$$$$$$$$$$$$$$$$$$$$$F
		`$$$$$$$>:$$r                .$$$$$$bc.`"$$$$$$$$$$$$$$$$$$$$$$
		 $$$$$$$'d$$$c               d$$$$$$$$$$b,`?$$$$$$$$$$$$$$$$$$"
		 `$$$$$'d$$$$$,             ,$$$$$$$$$$$$$b ?$$$$$$$$$$$$$$$P"
		  `$$$$$$$$$$$$.  ,%%%%%%'.d$$$$$$$$$$$$$$$b.$$$$$$$$$$$$$$" ..
		   `?$$$$$$$$$$$ '%%%%%' c$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$".d$$$$$$ec
			 `?$$$$$$$$$k `` ,zd$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$F.,d$$$$$$$$$$$b
				`?$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$P",d$$$$$$$$$$$$$$$
		.,,zd$$$bcu.`""??$$$$$$$$$$$$$$$$$$$$$$$$$$$$$P"".c$$$$$$$$$$$$$$$$PT!
	.:!!!!??$$$$$$$$$$N=  ..,,,..`""""??????????"""..,,ce$$B$$$$$$$$$$$$P?!!!!
	!!!!!!!!!??$$$ .,zed$$$$$$$$$F.d$$$beeeeeeee,4$$$$$$$$$$$$!$$$??T!!!!!!!!!
	!!!!!!!!!!!!!!W$$$$$$$$$$$$$F $$$$$$$$$$$$$$$m$$$$$$$$$$$%!!!WWd$$$W!!!!!!

*/
//--------------------------------------------
#include "HairRegister.h"
#include "HairBranch.h"

namespace ICEHAIR {
	//------------------------------------
	// Level Class
	//------------------------------------
	IHLevel::IHLevel(std::vector<IHEdge*>& edges)
	{
		_start.resize(edges.size());
		std::vector<IHEdge*>::iterator ei;
		ULONG id = 0;
		for (ei = edges.begin(); ei < edges.end(); ei++)
		{
			_start[id] = *ei;
			PushVertices((*ei)->_vertices[0]);
			PushVertices((*ei)->_vertices[1]);
			id++;
		}
	}

	void IHLevel::SetVirtualVertices(std::vector<IHVertex*>& _vertices)
	{
		_virtualvertices.clear();
		std::vector<IHVertex*>::iterator vi;
		for (vi = _vertices.begin(); vi < _vertices.end(); vi++)
		{

		}
	}

	void IHLevel::PushVertices(IHVertex* v)
	{
		std::vector<IHVertex*>::iterator vi;
		for (vi = _vertices.begin(); vi < _vertices.end(); vi++)
		{
			if ((*vi)->_id == v->_id)return;
		}
		_vertices.push_back(v);
	}

	void IHLevel::GetPolygons()
	{
		_polygons.clear();
		std::vector<IHEdge*>::iterator ei;
		std::vector<IHPolygon*>::iterator pi;
		for (ei = _start.begin(); ei < _start.end(); ei++)
		{
			(*ei)->_segment = true;
			for (pi = (*ei)->_polygons.begin(); pi < (*ei)->_polygons.end(); pi++)
			{
				if (!(*pi)->_checked && !(*pi)->_tip)
				{
					(*pi)->GetFlow(*ei);
					_polygons.push_back(*pi);
				}
			}
		}
	}

	void IHLevel::GetFlow()
	{
		_flow.clear();
		_end.clear();
		std::vector<IHPolygon*>::iterator pi;
		std::vector<IHEdge*>::iterator ei;

		for (pi = _polygons.begin(); pi < _polygons.end(); pi++)
		{
			for (ei = (*pi)->_edges.begin(); ei < (*pi)->_edges.end(); ei++)
			{
				// store unique flow edges
				if ((*ei)->_flow)
				{
					// check if edge already in _flow
					bool check = false;
					for (LONG v = 0; v < (LONG)_flow.size(); v++)
					{
						if (_flow[v]->_id == (*ei)->_id)check = true;
					}
					if (!check)
					{
						_flow.push_back(*ei);
					}
				}
				else
				{
					// check if edge already in _segments
					bool check = false;
					for (LONG v = 0; v < (LONG)_segments.size(); v++)
					{
						if (_segments[v]->_id == (*ei)->_id)check = true;
					}
					if (!check)
					{
						_segments.push_back(*ei);
					}

					bool start = false;
					for (std::vector<IHEdge*>::iterator ei2 = _start.begin(); ei2 < _start.end(); ei2++)
					{
						if ((*ei2)->_id == (*ei)->_id) { start = true; break; }
					}
					if (!start)
						_end.push_back(*ei);
				}
			}
		}
	}

	bool IHLevel::PolygonsLooping()
	{
		// we check if level polygons are forming a closed loop
		std::vector<IHEdge*>::iterator ei;
		std::vector<IHPolygon*>::iterator pi;
		std::vector<IHPolygon*>::iterator pi2;

		for (ei = _flow.begin(); ei < _flow.end(); ei++)
		{
			LONG nbp = (LONG)(*ei)->_polygons.size();
			for (pi = (*ei)->_polygons.begin(); pi < (*ei)->_polygons.end(); pi++)
			{
				//ULONG found = 0;
				bool found = false;
				for (pi2 = _polygons.begin(); pi2 < _polygons.end(); pi2++)
				{
					if ((*pi2)->_id == (*pi)->_id)found = true;
				}
				if (!found)return false;
			}
		}
		// tag those polygon as checked
		for (pi = _polygons.begin(); pi < _polygons.end(); pi++)
		{
			(*pi)->_checked = true;
		}
		return true;
	}

	IHLevel* IHLevel::GetNext()
	{
		GetPolygons();
		GetFlow();

		if (_polygons.size() == 0 || IsRoot())
		{
			_reached = true;
			return NULL;
		}

		if (!PolygonsLooping())
		{
			_reached = false;
			_next = NULL;
		}

		else
		{
			_next = new IHLevel(_end);
			_reached = false;
		}
		return _next;
	}

	bool IHLevel::IsRoot()
	{
		std::vector<IHEdge*>::iterator ei;
		for (ei = _start.begin(); ei < _start.end(); ei++)
		{
			if ((*ei)->_boundary)return true;
		}
		return false;
	}

	void IHLevel::CheckEdges()
	{
		std::vector<IHEdge*>::iterator ei;
		for (ei = _flow.begin(); ei < _flow.end(); ei++)
		{
			Application().LogMessage(L"Edge ID " + (CString)(*ei)->_id + L" is a flow edge");
			(*ei)->_checked = true;
		}
		for (ei = _segments.begin(); ei < _segments.end(); ei++)
		{
			Application().LogMessage(L"Edge ID " + (CString)(*ei)->_id + L" is a segment edge");
			(*ei)->_checked = true;
		}
	}

	//------------------------------------
	// Branch Class
	//------------------------------------
	IHBranch::IHBranch(IHTip* tip)
	{
		_island = tip;
		_guides.resize(tip->_guides.size());
		for (LONG gi = 0; gi < (LONG)tip->_guides.size(); gi++)
		{
			_guides[gi] = tip->_guides[gi];
		}
	}
	IHBranch::~IHBranch()
	{
		_guides.clear();
		_island = NULL;
		for (std::vector<IHLevel*>::iterator li = _levels.begin(); li < _levels.end(); li++)
		{
			delete(*li);
		}
		_levels.clear();
	}

	void IHBranch::WalkMesh()
	{
		IHLevel* level = new IHLevel(_island->_borders);

		_levels.push_back(level);

		level = level->GetNext();

		while (level != NULL)
		{
			_levels.push_back(level);
			PushGuidesData();
			level->CheckEdges();
			level = level->GetNext();
		}

		_started = true;

		// branch have not started succesfully  
		if (_levels.size() <= 1)_started = false;

		_reached = _levels.back()->_reached;
	}

	void IHBranch::GetEndEdges()
	{
		_end = _levels.back()->_start;
	}

	bool IHBranch::IsWaiting()
	{
		std::vector<IHEdge*>::iterator ei;
		_waiting = true;
		for (ei = _end.begin(); ei < _end.end(); ei++)
		{
			if ((*ei)->_branches.size() > 1)
				_waiting = false;
		}
		return _waiting;
	}

	void IHBranch::GetConnections()
	{
		std::vector<IHEdge*>::iterator ei;
		std::vector<LONG>::iterator li;
		_connections.clear();
		_branchingedges.clear();

		for (ei = _end.begin(); ei < _end.end(); ei++)
		{
			if ((*ei)->_branches.size() > 1)
			{
				_branchingedges.push_back(*ei);
				for (li = (*ei)->_branches.begin(); li < (*ei)->_branches.end(); li++)
				{
					PushConnection(*li);
				}
			}
		}
	}

	void IHBranch::PushConnection(LONG id)
	{
		std::vector<LONG>::iterator li;
		for (li = _connections.begin(); li < _connections.end(); li++)
		{
			if (*li == id)return;
		}
		_connections.push_back(id);
	}

	bool IHBranch::Connected(IHBranch* other)
	{
		std::vector<LONG>::iterator li1;
		std::vector<LONG>::iterator li2;
		for (li1 = _connections.begin(); li1 < _connections.end(); li1++)
		{
			for (li2 = other->_connections.begin(); li2 < other->_connections.end(); li2++)
			{
				if ((*li1) == (*li2))return true;
			}
		}
		return false;
	}

	void IHBranch::PushGuidesData()
	{
		IHLevel* level = _levels.back();
		std::vector<IHGuide*>::iterator gi;
		std::vector<IHVertex*>::iterator vi;
		IHVertex* last;

		// get guides walking on mesh
		for (gi = _guides.begin(); gi < _guides.end(); gi++)
		{
			if ((*gi)->_walking)
			{
				last = (*gi)->_vertices.back();
				for (vi = level->_vertices.begin(); vi < level->_vertices.end(); vi++)
				{
					if (last->IsNeighbor(*vi))
					{
						(*gi)->_vertices.push_back(*vi);
						break;
					}
				}
			}
		}
		// get drived guides
		for (gi = _guides.begin(); gi < _guides.end(); gi++)
		{
			if (!(*gi)->_walking)
			{
				(*gi)->PushVirtualVertices();
			}
		}
	}

	//------------------------------------
	// Tip Class
	//------------------------------------
	IHTip::IHTip()
	{
	}

	IHTip::~IHTip()
	{
		_polygons.clear();
		_borders.clear();
		_vertices.clear();
		_specials.clear();
		_guides.clear();
	}

	ULONG IHTip::GetNbPossibilities()
	{
		std::vector<IHVertex*>::iterator vi;
		std::vector<IHEdge*>::iterator ei;
		_nbpossibilities = 0;
		for (vi = _vertices.begin(); vi < _vertices.end(); vi++)
		{
			ULONG tnb = 0;
			for (ei = (*vi)->_edges.begin(); ei < (*vi)->_edges.end(); ei++)
			{
				if (!(*ei)->_tip)tnb++;
			}
			if (tnb > _nbpossibilities)_nbpossibilities = tnb;
		}
		return _nbpossibilities;
	}

	void IHTip::PushVertices(IHVertex* v)
	{
		std::vector<IHVertex*>::iterator vi;
		for (vi = _vertices.begin(); vi < _vertices.end(); vi++)
		{
			if ((*vi)->_id == v->_id)return;
		}
		_vertices.push_back(v);
	}


	void IHTip::PushSpecials(IHVertex* v)
	{
		std::vector<IHEdge*>::iterator ei;

		std::vector<IHVertex*>::iterator vi;
		for (vi = _specials.begin(); vi < _specials.end(); vi++)
		{
			if ((*vi)->_id == v->_id)return;
		}
		v->_special = true;
		_specials.push_back(v);
	}

	void IHTip::GetSpecials()
	{
		std::vector<IHVertex*>::iterator vi;;
		for (vi = _vertices.begin(); vi < _vertices.end(); vi++)
		{
			(*vi)->IsSpecial();
			if ((*vi)->_special)PushSpecials(*vi);
		}
	}

	void IHTip::PushGuideData(IHGuide* guide)
	{
		std::vector<IHGuide*>::iterator gi;
		for (gi = _guides.begin(); gi < _guides.end(); gi++)
		{
			if ((*gi)->_id == guide->_id)return;
		}
		_guides.push_back(guide);
	}

	void IHTip::ClearGuideData()
	{
		_guides.clear();
	}

	IHGuide* IHTip::GetGuideByVertexID(ULONG id)
	{
		std::vector<IHGuide*>::iterator gi;
		for (gi = _guides.begin(); gi < _guides.end(); gi++)
		{
			if ((*gi)->_vertices.back()->_id == id)
				return (*gi);
		}
		return NULL;
	}

}