#include "prim.h"
#include "utils.h"

U2XPrim::U2XPrim(const pxr::UsdPrim& prim, U2XPrim* parent)
  : _prim(prim)
  , _xform(pxr::GfMatrix4f(1))
  , _parent(parent)
{
}

U2XPrim::~U2XPrim()
{
}

void U2XPrim::GetBoundingBox(pxr::UsdGeomBBoxCache& bboxCache)
{
  _bbox = bboxCache.ComputeWorldBound(_prim);
}

void U2XPrim::GetVisibility(const pxr::UsdTimeCode& timeCode, bool init)
{
  if (init || !_parent)
  {
    pxr::UsdAttribute visibilityAttr = pxr::UsdGeomImageable(_prim).GetVisibilityAttr();
    if (visibilityAttr.GetNumTimeSamples() > 1)_visibilityVarying = true;
    else _visibilityVarying = false;

    pxr::TfToken visibility = pxr::UsdGeomImageable(_prim).ComputeVisibility(timeCode);
    if (visibility != pxr::UsdGeomTokens->invisible)_visibility = true;
    else _visibility = false;
  }
  else
  {
    if (_visibilityVarying) {
      pxr::UsdAttribute visibilityAttr = pxr::UsdGeomImageable(_prim).GetVisibilityAttr();
      TfToken visibility;
      visibilityAttr.Get(&visibility, timeCode);
      if (visibility == pxr::UsdGeomTokens->invisible)_visibility = false;
      else {
        _visibility = _parent->_visibility;
      }
    }
  }
}

void U2XPrim::GetXform(const pxr::UsdTimeCode& timeCode)
{
  pxr::GfMatrix4d xform;
  bool resetXformStack;
  if (pxr::UsdGeomXformable(_prim).GetLocalTransformation(&xform, &resetXformStack, timeCode))
  {
    _xform = pxr::GfMatrix4f(xform);
  }
}


U2XAttributeType U2XPrim::HasAttribute(const pxr::TfToken& name)
{
  pxr::UsdGeomPrimvarsAPI api(_prim);

  if (api.GetPrimvar(name).IsDefined())
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
    return U2XAttribute(pxr::UsdGeomPrimvarsAPI(_prim).GetPrimvar(name).GetAttr(), ATTR_PRIMVAR);
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

