#pragma once

#include "GL/glew.h"
#include <pxr/pxr.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/usd/usdGeom/bboxCache.h>


static const int NUM_TEST_POINTS = 4;

static float TEST_POINTS[NUM_TEST_POINTS * 3] = {
  -1.f, -1.f, 0.f,
  -1.f,  1.f, 0.f,
  1.f,  1.f, 0.f,
  1.f, -1.f, 0.f
};

static float TEST_NORMALS[NUM_TEST_POINTS * 3] = {
  0.f, 0.f, 1.f,
  0.f, 0.f, 1.f,
  0.f, 0.f, 1.f,
  0.f, 0.f, 1.f
};

static float TEST_COLORS[NUM_TEST_POINTS * 3] = {
  1.f, 0.f, 0.f,
  0.f, 1.f, 0.f,
  0.f, 0.f, 1.f,
  1.f, 0.f, 1.f
};

static const int NUM_TEST_INDICES = 6;

static int TEST_INDICES[NUM_TEST_INDICES] = {
  0, 1, 2,
  2, 3, 0
};

// Prim base class
class U2XPrim {
public:
  U2XPrim(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path);
  virtual ~U2XPrim();

  virtual void Init();
  virtual void Term();
  virtual void Update(double t);
  virtual void Draw();

  void GetBoundingBox(pxr::UsdGeomBBoxCache& bboxCache);

protected:
  pxr::UsdPrim                 _prim;
  pxr::SdfPath                 _path;
  pxr::GfBBox3d                _bbox;
  bool                         _visibility;
  pxr::GfMatrix4d              _xform;
  GLuint                       _vao;
  GLuint                       _ebo;
  std::vector<GLuint>          _vbos;
};

typedef std::shared_ptr<U2XPrim> U2XPrimSharedPtr;
typedef std::vector<U2XPrimSharedPtr> U2XPrimSharedPtrList;

