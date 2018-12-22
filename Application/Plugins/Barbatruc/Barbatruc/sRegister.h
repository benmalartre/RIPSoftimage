#ifndef _bbppRegister_
#define _bbppRegister_

#include <xsi_application.h>
#include <xsi_context.h>
#include <xsi_pluginregistrar.h>
#include <xsi_status.h>
#include <xsi_argument.h>
#include <xsi_command.h>
#include <xsi_utils.h>
#include <xsi_uitoolkit.h>
#include <xsi_progressbar.h>
#include <xsi_X3DObject.h>
#include <xsi_Primitive.h>
#include <xsi_Geometry.h>
#include <xsi_KinematicState.h>
#include <xsi_Kinematics.h>
#include <xsi_Point.h>
#include <xsi_PolygonMesh.h>
#include <xsi_Facet.h>
#include <xsi_Model.h>
#include <xsi_CustomOperator.h>
#include <xsi_OperatorContext.h>
#include <xsi_Factory.h>
#include <xsi_Selection.h>

#include "bbppGeoCache.h"

using namespace XSI;
using namespace MATH;

class sGeoCache  
{  
public:
	sGeoCache(){};
	~sGeoCache(){};

	CStatus Update(OperatorContext&);
	bbppGeoCache _reader;
	CVector3Array _vertices;
	CLongArray _polygons;
};

#endif