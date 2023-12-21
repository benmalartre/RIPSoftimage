#include "prim.h"
#include "utils.h"

U2IPrim::U2IPrim(const pxr::UsdPrim& prim, U2IPrim* parent)
  : _prim(prim)
  , _xform(pxr::GfMatrix4f(1))
  , _parent(parent)
{
}

U2IPrim::~U2IPrim()
{
}

void U2IPrim::GetBoundingBox(pxr::UsdGeomBBoxCache& bboxCache)
{
  _bbox = bboxCache.ComputeWorldBound(_prim);
}

void U2IPrim::GetVisibility(const pxr::UsdTimeCode& timeCode, bool init)
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

void U2IPrim::GetXform(const pxr::UsdTimeCode& timeCode)
{
  pxr::GfMatrix4d xform;
  bool resetXformStack;
  if (pxr::UsdGeomXformable(_prim).GetLocalTransformation(&xform, &resetXformStack, timeCode))
  {
    _xform = pxr::GfMatrix4f(xform);
  }
}


U2IAttributeType U2IPrim::HasAttribute(const pxr::TfToken& name)
{
  pxr::UsdGeomGprim gprim(_prim);

  pxr::UsdGeomPrimvarsAPI api(gprim);
  if (api.GetPrimvar(name).IsDefined())
    return ATTR_PRIMVAR;
  else if (_prim.GetAttribute(name).IsDefined())
    return ATTR_NORMAL;
  else
    return ATTR_UNAUTHORED;
}

U2IAttribute U2IPrim::CreateAttribute(const pxr::TfToken& name, U2IAttributeType type) 
{
  if (type == ATTR_PRIMVAR)
  {
    pxr::UsdGeomPrimvarsAPI api(_prim);
    return U2IAttribute(api.GetPrimvar(name).GetAttr(), ATTR_PRIMVAR);
  }
  else if (type == ATTR_NORMAL)
  {
    return U2IAttribute(_prim.GetAttribute(name), ATTR_NORMAL);
  }
}

void U2IPrim::GetAttributeValue(U2IAttribute& attr, const pxr::UsdTimeCode& timeCode)
{
  attr.GetValue(timeCode);
}

