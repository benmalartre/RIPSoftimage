#include "prim.h"
#include "utils.h"

U2XPrim::U2XPrim(const pxr::UsdPrim& prim)
  : _prim(prim)
  , _xform(pxr::GfMatrix4f(1))
  , _normalMatrix(pxr::GfMatrix4f(1))
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
  pxr::TfToken visibility = pxr::UsdGeomGprim(_prim).ComputeVisibility(timeCode);
  if (visibility != pxr::UsdGeomTokens->invisible)_visibility = true;
  else _visibility = false;
}

void U2XPrim::GetXform(const pxr::UsdTimeCode& timeCode)
{
  pxr::GfMatrix4d xform;
  bool resetXformStack;
  if (pxr::UsdGeomXformable(_prim).GetLocalTransformation(&xform, &resetXformStack, timeCode))
  {
    _xform = pxr::GfMatrix4f(xform);
    _normalMatrix = _xform.GetInverse().GetTranspose();
  }

}


U2XAttributeType U2XPrim::HasAttribute(const pxr::TfToken& name)
{
  pxr::UsdGeomGprim gprim(_prim);

  if (gprim.GetPrimvar(name).IsDefined())
    return ATTR_PRIMVAR;
  else if (_prim.GetAttribute(name).IsDefined())
    return ATTR_NORMAL;
  else
    return ATTR_UNAUTHORED;
}

U2XAttribute U2XPrim::CreateAttribute(const pxr::TfToken& name, U2XAttributeType type) 
{
  if (type == ATTR_PRIMVAR)
  {
    return U2XAttribute(pxr::UsdGeomGprim(_prim).GetPrimvar(name).GetAttr(), ATTR_PRIMVAR);
  }
  else if (type == ATTR_NORMAL)
  {
    return U2XAttribute(_prim.GetAttribute(name), ATTR_NORMAL);
  }
}

void U2XPrim::GetAttributeValue(U2XAttribute& attr, const pxr::UsdTimeCode& timeCode)
{
  attr.GetValue(timeCode);
}

