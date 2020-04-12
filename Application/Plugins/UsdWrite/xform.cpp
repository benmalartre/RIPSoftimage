#include "xform.h"
#include "utils.h"

X2UXform::X2UXform(std::string path, const CRef& ref)
  : X2UPrim(path, ref)
{
}

X2UXform::~X2UXform()
{ 
}

void X2UXform::Init(UsdStageRefPtr& stage)
{
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath(_fullname));
  _prim = xform.GetPrim();

  // xform attribute
  InitTransformAttribute();

  // visibility attribute
  InitVisibilityAttribute();

}

void X2UXform::WriteSample(double t)
{
  UsdTimeCode timeCode(t);

  // xform
  WriteTransformSample(t);

  // visibility
  WriteVisibilitySample(t);
}
