/*-------------------------------------------------
    ICERigRegister.h
 
    Include needed SDK headers
    Include needed Windows headers
    Define some global constants           
-------------------------------------------------*/
#ifndef IR_REGISTER_H
#define IR_REGISTER_H

#include <xsi_application.h>
#include <xsi_uitoolkit.h>
#include <xsi_context.h>
#include <xsi_pluginregistrar.h>
#include <xsi_status.h>

#include <xsi_model.h>
#include <xsi_icenodecontext.h>
#include <xsi_icenodedef.h>
#include <xsi_command.h>
#include <xsi_argument.h>
#include <xsi_factory.h>
#include <xsi_longarray.h>
#include <xsi_doublearray.h>
#include <xsi_controlpoint.h>
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
#include <xsi_kinematicstate.h>
#include <xsi_kinematics.h>
#include <xsi_port.h>
#include <xsi_operator.h>
#include <xsi_customoperator.h>
#include <xsi_operatorcontext.h>
#include <xsi_point.h>
#include <xsi_nurbsdata.h>
#include <xsi_selection.h>
#include <xsi_x3dobject.h>
#include <xsi_primitive.h>
#include <xsi_polygonmesh.h>
#include <xsi_nurbscurve.h>
#include <xsi_nurbscurvelist.h>
#include <xsi_facet.h>
#include <xsi_edge.h>
#include <xsi_vertex.h>
#include <xsi_triangle.h>
#include <xsi_trianglevertex.h>
#include <xsi_iceattribute.h>
#include <xsi_iceattributedataarray.h>
#include <xsi_iceattributedataarray2D.h>
#include <xsi_boolarray.h>
#include <xsi_indexset.h>
#include <xsi_random.h>
#include <xsi_toolcontext.h>
#include <xsi_menu.h>
#include <xsi_menuitem.h>

// C++
#include <vector>
#include <map>
#include <iostream>
#include <algorithm>
#include <math.h>
#include <string.h>
#include <utility> // make_pair

#include "IROpenGL.h"
#include "IROGLPrimitives.h"

using namespace XSI; 
using namespace MATH;
using namespace std;

namespace ICERIG {
  #define MAX(a,b)  ((a)<(b) ? (b)  : (a))
  #define MIN(a,b)  ((a)>(b) ? (b)  : (a))
  #define ABS(a)    ((a)<(0) ? (-a) : (a))

  static unsigned char ICERigDataR = 200;
  static unsigned char ICERigDataG = 255;
  static unsigned char ICERigDataB = 200;

  static unsigned char ICERigNodeR = 154;
  static unsigned char ICERigNodeG = 188;
  static unsigned char ICERigNodeB = 206;

  // Color constants
  #define IRInvalidIndex          -1
  #define IRSelectedHandleColor   RGBA_COLOR(255, 255,   0, 255)
  #define IRXHandleColor          RGBA_COLOR(  255,   0, 0, 255)
  #define IRYHandleColor          RGBA_COLOR(  0,   255, 0, 255)
  #define IRZHandleColor          RGBA_COLOR(  0,   0, 255, 255)

  #define IRXYHandleColor         RGBA_COLOR(  255, 255,   0, 255)
  #define IRXZHandleColor         RGBA_COLOR(  255, 255,   0, 255)
  #define IRYZHandleColor         RGBA_COLOR(  255,   0, 255, 255)

  // Other constants
  #define IRPickSize              14    // pixels
  #define IRHandleDragThreshold   5     // pixels
  #define IRGestureDragThreshold  10    // pixels
  #define IRShapeHandleSize       50    // pixels
  #define IROffsetHandleSize      100   // pixels
  #define IRMoveHandleSize        100

  // Shortcut Key constants
  #define IRSwitchModeKey 0x09 // VK_TAB

  // Tool Modes
  enum IRToolMode
  {
    IRInvalidMode = -1,
    IRShapeMode,
    IRMoveMode,
    IROffsetMode,
    IRUpVectorMode,
    IRSkeletonMode
  };

  //Attributes saved in cluster property
  enum IRElemAttribute
  {
    IRWidth = 0,
    IRDepth,
    IROffset,
    IRUpVector
  };

  enum IRElemType
  {
    IRNoType = -1,//invalid type
    IRSkeletonType,
    IRMuscleType
  };

  enum IRAxis
  {
    IRNoAxis = -1,
    IRAxisX,
    IRAxisY,
    IRAxisZ,
    IRAxisXY,
    IRAxisXZ,
    IRAxisYZ,
    IRAxisXYZ
  };

  static const char* IR_PROP_BUILDER = "RigBuilder";
  static const char* IR_PROP_SKELETON = "RigElement";
  static const char* IR_PROP_MUSCLE = "MuscleElement";
  static const char* IR_PROP_CONTROL = "ControlElement";

  inline CRotation GetRotationFromTwoVectors(CVector3 dir, CVector3 up = CVector3(0, 1, 0))
  {
    CVector3 cross;
    CVector3 side;

    dir.NormalizeInPlace();
    up.NormalizeInPlace();
    cross.Cross(dir, up);
    cross.NormalizeInPlace();
    side.Cross(dir, cross);
    side.NormalizeInPlace();

    CMatrix3 m;
    m.Set(cross.GetX(), cross.GetY(), cross.GetZ(),
	    dir.GetX(), dir.GetY(), dir.GetZ(),
	    side.GetX(), side.GetY(), side.GetZ());

    CRotation r;
    r.SetFromMatrix(m);
    return r;
  }

  inline CVector3 RotateVector(CVector3& v, CQuaternion& quat)
  {
    float len = (float)v.GetLength();
    CVector3 vn;
    CQuaternion q2;

    vn.Normalize(v);
    q2.Conjugate(quat);

    CQuaternion vecQuat, resQuat;

    vecQuat.PutX(vn.GetX());
    vecQuat.PutY(vn.GetY());
    vecQuat.PutZ(vn.GetZ());

    resQuat.Mul(vecQuat, q2);
    resQuat.Mul(quat, resQuat);

    CVector3 out(resQuat.GetX(), resQuat.GetY(), resQuat.GetZ());
    out.NormalizeInPlace();
    out.ScaleInPlace(len);
    return out;
  }

  #define LOG(_msg) Application().LogMessage(_msg);
  #define R01() ((float)rand() / (float)RAND_MAX)
  #define RAB(a, b) (((float)rand() / (float)RAND_MAX) * (b - a) + a)

  static unsigned char IRNodeR = 154;
  static unsigned char IRNodeG = 188;
  static unsigned char IRNodeB = 206;

} // ICERIG
#endif
