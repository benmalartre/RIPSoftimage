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

  X3DObject obj(_ref);
  Geometry geom = obj.GetActivePrimitive().GetGeometry();

  // get num points
  _GetNumPoints(geom);

  // points
  InitAttributeFromICE(geom, "PointPosition", "points", SdfValueTypeNames->Vector3fArray);
  WriteSampleFromICE(geom, UsdTimeCode::Default(), "points", _numPoints);

  // velocities
  InitAttributeFromICE(geom, "PointVelocity", "velocities", SdfValueTypeNames->Vector3fArray);
  WriteSampleFromICE(geom, UsdTimeCode::Default(), "velocities", _numPoints);

  // ids
  InitAttributeFromICE(geom, "ID", "ids", SdfValueTypeNames->Int64Array);
  WriteSampleFromICE(geom, UsdTimeCode::Default(), "ids", _numPoints);

  // widths
  InitAttributeFromICE(geom, "Size", "widths", SdfValueTypeNames->FloatArray);
  WriteSampleFromICE(geom, UsdTimeCode::Default(), "widths", _numPoints);

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
  X3DObject obj(_ref);
  Geometry geom = obj.GetActivePrimitive().GetGeometry(t, siConstructionModeSecondaryShape);

  bool topoChanged = _GetNumPoints(geom);
  
  WriteSampleFromICE(geom, timeCode, "points", _numPoints);
  WriteSampleFromICE(geom, timeCode, "velocities", _numPoints);
  WriteSampleFromICE(geom, timeCode, "ids", _numPoints);
  WriteSampleFromICE(geom, timeCode, "widths", _numPoints);
  WriteSampleFromICE(geom, timeCode, "colors", _numPoints);

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
  ICEAttribute nbPointsAttr = geom.GetICEAttributeFromName("NbPoints");
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
    ICEAttribute pointPositionAttr = geom.GetICEAttributeFromName("PointPosition");
    CICEAttributeDataArrayVector3f pointPositions;
    pointPositionAttr.GetDataArray(pointPositions);

    _bbox = X2UComputeBoundingBox<float>((float*)&pointPositions[0], _numPoints);
  }
  else
    _bbox = GfBBox3d(GfRange3d(GfVec3d(0), GfVec3d(0)));
}


void X2UExportPoints::InitColorAttribute(const Geometry& geom)
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
  /*
  ICEAttribute pointColorAttr = geom.GetICEAttributeFromName(L"Color");
  bool havePointColor = false;
  if (pointColorAttr.IsValid())
  {
    UsdGeomPrimvar displayColorPrimvar =
      UsdGeomGprim(_prim).CreateDisplayColorPrimvar(UsdGeomTokens->vertex);

    UsdAttribute displayColorAttr = displayColorPrimvar.GetAttr();

    _attributes["colors"] =
      X2UExportAttribute(
        displayColorAttr,
        L"Color"
      );
    
    // set default value
    if (_numPoints)
    {
      CICEAttributeDataArrayColor4f colors;
      pointColorAttr.GetDataArray(colors);
      size_t numElements = colors.GetCount();
      _attributes["colors"].WriteSample((const void*)&colors[0], numElements, UsdTimeCode::Default());
    }

    havePointColor = true;
    
  }

  if (!havePointColor)
  {
    Property displayProp;
    X3DObject xsiObj(_ref);
    if (xsiObj.GetPropertyFromName(L"Display", displayProp) == CStatus::OK) {
      VtArray<GfVec3f> dstColors(1);
      dstColors[0] = X2UGetDisplayColorFromShadingNetwork(xsiObj);
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
  */
}

