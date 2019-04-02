// Voronoi.h
#ifndef _VORONOI_H_
#define _VORONOI_H_

#include <xsi_pluginregistrar.h>
#include <xsi_application.h>
#include <xsi_factory.h>
#include <xsi_customoperator.h>
#include <xsi_operatorcontext.h>
#include <xsi_x3dobject.h>
#include <xsi_primitive.h>
#include <xsi_geometry.h>
#include <xsi_polygonmesh.h>
#include <xsi_point.h>
#include <xsi_command.h>
#include <xsi_selection.h>
#include <xsi_model.h>

#include  "HEMesh.h"

using namespace XSI;
using namespace MATH;

// CVoronoi Class
//------------------------------------
class CVoronoi
{  
public:
	CVoronoi(){};
	~CVoronoi(){};

	CStatus Update(OperatorContext&);
	void  BuildVoronoi();
	void BuildMesh();
	void CleanVoronoi();
	void InitVoronoi(LONG nb);

private:
	vector<HEMesh> m_voronoiCells;
	LONG m_numPoints;
	float m_offset;
	CVector3Array m_containerVertices;
	CLongArray m_containerPolygons;
	CVector3Array m_positions;
	HEPlane m_P;
	CVector3Array m_outVertices;
	CLongArray m_outPolygons;
};

#define LOG(_msg) Application().LogMessage(CString(_msg));

#endif