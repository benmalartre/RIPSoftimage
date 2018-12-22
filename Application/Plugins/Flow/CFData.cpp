//===============================================================================================
// CrappyFlow Data
//===============================================================================================
#include "CFData.h"

//-----------------------------------------------------
// CFCurve Object
//-----------------------------------------------------
CFCurve::~CFCurve()
{
	std::vector<CFSample*>::iterator it;
	for( it=_points.begin();it<_points.end();it++)
		delete (*it);
	_points.clear();

	for(it=_samples.begin();it<_samples.end();it++)
		delete (*it);
	_samples.clear();

	_previous.clear();
	_next.clear();
	_start=NULL;
	_end=NULL;
}

CVector3f CFCurve::GetFirstTangent()
{
	return _points.front()->_dir;
}

CVector3f CFCurve::GetLastTangent()
{
	return _points.back()->_dir;
	/*
	CVector3f out;
	out.Negate(_points.back()->_dir);
	return out;
	*/
}

float CFCurve::GetAngle(CFCurve* other)
{
	CVector3f tan1, tan2;
	// findCurves tangent at connexion point
	if(_startcornerid == other->_startcornerid){tan1 = GetFirstTangent();tan2 = other->GetFirstTangent();}
	else if(_startcornerid == other->_endcornerid){tan1 = GetFirstTangent();tan2 = other->GetLastTangent();}
	else if(_endcornerid == other->_startcornerid){tan1 = GetLastTangent();tan2 = other->GetFirstTangent();}
	else if(_endcornerid == other->_endcornerid){tan1 = GetLastTangent();tan2 = other->GetLastTangent();}
	else return 0.0;

	float angle = tan1.GetAngle(tan2);
	CFDebug(L"CurvePair("+(CString)this->_id+L","+(CString)other->_id+L")");
	CFDebug(L"Angle between Curves at Connexion Point :",angle);
	return angle;
}

//-----------------------------------------------------
// CFCorner Object
//-----------------------------------------------------
void CFCorner::GetAngles()
{
	CFCurve* first = _branches.front();
	std::vector<CFCurve*>::iterator it = _branches.begin()+1;
	_angles.resize(_nbbranches);
	ULONG id=0;
	for(;it<_branches.end();it++)
	{
		_angles[id++] = (*it)->GetAngle(first);
	}
}

bool CFCorner::GetTangent(ULONG id,CVector3f& tan)
{
	CFCurve* crv = _branches[id];
	std::vector<CFPoint*>::iterator pnt;

	for(pnt = _points.begin();pnt<_points.end();pnt++)
	{
		if(crv->_start->_id == (*pnt)->_id)
		{
			tan = crv->GetFirstTangent();
			return true;
		}

		if(crv->_end->_id == (*pnt)->_id)
		{
			tan = crv->GetLastTangent();
			return false;
		}
	}
	return false;
}

/*
bool CFCorner::SortBranchesFunction(CFCurve* first, CFCurve* second)
{
	float angle1 = first->GetAngle(_branches.front());
	float angle2 = second->GetAngle(_branches.front());

	return angle1<angle2;
}
*/

bool CFCorner_SortBranch(std::pair<float,ULONG> a, std::pair<float,ULONG> b)
{
	return a.first < b.first;	
}

void CFCorner_LogBranches(std::vector<std::pair<float,ULONG>>& pairs)
{
	std::vector<std::pair<float,ULONG>>::iterator it;
	CString sAngles = L"";
	CString sID = L"";
	for(it=pairs.begin();it<pairs.end();it++)
	{
		sAngles +=(CString)it->first+L",";
		sID +=(CString)it->second+L",";
	}
	CFDebug(L"Angles Array :",sAngles);
	CFDebug(L"ID Array :",sID);
}

void CFCorner::SortBranches()
{

	CFCurve* last = _branches.front();
	std::vector<CFCurve*>::iterator i1 = _branches.begin()+1;
	std::vector<CFCurve*>::iterator i2;

	float angle1,angle2;
	CFCurve* tmp;
	while(i1<_branches.end())
	{
		angle1 = (*i1)->GetAngle(last);
		i2 = i1+1;
		for(;i2<_branches.end();i2++)
		{
			angle2 = (*i2)->GetAngle(last); 
			if(angle2<angle1)
			{
				CFDebug(L" Swap Element "+(CString)(*i1)->_id+L", with "+(CString)(*i2)->_id);
				tmp = *i1;
				*i1 = *i2;
				*i2 = tmp;
			}
		}
		last = *i1;
		i1++;
	}
	
	/*
	std::vector<std::pair<float,ULONG>> pairs;
	std::vector<ULONG> unsorted;
	float minangle = 360;

	for(ULONG i=1;i<_branches.size();i++)
	{
		std::pair<float,ULONG> pair;
		pair.first = _branches[i]->GetAngle(first);
		pair.second = i;
		pairs.push_back(pair);
		unsorted.push_back(i);
	}

	CFDebug(L"Before Sort --------------------------------------------------");
	CFCorner_LogBranches(pairs);
	std::sort(pairs.begin(),pairs.end(),CFCorner_SortBranch);
	CFDebug(L"After Sort --------------------------------------------------");
	CFCorner_LogBranches(pairs);

	// swap branches
	for(ULONG i=0;i<pairs.size();i++)
	{
		if(pairs[i].second!=(i+1))
		{
			CFDebug(L"Swap "+(CString)(i+1)+L" with "+(CString)pairs[i].second);
			CFCurve* tmp = _branches[i+1];
			_branches[i+1] = _branches[pairs[i].second];
			_branches[pairs[i].second] = tmp;
		}
		//_branches.swap(_branches[pairs[i].second]);
		//_branches[i+1] = _branches[pairs[i].second];
		
		if(pairs[i].second>(i+1))
		{
			CFDebug(L"Swap "+(CString)(i+1)+L" with "+(CString)pairs[i].second);
			CFCurve* tmp = _branches[pairs[i].second];
			_branches[pairs[i].second] = _branches[i+1];
			_branches[i+1] = tmp;
		}
	
	}
	*/
}

//-----------------------------------------------------
// CFData Object
//-----------------------------------------------------
void CFData::Clear()
{
	for(ULONG j=0;j<_corners.size();j++)
		delete _corners[j];
	_corners.clear();

	for(ULONG j=0;j<_curves.size();j++)
		delete _curves[j];
	_curves.clear();

	for(ULONG j=0;j<_points.size();j++)
		delete _points[j];
	_points.clear();

	_nbc = 0;
	_nbpoints = 0;
	_nbpolygons = 0;
	_nbsamples = 0;
	_nbvertices = 0;
}

void CFData::GetData(CICEGeometry& geom)
{
	ULONG nbc = geom.GetSubGeometryCount();
	CICEGeometry subgeom;
	CMatrix4f matrix;
	
	CDoubleArray pos;
	CVector3f diff;

	// Create one branch along each curve
	for(ULONG i=0; i<nbc ;i++)
	{
		subgeom = geom.GetSubGeometry(i);
		CFCurve* crv = new CFCurve(i);
		float l = 0.0;

		subgeom.GetPointPositions(pos);
		subgeom.GetTransformation(matrix);
		CFLocalToGlobalPositionArray(matrix, pos);
		ULONG nbp = subgeom.GetPointPositionCount();

		for(ULONG x=0;x<nbp;x++)
		{
			CFSample* sample = new CFSample();
			crv->_points.push_back(sample);

			sample->_pos.Set( pos[x*3],pos[x*3+1],pos[x*3+2]);
			sample->_scl.Set(1,1,1);
			
			if(x == 0)
			{
				diff.Sub(CVector3f(pos[x*3],pos[x*3+1],pos[x*3+2]),CVector3f(pos[x*3+3],pos[x*3+4],pos[x*3+5]));
				l += diff.GetLength();
				sample->_dir.Normalize(diff);
			}

			else if(x < nbp-1)
			{
				diff.Sub(CVector3f(pos[x*3],pos[x*3+1],pos[x*3+2]),CVector3f(pos[x*3+3],pos[x*3+4],pos[x*3+5]));
				l += diff.GetLength();
				sample->_dir.Normalize(diff);
				sample->_dir.LinearlyInterpolate(crv->_points[x-1]->_dir,crv->_points[x]->_dir,0.5);
			}
		
			else
			{
				diff.Sub(CVector3f(pos[x*3-3],pos[x*3-2],pos[x*3-1]),CVector3f(pos[x*3],pos[x*3+1],pos[x*3+2]));
				sample->_dir.Normalize(diff);
			}
		}
		
		crv->_length = l;
		crv->_start = new CFPoint(crv->_points.front()->_pos,crv->_id,true,false);
		crv->_start->_id = _nbpoints++;
		_points.push_back(crv->_start);

		crv->_end = new CFPoint(crv->_points.back()->_pos,crv->_id,false,true);
		crv->_end->_id = _nbpoints++;
		_points.push_back(crv->_end);

		_curves.push_back(crv);
	
	}

	_nbc = nbc;

	for(ULONG c=0;c<_nbc;c++)
	{
		CFDebug(L"Curve ID",c,"Have",_curves[c]->_points.size(),L"Points");
	}
}

void CFData::CreateCorners()
{
	CVector3f diff;
	CFCorner* corner = NULL;
	ULONG cornerid = 0;
	for(ULONG p1=0;p1<_points.size();p1++)
	{
		if(_points[p1]->_connected)continue;

		//find how many points can connect
		std::vector<LONG> ids;
		for(ULONG p2=0;p2<_points.size();p2++)
		{
			if(p1==p2 || _points[p1]->_crvid == _points[p2]->_crvid)continue;

			diff.Sub(_points[p1]->_pos,_points[p2]->_pos);
			if(diff.GetLength()<1.0)
			{
				ids.push_back((LONG)p2);
			}
		}

		if(ids.size()>0)
		{
			CFDebug(L"+++++++++++++++++++++++ Add Corner +++++++++++++++++++++++");
			CFCorner* corner = new CFCorner(cornerid++);
			CVector3f avg;
			corner->_nbbranches = 0;
			
			if(_points[p1]->_isstart)
			{
				_curves[_points[p1]->_crvid]->_startconnected = true;
				_curves[_points[p1]->_crvid]->_startcornerid = corner->_id;
				//corner->_outputs.push_back(_curves[_points[p1]->_crvid]);
			}
			else if(_points[p1]->_isend)
			{
				_curves[_points[p1]->_crvid]->_endconnected = true;
				_curves[_points[p1]->_crvid]->_endcornerid = corner->_id;
				//corner->_inputs.push_back(_curves[_points[p1]->_crvid]);
			}
			
			corner->_branches.push_back(_curves[_points[p1]->_crvid]);
			corner->_nbbranches++;

			corner->_points.push_back(_points[p1]);

			avg.AddInPlace(_points[p1]->_pos);
			
			CFPoint* pnt;
			for(ULONG i=0;i<ids.size();i++)
			{
				pnt = _points[ids[i]];
				avg.AddInPlace(pnt->_pos);
				pnt->_connected = true;

				//Update Curve Connexion State
				if(pnt->_isstart)
				{
					_curves[pnt->_crvid]->_startconnected = true;
					_curves[pnt->_crvid]->_startcornerid = corner->_id;
				}
				else if(pnt->_isend)
				{
					_curves[pnt->_crvid]->_endconnected = true;
					_curves[pnt->_crvid]->_endcornerid = corner->_id;
				}
				/*
				if(pnt->_isstart)
				{
					corner->_outputs.push_back(_curves[pnt->_crvid]);
				}
				else if(pnt->_isend)
				{
					corner->_inputs.push_back(_curves[pnt->_crvid]);
				}
				*/
				corner->_branches.push_back(_curves[pnt->_crvid]);
				corner->_nbbranches++;
				corner->_points.push_back(pnt);
			}
			avg.ScaleInPlace(1/(float)(ids.size()+1));
			corner->_pos = avg;

			if(corner->_nbbranches>2)
				corner->SortBranches();

			
			_corners.push_back(corner);
		}


	}
	//CFDebug(L"Nb Created Corners : ",(LONG)_corners.size());
	std::vector<CFCorner*>::iterator it = _corners.begin();
	for(;it<_corners.end();it++)
	{
		//_nbvertices += (*it)->_nbbranches;
		_nbpolygons += (*it)->_nbbranches*2+1;
	}
}

void CFData::CreateBranches()
{
	// Sample each curve
	std::vector<CFCurve*>::iterator crv = _curves.begin();
	for(;crv<_curves.end();crv++)
	{
		// need to implement interpolation function
		ULONG nbs = (*crv)->_points.size();

		for(ULONG x=0;x<nbs;x++)
		{
			CFSample* sample = new CFSample();
			(*crv)->_samples.push_back(sample);
			sample->_pos = (*crv)->_points[x]->_pos;
			sample->_scl = (*crv)->_points[x]->_scl;
			sample->_dir = (*crv)->_points[x]->_dir;
		}
		_nbsamples += nbs;
		_nbvertices += (2*nbs);
		_nbpolygons += (5*(nbs-1));
	}
}

void CFData::Build()
{
	_vertices.resize(_nbvertices);
	_polygons.resize(_nbpolygons);
	
	CFSample* sample;
	ULONG vOffset = 0;
	ULONG pOffset = 0;
	CVector3f a,b,o;
	CMatrix3f matrix;

	//Build Corners
	//------------------------------------
	std::vector<CFCorner*>::iterator it = _corners.begin();

	CVector3f pos,tan;
	LONG id=0;
	for(;it<_corners.end();it++)
	{
		for(ULONG x=0;x<(*it)->_nbbranches;x++)
		{
			bool isstart = (*it)->GetTangent(x,tan);
			o.Set((*it)->_pos.GetX(),(*it)->_pos.GetY(),(*it)->_pos.GetZ());
			/*
			a.Set(-_width,0,0);
			b.Set(_width,0,0);
			o.Set((*it)->_pos.GetX(),(*it)->_pos.GetY(),(*it)->_pos.GetZ());
			
			CFDirectionToRotation(matrix,tan,CVector3f(0,1,0));
			tan.ScaleInPlace(_width*2);

			pos.Add(o,tan);

			a.MulByMatrix3InPlace(matrix);
			b.MulByMatrix3InPlace(matrix);

			a.AddInPlace(pos);
			b.AddInPlace(pos);
			*/
			a.Add(o,tan);
			b.Add(o,tan);
			
			_vertices[vOffset] = a;
			_vertices[vOffset+1] = b;
			_polygons[pOffset++] = vOffset;
			_polygons[pOffset++] = vOffset+1;
			vOffset+=2;
			
			if(isstart)
			{
				(*it)->_branches[x]->_start->_leftvertexid = vOffset;
				(*it)->_branches[x]->_start->_rightvertexid = vOffset+1;
			}
			else
			{
				(*it)->_branches[x]->_end->_rightvertexid = vOffset;
				(*it)->_branches[x]->_end->_leftvertexid = vOffset+1;
			}
		}
		_polygons[pOffset++] = -2;
	}

	// Build Branches
	//------------------------------------
	/*
	for(ULONG c=0;c<_curves.size();c++)
	{
		for(ULONG s=0;s<_curves[c]->_samples.size();s++)
		{
			sample = _curves[c]->_samples[s];

			a.Set(-_width,0,0);
			b.Set(_width,0,0);
			o = sample->_pos;

			if(s==0 && _curves[c]->_startconnected)
			{
				//CFDebug(L"Start Point Is Connected",_curves[c]->_start->_pos);
			}
			else if(s==_curves[c]->_samples.size()-1 && _curves[c]->_endconnected)
			{
				//CFDebug(L"End Point Is Connected",_curves[c]->_end->_pos);
			}
			else
			{
				CFDirectionToRotation(matrix,sample->_dir,CVector3f(0,1,0));

				a.MulByMatrix3InPlace(matrix);
				b.MulByMatrix3InPlace(matrix);

				a.AddInPlace(o);
				b.AddInPlace(o);

				_vertices[vOffset].Set(a.GetX(),a.GetY(),a.GetZ());
				_vertices[vOffset+1].Set(b.GetX(),b.GetY(),b.GetZ());
				
				if( s<(_curves[c]->_samples.size()-1))
				{
					_polygons[pOffset++] = vOffset;
					_polygons[pOffset++] = vOffset+1;
					_polygons[pOffset++] = vOffset+3;
					_polygons[pOffset++] = vOffset+2;
					_polygons[pOffset++] = -2;
				}
				
				vOffset+=2;
			}
		}
	}
	*/
}