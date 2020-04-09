#include "xform.h"
#include "utils.h"

X2UExportXform::X2UExportXform(std::string path, const CRef& ref)
  : X2UExportPrim(path, ref)
{
}

X2UExportXform::~X2UExportXform()
{ 
}

void X2UExportXform::Init(UsdStageRefPtr& stage)
{
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath(_fullname));
  _prim = xform.GetPrim();

  X3DObject obj(_ref);

  // xform attribute
  InitTransformAttribute();

  // visibility attribute
  InitVisibilityAttribute();

}

void X2UExportXform::WriteSample(double t)
{
  UsdTimeCode timeCode(t);
  X3DObject obj(_ref);

  // xform
  WriteTransformSample(t);

  // visibility
  WriteVisibilitySample(t);
}
