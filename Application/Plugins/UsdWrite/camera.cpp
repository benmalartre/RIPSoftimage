#include "camera.h"
#include "utils.h"

X2UCamera::X2UCamera(std::string path, const CRef& ref)
  : X2UPrim(path, ref)
{
}

X2UCamera::~X2UCamera()
{ 
}

void X2UCamera::Init(UsdStageRefPtr& stage)
{
  UsdTimeCode timeCode = UsdTimeCode::Default();
  UsdGeomCamera camera = UsdGeomCamera::Define(stage, SdfPath(_fullname));
  _prim = camera.GetPrim();

  Camera xCamera(_xObj);
  CParameterRefArray parameters = xCamera.GetParameters();

  
  // projection
  _attributes["proj"] =
    X2UAttribute(
      camera.CreateProjectionAttr(), 
      X2U_DATA_TOKEN, 
      X2U_PRECISION_SINGLE, 
      false);

  if (parameters.GetValue("proj") == 1)
    _attributes["proj"].WriteSample(
      (const void*)&UsdGeomTokens->perspective,
      1,
      timeCode);
  else
    _attributes["proj"].WriteSample(
      (const void*)&UsdGeomTokens->orthographic, 
      1, 
      timeCode);

  // focus distance
  _attributes["focusDistance"] =
    X2UAttribute(
      camera.CreateFocusDistanceAttr(),
      X2U_DATA_DOUBLE,
      X2U_PRECISION_SINGLE,
      false);

  double focusDistance = xCamera.GetParameterValue(L"interestdist");
  _attributes["focusDistance"].WriteSample(
    (const void*)&focusDistance,
    1,
    timeCode);
  
  // field of view (horizontal aperture)
  _attributes["focalLength"] =
    X2UAttribute(
      camera.CreateFocalLengthAttr(),
      X2U_DATA_DOUBLE, 
      X2U_PRECISION_SINGLE, 
      false);

  double focalLength = xCamera.GetParameterValue(L"projplanedist");
  _attributes["focalLength"].WriteSample(
    (const void*)&focalLength,
    1, 
    timeCode);

  // vertical aperture
  _attributes["verticalAperture"] =
    X2UAttribute(
      camera.CreateVerticalApertureAttr(),
      X2U_DATA_DOUBLE,
      X2U_PRECISION_SINGLE,
      false);

  double verticalAperture =
    (double)xCamera.GetParameterValue(L"projplaneheight") * 25.4;
  _attributes["verticalAperture"].WriteSample(
    (const void*)&verticalAperture,
    1,
    timeCode);

  // horizontal aperture
  _attributes["horizontalAperture"] =
    X2UAttribute(
      camera.CreateHorizontalApertureAttr(),
      X2U_DATA_DOUBLE,
      X2U_PRECISION_SINGLE,
      false);

  double horizontalAperture =
    (double)xCamera.GetParameterValue(L"projplanewidth") * 25.4;
  _attributes["horizontalAperture"].WriteSample(
    (const void*)&horizontalAperture,
    1,
    timeCode);

  // clipping range
  _attributes["clippingRange"] =
    X2UAttribute(
      camera.CreateClippingRangeAttr(),
      X2U_DATA_VECTOR2,
      X2U_PRECISION_SINGLE,
      false);

  _attributes["clippingRange"].WriteSample(
    (const void*)&GfVec2f(
      xCamera.GetParameterValue(L"near"),
      xCamera.GetParameterValue(L"far")),
    1,
    timeCode);

  // xform attribute
  InitTransformAttribute();

  // visibility attribute
  InitVisibilityAttribute();

}

void X2UCamera::WriteSample(double t)
{
  UsdTimeCode timeCode(t);
  Camera xCamera(_xObj);

  // focus distance
  double focusDistance = xCamera.GetParameterValue(L"interestdist", t);
  _attributes["focusDistance"].WriteSample(
    (const void*)&focusDistance,
    1,
    timeCode);

  // field of view (horizontal aperture)
  double focalLength = xCamera.GetParameterValue(L"projplanedist", t);
  _attributes["focalLength"].WriteSample(
    (const void*)&focalLength,
    1,
    timeCode);

  // vertical aperture
  double verticalAperture =
    (double)xCamera.GetParameterValue(L"projplaneheight", t) * 25.4;
  _attributes["verticalAperture"].WriteSample(
    (const void*)&verticalAperture,
    1,
    timeCode);

  // horizontal aperture
  double horizontalAperture =
    (double)xCamera.GetParameterValue(L"projplanewidth", t) * 25.4;
  _attributes["horizontalAperture"].WriteSample(
    (const void*)&horizontalAperture,
    1,
    timeCode);

  // clipping range
  _attributes["clippingRange"].WriteSample(
    (const void*)&GfVec2f(
      xCamera.GetParameterValue(L"near", t),
      xCamera.GetParameterValue(L"far", t)),
    1,
    timeCode);

  // xform
  WriteTransformSample(t);

  // xform
  WriteVisibilitySample(t);
  
}
