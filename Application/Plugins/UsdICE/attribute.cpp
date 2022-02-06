#include "attribute.h"
#include "utils.h"

U2XAttribute::U2XAttribute(const pxr::UsdAttribute& attr, U2XAttributeType type)
  : _attr(attr)
  , _animated(false)
  , _type(type)
  , _interpolation(INTERPOLATION_CONSTANT)
  , _hash(0)
{
  _name = attr.GetName();
  _animated = attr.GetNumTimeSamples() > 1;
  if (type == ATTR_PRIMVAR)
  {
    pxr::UsdGeomPrimvar primvar(attr);
    TfToken interpolation(primvar.GetInterpolation());
    if (interpolation == pxr::UsdGeomTokens->constant)
      _interpolation = INTERPOLATION_CONSTANT;
    else if (interpolation == pxr::UsdGeomTokens->uniform)
      _interpolation = INTERPOLATION_UNIFORM;
    else if (interpolation == pxr::UsdGeomTokens->varying)
      _interpolation = INTERPOLATION_VARYING;
    else if (interpolation == pxr::UsdGeomTokens->vertex)
      _interpolation = INTERPOLATION_VERTEX;
    else if (interpolation == pxr::UsdGeomTokens->faceVarying)
      _interpolation = INTERPOLATION_FACEVARYING;
  }
  attr.Get(&_value, pxr::UsdTimeCode::Default());

  _hash = _value.GetHash();
}

U2XAttribute::~U2XAttribute()
{

}

const VtValue& U2XAttribute::GetValue(const pxr::UsdTimeCode& timeCode)
{
  _attr.Get(&_value, timeCode);
  return _value;
}


