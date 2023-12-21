#ifndef U2I_PRIM_H
#define U2I_PRIM_H

#include <pxr/pxr.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/gprim.h>
#include <pxr/usd/usdGeom/primvarsApi.h>
#include <pxr/imaging/garch/glApi.h>

// Prim definition node
class U2IPrim {
public:
  pxr::SdfPath _path;
};

#endif