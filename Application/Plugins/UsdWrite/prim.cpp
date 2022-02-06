#include "prim.h"
#include "utils.h"


X2UPrim::X2UPrim(std::string path, const CRef& ref)
  : _fullname(path)
{
  _xObj = X3DObject(ref);
  _xPrim = _xObj.GetActivePrimitive();
  _xID = _xObj.GetObjectID();
}

X2UPrim::~X2UPrim()
{

}

void X2UPrim::InitExtentAttribute()
{
  _attributes["extent"] =
    X2UAttribute(
      UsdGeomGprim(_prim).CreateExtentAttr(VtValue(), true),
      X2U_DATA_VECTOR3,
      X2U_PRECISION_DOUBLE,
      true);

  // set default value
  // bbox computation have to be done before calling this
  _attributes["extent"].WriteSample((const void*)&_bbox.GetRange().GetMin(),
    2, UsdTimeCode::Default());
}

void X2UPrim::InitTransformAttribute()
{
  GfMatrix4d dstMatrix;
  X2UGetLocalTransformAtTime(_xObj, dstMatrix);
  _xformOp = UsdGeomGprim(_prim).AddTransformOp();
  UsdAttribute xfoAttr = _xformOp.GetAttr();

  _attributes["xform"] =
    X2UAttribute(
      xfoAttr,
      X2U_DATA_MATRIX4,
      X2U_PRECISION_DOUBLE,
      false);

  // set default value
   _attributes["xform"].WriteSample((const void*)&dstMatrix, 1, UsdTimeCode::Default());
}

void X2UPrim::InitVisibilityAttribute()
{
  bool visibility = X2UGetObjectVisibility(_xObj);
  UsdAttribute visibilityAttr = UsdGeomGprim(_prim).CreateVisibilityAttr();

  _attributes["visibility"] =
    X2UAttribute(
      visibilityAttr,
      X2U_DATA_BOOL,
      X2U_PRECISION_SINGLE,
      false);

  // set default value
  X2UAttribute& item = GetAttribute("visibility");
  if (visibility)item.WriteSample(UsdGeomTokens->visible, UsdTimeCode::Default());
  else item.WriteSample(UsdGeomTokens->invisible, UsdTimeCode::Default());
}

void X2UPrim::WriteExtentSample(double t)
{
  X2UAttribute& item = GetAttribute("extent");
  item.WriteSample((const void*)&_bbox.GetRange().GetMin()[0], 2, UsdTimeCode(t));
}

void X2UPrim::WriteTransformSample(double t)
{
  GfMatrix4d dstMatrix;
  X2UGetLocalTransformAtTime(_xObj, dstMatrix, t);
  X2UAttribute& item = GetAttribute("xform");
  item.WriteSample((const void*)&dstMatrix, 1, UsdTimeCode(t));
}

void X2UPrim::WriteVisibilitySample(double t)
{
  bool visibility = X2UGetObjectVisibility(_xObj);
  X2UAttribute& item = GetAttribute("visibility");
  if(visibility)item.WriteSample(UsdGeomTokens->inherited, UsdTimeCode(t));
  else item.WriteSample(UsdGeomTokens->invisible, UsdTimeCode(t));
}

bool X2UPrim::InitAttributeFromICE(
  const Geometry& geom,
  const CString& iceAttrName,
  const CString& usdAttrName,
  SdfValueTypeName usdDataType
)
{

  CRefArray attributes = geom.GetICEAttributes();
  int iceAttrIndex;
  ICEAttribute iceAttr = X2UGetICEAttributeFromArray(attributes, iceAttrName, iceAttrIndex);

  if (iceAttrIndex >= 0)
  {
    UsdAttribute usdAttr = _prim.CreateAttribute(TfToken(usdAttrName.GetAsciiString()), usdDataType);

    _attributes[usdAttrName.GetAsciiString()] =
      X2UAttribute(
        usdAttr,
        iceAttrIndex
      );
    return true;
  }
  else return false;
}

void X2UPrim::WriteSampleFromICE(const Geometry& geom, UsdTimeCode t, const std::string& attrName)
{
  auto it = _attributes.find(attrName);
  if (it != _attributes.end())
  {
    X2UAttribute attr = it->second;
    it->second.WriteSample(geom, t);
  }
}


