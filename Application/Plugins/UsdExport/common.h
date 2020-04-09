#pragma once

#include <xsi_application.h>
#include <xsi_context.h>
#include <xsi_pluginregistrar.h>
#include <xsi_status.h>
#include <xsi_argument.h>
#include <xsi_command.h>
#include <xsi_menu.h>
#include <xsi_factory.h>
#include <xsi_project.h>
#include <xsi_scene.h>
#include <xsi_model.h>
#include <xsi_x3dobject.h>
#include <xsi_kinematics.h>
#include <xsi_primitive.h>
#include <xsi_polygonmesh.h>
#include <xsi_geometryaccessor.h>
#include <xsi_cluster.h>
#include <xsi_nurbscurvelist.h>
#include <xsi_nurbscurve.h>
#include <xsi_clusterproperty.h>
#include <xsi_polygonnode.h>
#include <xsi_property.h>
#include <xsi_point.h>
#include <xsi_iceattribute.h>
#include <xsi_iceattributedataarray.h>
#include <xsi_matrix4.h>
#include <xsi_matrix4f.h>

#include <pxr/pxr.h>
#include <pxr/base/tf/hash.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usdGeom/mesh.h>

#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/vec2d.h>
#include <pxr/base/gf/vec3i.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/vec4i.h>
#include <pxr/base/gf/vec4f.h>
#include <pxr/base/gf/vec4d.h>
#include <pxr/base/gf/quatf.h>
#include <pxr/base/gf/quatd.h>
#include <pxr/base/gf/matrix3f.h>
#include <pxr/base/gf/matrix3d.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/range3f.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/bbox3d.h>

#include <iostream>
#include <vector>
#include <map>
#include <memory>

using namespace XSI;
using namespace pxr;

// softimage log message
#define LOG(msg) Application().LogMessage(msg);

// Scene time infos
struct X2UExportTimeInfos {
  double startFrame;
  double endFrame;
  double sampleRate;
};

enum X2UDataType {
  X2U_DATA_NULL,
  X2U_DATA_BOOL,
  X2U_DATA_LONG,
  X2U_DATA_FLOAT,
  X2U_DATA_DOUBLE,
  X2U_DATA_VECTOR2,
  X2U_DATA_VECTOR3,
  X2U_DATA_VECTOR4,
  X2U_DATA_COLOR4,
  X2U_DATA_ROTATION,
  X2U_DATA_QUATERNION,
  X2U_DATA_MATRIX3,
  X2U_DATA_MATRIX4,
  X2U_DATA_STRING
};

enum X2UDataPrecision {
  X2U_PRECISION_SINGLE,
  X2U_PRECISION_DOUBLE
};

enum X2UPrimvarInterpolation {
  X2U_INTERPOLATION_CONSTANT,
  X2U_INTERPOLATION_UNIFORM,
  X2U_INTERPOLATION_VARYING,
  X2U_INTERPOLATION_VERTEX,
  X2U_INTERPOLATION_FACEVARYING
};

enum X2UWriteOptions {
  X2U_WRITE_COLORS  = 1,
  X2U_WRITE_NORMALS = 2,
  X2U_WRITE_UVS     = 4,
  X2U_WRITE_CUSTOM  = 8
};

static size_t X2UGetDataSize(X2UDataType type, X2UDataPrecision precision)
{
  switch (type)
  {
  case X2U_DATA_BOOL:
    return sizeof(bool);
  case X2U_DATA_LONG:
    return sizeof(LONG);
  case X2U_DATA_FLOAT:
    return sizeof(float);
  case X2U_DATA_DOUBLE:
    return sizeof(double);
  case X2U_DATA_VECTOR2:
    if(precision == X2U_PRECISION_DOUBLE)
      return sizeof(GfVec2d);
    else
      return sizeof(GfVec2f);
  case X2U_DATA_VECTOR3:
    if (precision == X2U_PRECISION_DOUBLE)
      return sizeof(GfVec3d);
    else
      return sizeof(GfVec3f);
  case X2U_DATA_VECTOR4:
    if (precision == X2U_PRECISION_DOUBLE)
      return sizeof(GfVec4d);
    else
      return sizeof(GfVec4f);
  case X2U_DATA_COLOR4:
    if (precision == X2U_PRECISION_DOUBLE)
      return sizeof(GfVec4d);
    else
      return sizeof(GfVec4f);
  case X2U_DATA_ROTATION:
    if (precision == X2U_PRECISION_DOUBLE)
      return sizeof(GfVec4d);
    else
      return sizeof(GfVec4f);
  case X2U_DATA_QUATERNION:
    if (precision == X2U_PRECISION_DOUBLE)
      return sizeof(GfQuatd);
    else
      return sizeof(GfQuatf);
  case X2U_DATA_MATRIX3:
    if (precision == X2U_PRECISION_DOUBLE)
      return sizeof(GfMatrix3d);
    else
      return sizeof(GfMatrix3f);
  case X2U_DATA_MATRIX4:
    if (precision == X2U_PRECISION_DOUBLE)
      return sizeof(GfMatrix4d);
    else
      return sizeof(GfMatrix4f);
  case siICENodeDataString:
    return sizeof(std::string);
  default:
    return 1;
  }
}

static X2UDataType X2UDataTypeFromICEType(siICENodeDataType type)
{
  switch (type)
  {
  case siICENodeDataBool:
    return X2U_DATA_BOOL;

  case siICENodeDataLong:
    return X2U_DATA_LONG;

  case siICENodeDataFloat:
    return X2U_DATA_FLOAT;

  case siICENodeDataVector2:
    return X2U_DATA_VECTOR2;

  case siICENodeDataVector3:
    return X2U_DATA_VECTOR3;

  case siICENodeDataVector4:
    return X2U_DATA_VECTOR4;

  case siICENodeDataRotation:
    return X2U_DATA_QUATERNION;

  case siICENodeDataQuaternion:
    return X2U_DATA_ROTATION;

  case siICENodeDataColor4:
    return X2U_DATA_COLOR4;

  case siICENodeDataMatrix33:
    return X2U_DATA_MATRIX3;

  case siICENodeDataMatrix44:
    return X2U_DATA_MATRIX4;
  
  default:
    return X2U_DATA_NULL;
  }
}

// templated copy data
template<typename SRC, typename DST>
void X2UCopyData(const SRC* src, DST* dst, size_t num)
{
  if (sizeof(SRC) == sizeof(DST))
  {
    memcpy(dst, src, num * sizeof(SRC));
  }
  else
  {
    for (int i = 0; i < num; ++i)
    {
      dst[i] = static_cast<DST>(src[i]);
    }
  }
}

// templated cast tuppled data
template<typename SRC, typename DST>
void X2UCastTuppledData(const SRC* src, DST* dst, size_t num, int srcN, int dstN)
{
  int minN = srcN > dstN ? dstN : srcN;
  for (int i = 0; i < num; ++i)
  {
    for(int j=0;j<minN;++j)dst[i][j] = src[i][j];
  }
}