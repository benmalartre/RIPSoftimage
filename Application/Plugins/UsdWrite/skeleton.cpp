#include "skeleton.h"
#include "utils.h"

void X2USkeleton::_RecurseSkeletonTopology(UsdStageRefPtr& stage, CRef& ref,  const pxr::TfToken& parentPath)
{
  X3DObject obj(ref);
  LOG(CString("obj : ")+obj.GetName());
  CString type = obj.GetType();

  if(type == "bone" || type == "root" || /*type == "eff" ||*/ type == "null") {
    pxr::TfToken name(parentPath.GetString() + obj.GetName().GetAsciiString());
    pxr::UsdGeomXform xform = pxr::UsdGeomXform::Define(stage, pxr::SdfPath(pxr::TfToken("/"+name.GetString())));
    _skelTopology.push_back(name);
  }

  CRefArray childrens = obj.GetChildren();
  if(!childrens.GetCount())return;

  for(size_t i = 0; i < childrens.GetCount(); ++i) {
    _RecurseSkeletonTopology(
      stage, childrens[i], 
      pxr::TfToken(parentPath.GetString() + obj.GetName().GetAsciiString() + "/")
    );
  }
    
}

X2USkeleton::X2USkeleton(std::string path, const CRef& ref)
  : X2UPrim(path, ref)
{
}

X2USkeleton::~X2USkeleton()
{
}

void X2USkeleton::Init(UsdStageRefPtr& stage)
{
  pxr::SdfPath path(_fullname);
  pxr::UsdSkelRoot root = pxr::UsdSkelRoot::Define(stage, path);

  _RecurseSkeletonTopology(stage, _xObj, pxr::TfToken(""));

  pxr::UsdSkelSkeleton skeleton = pxr::UsdSkelSkeleton(root.GetPrim());
  skeleton.CreateJointsAttr(pxr::VtValue(_skelTopology));

  LOG("INIT FCKING SKELTON :p");
  /*
  _prim = points.GetPrim();


  Geometry geom = _xPrim.GetGeometry();
  CRefArray iceAttributes = geom.GetICEAttributes();

  // get num points
  _GetNumPoints(geom);

  // points
  InitAttributeFromICE(geom, "PointPosition", "points", SdfValueTypeNames->Vector3fArray);
  WriteSampleFromICE(geom, UsdTimeCode::Default(), "points");

  // velocities
  InitAttributeFromICE(geom, "PointVelocity", "velocities", SdfValueTypeNames->Vector3fArray);
  WriteSampleFromICE(geom, UsdTimeCode::Default(), "velocities");

  // ids
  InitAttributeFromICE(geom, "ID", "ids", SdfValueTypeNames->Int64Array);
  WriteSampleFromICE(geom, UsdTimeCode::Default(), "ids");

  // widths
  InitAttributeFromICE(geom, "Size", "widths", SdfValueTypeNames->FloatArray);
  WriteSampleFromICE(geom, UsdTimeCode::Default(), "widths");

  // colors
  InitColorAttribute(geom);

  // extent attribute
  _ComputeBoundingBox(geom);
  InitExtentAttribute();

  // xform attribute
  InitTransformAttribute();

  // visibility attribute
  InitVisibilityAttribute();
  */
}

void X2USkeleton::WriteSample(double t)
{
  /*
  
  UsdTimeCode timeCode(t);

  Geometry geom = _xPrim.GetGeometry(t, siConstructionModeSecondaryShape);

  bool topoChanged = _GetNumPoints(geom);

  WriteSampleFromICE(geom, timeCode, "points");
  WriteSampleFromICE(geom, timeCode, "velocities");
  WriteSampleFromICE(geom, timeCode, "accelerations");
  WriteSampleFromICE(geom, timeCode, "ids");
  WriteSampleFromICE(geom, timeCode, "widths");
  WriteSampleFromICE(geom, timeCode, "colors");

  // extent attribute
  _ComputeBoundingBox(geom);
  WriteExtentSample(t);

  // xform attribute
  WriteTransformSample(t);

  // visibility attribute
  WriteVisibilitySample(t);
  */
}

