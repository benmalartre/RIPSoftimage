#pragma once

#include <pxr/pxr.h>
#include <pxr/usd/usd/attribute.h>

enum U2XAttributeType : short {
  ATTR_UNAUTHORED,
  ATTR_NORMAL,
  ATTR_PRIMVAR
};

enum U2XAttributeInterpolation : short {
  INTERPOLATION_CONSTANT,
  INTERPOLATION_UNIFORM,
  INTERPOLATION_VARYING,
  INTERPOLATION_VERTEX,
  INTERPOLATION_FACEVARYING
};

// Prim base class
class U2XAttribute {
public:
  U2XAttribute(const pxr::UsdAttribute& attr, U2XAttributeType type);
  virtual ~U2XAttribute();

  const pxr::UsdAttribute& Get() { return _attr; };
  const pxr::VtValue& GetValue(const pxr::UsdTimeCode& timeCode);
  U2XAttributeInterpolation GetInterpolation() { return _interpolation; };

protected:
  pxr::UsdAttribute         _attr;
  pxr::TfToken              _name;
  U2XAttributeType          _type;
  U2XAttributeInterpolation _interpolation;
  bool                      _animated;
  size_t                    _hash;
  pxr::VtValue              _value;
};
