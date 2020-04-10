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
  InitAttributeFromICE(geom, L"PointPosition", "points", SdfValueTypeNames->Vector3fArray);
  WriteSampleFromICE(geom, UsdTimeCode::Default(), "points");

  /*
  // velocities
  InitAttributeFromICE(geom, L"PointVelocity", "velocities", SdfValueTypeNames->Vector3fArray);
  WriteSampleFromICE(geom, UsdTimeCode::Default(), "velocities");

  // accelerations
  InitAttributeFromICE(geom, L"PointAcceleration", "accelerations", SdfValueTypeNames->Vector3fArray);
  WriteSampleFromICE(geom, UsdTimeCode::Default(), "accelerations");
  */
  // ids
  InitAttributeFromICE(geom, L"ID", "ids", SdfValueTypeNames->Int64Array);
  WriteSampleFromICE(geom, UsdTimeCode::Default(), "ids");

  // widths
  InitAttributeFromICE(geom, L"Size", "widths", SdfValueTypeNames->FloatArray);
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
  //WriteSampleFromICE(geom, timeCode, "velocities");
  //WriteSampleFromICE(geom, timeCode, "accelerations");
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
  bool havePointColor = false;
  if (iceAttrIndex>=0)
  {
    UsdGeomPrimvar displayColorPrimvar =
      UsdGeomGprim(_prim).CreateDisplayColorPrimvar(UsdGeomTokens->vertex);

    UsdAttribute displayColorAttr = displayColorPrimvar.GetAttr();

    _attributes["colors"] =
      X2UExportAttribute(
        displayColorAttr,
        iceAttrIndex
      );
    
    // set default value
    if (_numPoints)
    {
      CICEAttributeDataArrayColor4f colors;
      iceAttr.GetDataArray(colors);
      size_t numElements = colors.GetCount();
      _attributes["colors"].WriteSample((const void*)&colors[0], numElements, UsdTimeCode::Default());
    }

    havePointColor = true;
    
  }

  if (!havePointColor)
  {
    Property displayProp;

    if (_xObj.GetPropertyFromName(L"Display", displayProp) == CStatus::OK) {
      VtArray<GfVec3f> dstColors(1);
      dstColors[0] = X2UGetDisplayColorFromShadingNetwork(_xObj);
      UsdAttribute dstColorAttr = UsdGeomMesh(_prim).CreateDisplayColorAttr();

      _attributes["colors"] =
        X2UExportAttribute(
          dstColorAttr,
          X2U_DATA_COLOR4,
          X2U_PRECISION_SINGLE,
          true);

      // set default value
      _attributes["colors"].WriteSample((const void*)dstColors.data(), dstColors.size(), UsdTimeCode::Default());
    }
    havePointColor = true;
  }
 
}

