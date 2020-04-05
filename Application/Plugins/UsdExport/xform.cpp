#include "xform.h"
#include "utils.h"

X2UExportXform::X2UExportXform()
{
}

X2UExportXform::~X2UExportXform()
{ 
}

void X2UExportXform::Init(UsdStageRefPtr& stage, std::string path, const CRef& ref)
{
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath(path));
  _type = X2U_XFORM;
  _ref = ref;
  _fullname = path;
  _prim = xform.GetPrim();

  X3DObject obj(_ref);

  // xform attribute
  {
    GfMatrix4d dstMatrix;
    GetLocalTransformAtTime(obj, dstMatrix);
    _xformOp = xform.AddTransformOp();
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

  // visibility attribute
  {
    /*
    MATH::CTransformation xfo = obj.GetKinematics().GetLocal().GetTransform();
    MATH::CMatrix4 srcMatrix = xfo.GetMatrix4();
    GfMatrix4d dstMatrix;
    X2UConvertMatrix4DoubleToDouble(srcMatrix, dstMatrix);
    _xformOp = xform.AddTransformOp();
    _xformOp.Set<GfMatrix4d>(dstMatrix);
    */
  }

}

void X2UExportXform::WriteSample(double t)
{
  UsdTimeCode timeCode(t);
  X3DObject obj(_ref);

  // xform
  {
    GfMatrix4d dstMatrix;
    GetLocalTransformAtTime(obj, dstMatrix, t);
    X2UExportAttribute& item = GetAttribute("xform");
    item.WriteSample((const void*)&dstMatrix, 1, timeCode);
  }
}

void X2UExportXform::InitDisplayColor()
{
 
}

/*
void X2UExportCurve::GetNormals()
{
  _haveNormals = false;
}

void X2UExportCurve::GetUVs()
{
  _haveUVs = false;
}
*/