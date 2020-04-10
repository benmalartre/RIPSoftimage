#include "prim.h"
#include "utils.h"


X2UExportPrim::X2UExportPrim(std::string path, const CRef& ref)
  : _fullname(path)
{
  _xObj = X3DObject(ref);
  _xPrim = _xObj.GetActivePrimitive();
}

X2UExportPrim::~X2UExportPrim()
{

}

void X2UExportPrim::InitExtentAttribute()
{
  _attributes["extent"] =
    X2UExportAttribute(
      UsdGeomGprim(_prim).CreateExtentAttr(VtValue(), true),
      X2U_DATA_VECTOR3,
      X2U_PRECISION_DOUBLE,
      true);

  // set default value
  // bbox computation have to be done before calling this
  _attributes["extent"].WriteSample((const void*)&_bbox.GetRange().GetMin(),
    2, UsdTimeCode::Default());
}

void X2UExportPrim::InitTransformAttribute()
{
  GfMatrix4d dstMatrix;
  X2UGetLocalTransformAtTime(_xObj, dstMatrix);
  _xformOp = UsdGeomGprim(_prim).AddTransformOp();
  UsdAttribute xfoAttr = _xformOp.GetAttr();

  _attributes["xform"] =
    X2UExportAttribute(
      xfoAttr,
      X2U_DATA_MATRIX4,
      X2U_PRECISION_DOUBLE,
      false);

  // set default value
   _attributes["xform"].WriteSample((const void*)&dstMatrix, 1, UsdTimeCode::Default());
}

void X2UExportPrim::InitVisibilityAttribute()
{
  bool visibility = X2UGetObjectVisibility(_xObj);
  UsdAttribute visibilityAttr = UsdGeomGprim(_prim).CreateVisibilityAttr();

  _attributes["visibility"] =
    X2UExportAttribute(
      visibilityAttr,
      X2U_DATA_BOOL,
      X2U_PRECISION_SINGLE,
      false);

  // set default value
  X2UExportAttribute& item = GetAttribute("visibility");
  if (visibility)item.WriteSample(UsdGeomTokens->inherited, UsdTimeCode::Default());
  else item.WriteSample(UsdGeomTokens->invisible, UsdTimeCode::Default());
}

void X2UExportPrim::WriteExtentSample(double t)
{
  X2UExportAttribute& item = GetAttribute("extent");
  item.WriteSample((const void*)&_bbox.GetRange().GetMin()[0], 2, UsdTimeCode(t));
}

void X2UExportPrim::WriteTransformSample(double t)
{
  GfMatrix4d dstMatrix;
  X2UGetLocalTransformAtTime(_xObj, dstMatrix, t);
  X2UExportAttribute& item = GetAttribute("xform");
  item.WriteSample((const void*)&dstMatrix, 1, UsdTimeCode(t));
}

void X2UExportPrim::WriteVisibilitySample(double t)
{
  bool visibility = X2UGetObjectVisibility(_xObj);
  X2UExportAttribute& item = GetAttribute("visibility");
  if(visibility)item.WriteSample(UsdGeomTokens->inherited, UsdTimeCode(t));
  else item.WriteSample(UsdGeomTokens->invisible, UsdTimeCode(t));
}

bool X2UExportPrim::InitAttributeFromICE(
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
      X2UExportAttribute(
        usdAttr,
        iceAttrIndex
      );
    return true;
  }
  else
  {
    LOG(iceAttrName + " NOT FOUND !!!");
    return false;
  }
}

void X2UExportPrim::WriteSampleFromICE(const Geometry& geom, UsdTimeCode t, const std::string& attrName)
{
  auto it = _attributes.find(attrName);
  if (it != _attributes.end())
  {
    X2UExportAttribute attr = it->second;
    it->second.WriteSample(geom, t);
  }
}


