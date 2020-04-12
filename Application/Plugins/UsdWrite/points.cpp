#include "points.h"
#include "utils.h"

X2UExportPoints::X2UExportPoints(std::string path, const CRef& ref)
  : X2UExportPrim(path, ref)
  , _numPoints(0)
{
}

X2UExportPoints::~X2UExportPoints()
{
}

void X2UExportPoints::Init(UsdStageRefPtr& stage)
{
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

}

void X2UExportPoints::WriteSample(double t)
{
  
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

}

bool X2UExportPoints::_GetNumPoints(const Geometry& geom)
{
  ICEAttribute nbPointsAttr = geom.GetICEAttributeFromName(L"NbPoints");

  CICEAttributeDataArrayLong nbPoints;
  nbPointsAttr.GetDataArray(nbPoints);
  bool topoChanged = false;
  if (nbPoints[0] != _numPoints)topoChanged = true;
  _numPoints = nbPoints[0];
  return topoChanged;
}

void X2UExportPoints::_ComputeBoundingBox(const Geometry& geom)
{
  if (_numPoints)
  {

    ICEAttribute pointPositionAttr = geom.GetICEAttributeFromName(L"PointPosition");
    CICEAttributeDataArrayVector3f pointPositions;
    pointPositionAttr.GetDataArray(pointPositions);

    _bbox = X2UComputeBoundingBox<float>(&pointPositions[0][0], _numPoints);
  }
  else
    _bbox = GfBBox3d(GfRange3d(GfVec3d(0), GfVec3d(0)));
}


void X2UExportPoints::InitColorAttribute(const Geometry& geom)
{


  CRefArray attributes = geom.GetICEAttributes();
  int iceAttrIndex;
  ICEAttribute iceAttr = X2UGetICEAttributeFromArray(attributes, L"Color", iceAttrIndex);

  if (iceAttrIndex >= 0)
  {
    ICEAttribute pointColorAttr = geom.GetICEAttributeFromName(L"Color");
    UsdGeomPrimvar displayColorPrimvar =
      UsdGeomGprim(_prim).CreateDisplayColorPrimvar(UsdGeomTokens->vertex);

    UsdAttribute displayColorAttr = displayColorPrimvar.GetAttr();

    _attributes["colors"] =
      X2UExportAttribute(
        displayColorAttr,
        L"Color"
      );

    // set default value
    if (_numPoints && pointColorAttr.IsDefined())
    {
      CICEAttributeDataArrayColor4f colors;
      pointColorAttr.GetDataArray(colors);
      size_t numElements = colors.GetCount();
      _attributes["colors"].WriteSample((const void*)&colors[0], numElements, UsdTimeCode::Default());
    }
    _haveColors = true;
  }

}

