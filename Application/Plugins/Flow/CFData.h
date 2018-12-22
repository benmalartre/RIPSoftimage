#ifndef __CRAPYFLOW_DATA__
#define __CRAPYFLOW_DATA__

#include <xsi_application.h>
#include <xsi_context.h>
#include <xsi_pluginregistrar.h>
#include <xsi_status.h>

#include <xsi_icenodecontext.h>
#include <xsi_icenodedef.h>
#include <xsi_command.h>
#include <xsi_factory.h>
#include <xsi_longarray.h>
#include <xsi_doublearray.h>
#include <xsi_math.h>
#include <xsi_vector2f.h>
#include <xsi_vector3f.h>
#include <xsi_vector4f.h>
#include <xsi_matrix3f.h>
#include <xsi_matrix4f.h>
#include <xsi_rotationf.h>
#include <xsi_quaternionf.h>
#include <xsi_color4f.h>
#include <xsi_shape.h>
#include <xsi_icegeometry.h>
#include <xsi_iceportstate.h>
#include <xsi_indexset.h>
#include <xsi_dataarray.h>
#include <xsi_dataarray2D.h>

#include <vector>
#include <algorithm>    // std::find

using namespace XSI; 
using namespace MATH;
using namespace std;

enum CFDirty
{
	NotDirty = 0,
	JustDirty = 1
};

struct CFPoint
{
	CVector3f _pos;
	ULONG _id;
	ULONG _crvid;
	bool _isstart;
	bool _isend;
	bool _connected;
	ULONG _leftvertexid;
	ULONG _rightvertexid;

	CFPoint(CVector3f& pos, ULONG crvid){_pos = pos; _crvid=crvid;_connected=false;_isstart=false;_isend=false;};
	CFPoint(CVector3f& pos, ULONG crvid,bool isstart, bool isend){_pos = pos; _crvid=crvid;_isstart = isstart;_isend=isend;_connected=false;};
	~CFPoint(){};
};

struct CFSample
{
	float _u;
	CVector3f _pos;
	CVector3f _dir;
	CVector3f _up;
	CVector3f _scl;
};

class CFCurve
{
public:
	ULONG _id;
	CFPoint* _start;
	CFPoint* _end;
	float _length;
	std::vector<CFSample*> _points;
	std::vector<CFSample*> _samples;
	std::vector<CFCurve*> _previous;
	std::vector<CFCurve*> _next;

	bool _startconnected;
	bool _endconnected;
	LONG _startcornerid;
	LONG _endcornerid;

	CFCurve(ULONG id){_id=id;_startconnected=false;_endconnected=false;};
	CVector3f CFCurve::GetFirstTangent();
	CVector3f CFCurve::GetLastTangent();
	float GetAngle(CFCurve* other);
	~CFCurve();
};

class CFCorner
{
public:
	ULONG _id;
	CVector3f _pos;
	ULONG _nbbranches;
	
	std::vector<CFPoint*> _points;
	std::vector<CFCurve*> _branches;
	std::vector<float> _angles;

	bool GetTangent(ULONG id,CVector3f& tan);
	void GetAngles();
	void SortBranches();
	//bool SortBranchesFunction(CFCurve* first, CFCurve* second);
	//void GetBissectors();
	CFCorner(){};
	CFCorner(ULONG id){_id = id;};
	~CFCorner(){_points.clear();_branches.clear();};
};

class CFData
{
public:
	std::vector<CFPoint*> _points;
	std::vector<CFCurve*> _curves;
	std::vector<CFCorner*> _corners;
	ULONG _nbc;
	ULONG _nbpoints;
	ULONG _nbsamples;
	ULONG _nbvertices;
	ULONG _nbpolygons;
	float _width;

	std::vector<CVector3f> _vertices;
	std::vector<LONG> _polygons;

	void GetData(CICEGeometry& geom);
	void CreateBranches();
	void CreateCorners();

	void Clear();
	void Build();
	
};
inline void CFLocalToGlobalPositionArray(CMatrix4f& matrix, CDoubleArray& pos)
{
	CVector3f tmp;
	ULONG nbp = pos.GetCount()/3;
	for(ULONG i=0;i<nbp;i++)
	{
		tmp.Set(pos[i*3],pos[i*3+1],pos[i*3+2]);
		tmp.MulByMatrix4InPlace(matrix);
		pos[i*3] = tmp.GetX();
		pos[i*3+1] = tmp.GetY();
		pos[i*3+2] = tmp.GetZ();
	}
}

inline void CFDirectionToRotation(CMatrix3f& in_mat, CVector3f& in_dir, CVector3f& in_up)
{
	CVector3f v1, v2,dir,up;

	dir.Set(in_dir.GetX(),in_dir.GetY(),in_dir.GetZ());
	up.Set(in_up.GetX(),in_up.GetY(),in_up.GetZ());

	dir.NormalizeInPlace();
	up.NormalizeInPlace();

	v1.Cross(dir, up);
	v2.Cross(v1, dir);

	v1.NormalizeInPlace();
	v2.NormalizeInPlace();

	in_mat.Set(v1.GetX(), v1.GetY(), v1.GetZ(), dir.GetX(), dir.GetY(), dir.GetZ(), v2.GetX(), v2.GetY(), v2.GetZ());
}

inline void CFDebug(CString info)
{
	Application().LogMessage(info);
}

inline void CFDebug(CString Title, CString info)
{
	Application().LogMessage(L"["+Title+"] "+info);
}

inline void CFDebug(CString Title, bool value)
{
	Application().LogMessage(Title+L" "+(CString)value);
}

inline void CFDebug(CString Title, float value)
{
	Application().LogMessage(Title+L" "+(CString)value);
}

inline void CFDebug(CString Title, LONG info)
{
	Application().LogMessage(Title+": "+(CString)info);
}

inline void CFDebug(CString Title, CVector3f& pos)
{
	Application().LogMessage(L"["+Title+L"] ("+(CString)pos.GetX()+L","+(CString)pos.GetY()+L","+(CString)pos.GetZ()+L")");
}

inline void CFDebug(CString subject, LONG id,CString verb, LONG nb , CString what)
{
	Application().LogMessage(subject+L" "+(CString)id+L" "+verb+L" "+(CString)nb+L" "+what);
}


#endif