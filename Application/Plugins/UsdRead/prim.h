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
#include "attribute.h"


// Prim base class
class U2XPrim {
public:
  U2XPrim(const pxr::UsdPrim& prim);
  virtual ~U2XPrim();

  virtual void Init()=0;
  virtual void Term()=0;
  virtual void Update(double t)=0;
  virtual void Prepare() = 0;
  virtual void Draw()=0;

  const pxr::UsdPrim& Get() { return _prim; };
  void GetBoundingBox(pxr::UsdGeomBBoxCache& bboxCache);
  void SetMatrix(const pxr::GfMatrix4d& m) {
    _xform = pxr::GfMatrix4f(m);
  };
  const float* GetMatrix() { return (const float*)&_xform[0][0]; };
  const float* GetNormalMatrix() { 
    return (const float*)&_xform.GetInverse().GetTranspose()[0][0];
  };
  
  bool IsVisible() { return _visibility; };
  void GetVisibility(const pxr::UsdTimeCode& timeCode);
  void GetXform(const pxr::UsdTimeCode& timeCode);
  U2XAttributeType HasAttribute(const pxr::TfToken& name);
  U2XAttribute CreateAttribute(const pxr::TfToken& name, U2XAttributeType type);
  void GetAttributeValue(U2XAttribute& attr, const pxr::UsdTimeCode& timeCode);

protected:
  pxr::UsdPrim                 _prim;
  pxr::SdfPath                 _path;
  pxr::GfBBox3d                _bbox;
  pxr::GfMatrix4f              _xform;
  bool                         _visibility;
  bool                         _pointsVarying;
  bool                         _topoVarying;
  bool                         _colorsVarying;
  bool                         _haveAuthoredColors;
  bool                         _haveAuthoredOpacities;
  U2XVertexArray               _vao;
};
