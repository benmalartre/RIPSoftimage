#include "prim.h"
#include "utils.h"

U2XPrim::U2XPrim(const pxr::UsdPrim& prim)
  :_prim(prim), _xform(pxr::GfMatrix4f(1))
{
}

U2XPrim::~U2XPrim()
{
}

void U2XPrim::GetBoundingBox(pxr::UsdGeomBBoxCache& bboxCache)
{
  _bbox = bboxCache.ComputeWorldBound(_prim);
}

void U2XPrim::GetVisibility(const pxr::UsdTimeCode& timeCode)
{
  pxr::UsdAttribute visibilityAttr = pxr::UsdGeomGprim(_prim).GetVisibilityAttr();
  pxr::TfToken visibility;
  if (visibilityAttr.IsValid())
  {
    visibilityAttr.Get(&visibility);
  }
  _visibility = true;
}

void U2XPrim::GetXform(const pxr::UsdTimeCode& timeCode)
{
  pxr::GfMatrix4d xform;
  bool resetXformStack;
  if(pxr::UsdGeomXformable(_prim).GetLocalTransformation(&xform, &resetXformStack, timeCode))
    _xform = pxr::GfMatrix4f(xform);

}


