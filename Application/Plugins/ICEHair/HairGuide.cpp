//------------------------------------------
/*
	Implementation of HairGuide class

                           _,.
                        ,''   `.     __....__ 
                      ,'        >.-''        ``-.__,)
                    ,'      _,''           _____ _,'
                   /      ,'           _.:':::_`:-._ 
                  :     ,'       _..-''  \`'.;.`-:::`:. 
                  ;    /       ,'  ,::'  .\,'`.`. `\::)`  
                 /    /      ,'        \   `. '  )  )/ 
                /    /      /:`.     `--`'   \     '`
                `-._/      /::::)             )
                   /      /,-.:(   , _   `.-' 
                  ;      :(,`.`-' ',`.     ;
                 :       |:\`' )      `-.._\ _
                 |         `:-(             `)``-._ 
                 |           `.`.        /``'      ``:-.-__,
                 :           / `:\ .     :            ` \`-
                  \        ,'   '}  `.   |
               _..-`.    ,'`-.   }   |`-'    
             ,'__    `-'' -.`.'._|   | 
                 ```--..,.__.(_|.|   |::._
                   __..','/ ,' :  `-.|::)_`.
                   `..__..-'   |`.      __,' 
                               :  `-._ `  ;
                                \ \   )  /
                                .\ `.   /
                                 ::    /
                                 :|  ,'
                                 :;,' SSt
                                 `'

*/
#include "HairRegister.h"
#include "HairData.h"

namespace ICEHAIR
{
	//--------------------------------------------
	// Guide object
	//
	//------------------------------------------
	IHGuide::IHGuide()
	{
	}

	IHGuide::~IHGuide()
	{
		ClearVirtualVertices();

		_vertices.clear();
		_guides.clear();
		_weights.clear();
	}

	void IHGuide::ClearVirtualVertices()
	{
		std::vector<IHVirtualVertex*>::iterator vi;
		for (vi = _virtuals.begin(); vi < _virtuals.end(); vi++)
		{
			delete (*vi);
		}
		_virtuals.clear();
	}

	void IHGuide::GetDrivingGuides(IHTip* tip)
	{
		LONG nb = (LONG)tip->_vertices.size();

		_guides.clear();
		CFloatArray length;
		length.Clear();
		float totallength = 0;
		float l;
		CVector3f v;
		IHGuide* g;

		for (LONG vi = 0; vi < nb; vi++)
		{
			if (!tip->_vertices[vi]->_special)
			{
				g = tip->GetGuideByVertexID(tip->_vertices[vi]->_id);
				_guides.push_back(g);

				v.Sub(_vertices.back()->_pos, tip->_vertices[vi]->_pos);
				l = v.GetLength();
				l = 1 / (l*l);
				length.Add(l);
				totallength += l;
			}
		}
		_weights.resize(_guides.size());
		for (LONG vi = 0; vi < (LONG)_guides.size(); vi++)
			_weights[vi] = length[vi] / totallength;

	}

	void IHGuide::PushVirtualVertices()
	{
		CVector3f v, pos, ref;
		ref = _vertices.back()->_pos;
		pos.SetNull();
		std::vector<IHGuide*>::iterator gi;
		ULONG i = 0;
		float l;

		LONG id = (LONG)_virtuals.size();
		IHVirtualVertex* vertex = new IHVirtualVertex();

		//find three guides: closest, furthest and a third in the middle
		std::vector<ULONG> drivers;
		std::vector<float> weights;
		drivers.resize(3);
		weights.resize(3);

		//first get distances from ref to guides
		std::map<float, ULONG> distances;
		int cnt = 0;
		for (gi = _guides.begin(); gi < _guides.end(); gi++)
		{
			v.Sub(ref, (*gi)->_vertices.back()->_pos);
			l = v.GetLength();
			distances.insert(std::pair<float, ULONG>(l, cnt));
			cnt++;
			//Application().LogMessage(L"Insert Distance/ID in Map : ("+(CString)l+L","+(CString)(*gi)->_id+L")");
		}

		// Build a key list and parse it
		std::list<float> key_list;
		for (std::map<float, ULONG>::iterator it = distances.begin(); it != distances.end(); ++it)
		{
			key_list.push_back(it->first);
		}

		key_list.sort();
		int nbg = key_list.size();
		int first = 0;
		int middle = (int)nbg / 3;
		int last = nbg - 1;
		cnt = 0;
		for (std::list<float>::iterator it = key_list.begin(); it != key_list.end(); ++it)
		{
			if (cnt == first)
			{
				drivers[0] = distances[*it];
				weights[0] = _weights[*it];
			}
			else if (cnt == middle)
			{
				drivers[1] = distances[*it];
				weights[1] = _weights[*it];
			}
			else if (cnt == last)
			{
				drivers[2] = distances[*it];
				weights[2] = _weights[*it];
			}
			cnt++;
		}

		float weightsum = weights[0] + weights[1] + weights[2];
		float tl = 0;
		for (int i = 0; i < 3; i++)
		{
			IHGuide* g = _guides[drivers[i]];
			IHVertex* vt;
			vt = g->_vertices.back();
			if (g->_vertices.size() > 1)
				vertex->_previous.push_back(g->_vertices[g->_vertices.size() - 2]);

			tl += g->GetLastSegmentLength();

			vertex->_vertices.push_back(vt);

			vertex->_weights.push_back(weights[i] / weightsum);
			v.Scale(weights[i] / weightsum, vt->_pos);
			pos.AddInPlace(v);
		}

		vertex->_last = _vertices.back();

		vertex->_id = id;
		vertex->GetPosition();
		_virtuals.push_back(vertex);
		_vertices.push_back(vertex);

		/*
			// find three closest guides
			float l;

			CFloatArray dist;
			CFloatArray weights;
			std::vector<ULONG> closest;
			dist.Resize(3);
			closest.resize(3);
			weights.Resize(3);

			dist[0] = dist[1] = dist[2] = 10000;
			closest[0] = closest[1] = closest[2] = _guides.front()->_id;

			int cnt=0;

			for(gi=_guides.begin();gi<_guides.end();gi++)
			{
				v.Sub(ref,(*gi)->_vertices.back()->_pos);
				l = v.GetLength();
				if(l<dist[0])
				{
					dist[2] = dist[1];
					dist[1] = dist[0];
					dist[0] = l;
					closest[2] = closest[1];
					closest[1] = closest[0];
					closest[0] = cnt;
					weights[2] = weights[1];
					weights[1] = weights[0];
					weights[0] = _weights[cnt];
				}
				else if(l<dist[1])
				{
					dist[2] = dist[1];
					dist[1] = l;
					closest[2] = closest[1];
					closest[1] = cnt;
					weights[2] = weights[1];
					weights[1] = _weights[cnt];
				}
				else if(l<dist[2])
				{
					dist[2] = l;
					closest[2] = cnt;
					weights[2] = _weights[cnt];
				}
				cnt++;
			}

			float weightsum = weights[0]+weights[1]+weights[2];
			float tl = 0;
			for(int i=0;i<3;i++)
			{
				Guide* g = _guides[closest[i]];
				Vertex* vt;
				vt = g->_vertices.back();
				if(g->_vertices.size()>1)
					vertex->_previous.push_back(g->_vertices[g->_vertices.size()-2]);

				tl +=g->GetLastSegmentLength();

				vertex->_vertices.push_back(vt);

				vertex->_weights.push_back(weights[i]/weightsum);
				v.Scale(weights[i]/weightsum,vt->_pos);
				pos.AddInPlace(v);
			}

			vertex->_last = _vertices.back();

			vertex->_id = id;
			vertex->GetPosition();
			_virtuals.push_back(vertex);
			_vertices.push_back(vertex);
			*/
	}

	CVector3f IHGuide::GetLastSegment()
	{
		int s = (int)_vertices.size();
		CVector3f d;
		if (s > 1)
		{
			d.Sub(_vertices[s - 1]->_pos, _vertices[s - 2]->_pos);
			return d;
		}
		else return d;
	}

	float IHGuide::GetLastSegmentLength()
	{
		CVector3f d = GetLastSegment();
		return d.GetLength();
	}

}