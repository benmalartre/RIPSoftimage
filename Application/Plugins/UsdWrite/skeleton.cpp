#include "skeleton.h"
#include "utils.h"


void _RecurseSkeletonTopology(CRef& ref, std::vector<pxr::TfToken>& topology)
{
  X3DObject obj(ref);
  topology.push_back(pxr::TfToken(obj.GetFullName().GetAsciiString()));
  LOG(obj.GetFullName());
  CRefArray childrens = obj.GetChildren();
  if(!childrens.GetCount())return;

  for(size_t c = 0; c < childrens.GetCount(); ++c)
    _RecurseSkeletonTopology(childrens[c], topology);
}

X2USkeleton::X2USkeleton(std::string path, const CRef& ref)
  : X2UPrim(path, ref)
  , _skelRoot(ref)
{
  /*
  _xObj = X3DObject(ref);
  _xPrim = _xObj.GetActivePrimitive();
  _xID = _xObj.GetObjectID();
  */
  X3DObject root(_skelRoot);
  _skelTopology.clear();

  CRefArray childrens = root.GetChildren();
  for(size_t c = 0; c < childrens.GetCount(); ++c) {
    _RecurseSkeletonTopology(childrens[c], _skelTopology);
  }
}

X2USkeleton::~X2USkeleton()
{
}

void X2USkeleton::Init(UsdStageRefPtr& stage)
{
  // walk 
  /*
  UsdGeomPoints points = UsdGeomPoints::Define(stage, SdfPath(_fullname));
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

