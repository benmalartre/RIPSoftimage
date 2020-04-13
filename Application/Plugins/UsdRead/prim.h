#pragma once

#include "GL/glew.h"
#include <pxr/pxr.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/gprim.h>

#include "vertexBuffer.h"
#include "vertexArray.h"

// Prim base class
class U2XPrim {
public:
  U2XPrim(const pxr::UsdPrim& prim);
  virtual ~U2XPrim();

  virtual void Init()=0;
  virtual void Term()=0;
  virtual void Update(double t)=0;
  virtual void Prepare() = 0;
  virtual void Draw(GLuint modelUniform)=0;

  void GetBoundingBox(pxr::UsdGeomBBoxCache& bboxCache);
  const float* GetMatrix() { return (const float*)&_xform[0][0]; };

  void GetVisibility(const pxr::UsdTimeCode& timeCode);
  void GetXform(const pxr::UsdTimeCode& timeCode);

protected:
  pxr::UsdPrim                 _prim;
  pxr::SdfPath                 _path;
  pxr::GfBBox3d                _bbox;
  bool                         _visibility;
  pxr::GfMatrix4f              _xform;
  U2XVertexArray               _vao;
};
