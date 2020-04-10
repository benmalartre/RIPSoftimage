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

<<<<<<< HEAD

  Geometry geom = _xPrim.GetGeometry();
  CRefArray iceAttributes = geom.GetICEAttributes();
=======
<<<<<<< HEAD
  Geometry geom = _xPrim.GetGeometry();

  CRefArray iceAttributes = geom.GetICEAttributes();
=======
  X3DObject obj(_ref);
  Geometry geom = obj.GetActivePrimitive().GetGeometry();
>>>>>>> 33340c508fdb6d0031fe4ee59c86f29fc1234688
>>>>>>> 48678e15845f92630eb30401ac68da8c9b23c42b

  // get num points
  _GetNumPoints(geom);

  // points
<<<<<<< HEAD
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
=======
<<<<<<< HEAD
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
=======
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
>>>>>>> 33340c508fdb6d0031fe4ee59c86f29fc1234688
>>>>>>> 48678e15845f92630eb30401ac68da8c9b23c42b

  // colors
  InitColorAttribute(geom);

  // extent attribute
  _ComputeBoundingBox(geom);
  InitExtentAttribute();

  // xform attribute
  InitTransformAttribute();

  // visibility attribute
  InitVisibilityAttribute();

<<<<<<< HEAD
=======
<<<<<<< HEAD

=======
>>>>>>> 33340c508fdb6d0031fe4ee59c86f29fc1234688
>>>>>>> 48678e15845f92630eb30401ac68da8c9b23c42b
}

void X2UExportPoints::WriteSample(double t)
{
  
  UsdTimeCode timeCode(t);
<<<<<<< HEAD

=======
<<<<<<< HEAD
>>>>>>> 48678e15845f92630eb30401ac68da8c9b23c42b
  Geometry geom = _xPrim.GetGeometry(t, siConstructionModeSecondaryShape);

  bool topoChanged = _GetNumPoints(geom);

  WriteSampleFromICE(geom, timeCode, "points");
<<<<<<< HEAD
  WriteSampleFromICE(geom, timeCode, "velocities");
  WriteSampleFromICE(geom, timeCode, "accelerations");
  WriteSampleFromICE(geom, timeCode, "ids");
  WriteSampleFromICE(geom, timeCode, "widths");
  WriteSampleFromICE(geom, timeCode, "colors");
=======
  //WriteSampleFromICE(geom, timeCode, "velocities");
  //WriteSampleFromICE(geom, timeCode, "accelerations");
  WriteSampleFromICE(geom, timeCode, "ids");
  WriteSampleFromICE(geom, timeCode, "widths");
  WriteSampleFromICE(geom, timeCode, "colors");
=======
  X3DObject obj(_ref);
  Geometry geom = obj.GetActivePrimitive().GetGeometry(t, siConstructionModeSecondaryShape);

  bool topoChanged = _GetNumPoints(geom);
  
  WriteSampleFromICE(geom, timeCode, "points", _numPoints);
  WriteSampleFromICE(geom, timeCode, "velocities", _numPoints);
  WriteSampleFromICE(geom, timeCode, "ids", _numPoints);
  WriteSampleFromICE(geom, timeCode, "widths", _numPoints);
  WriteSampleFromICE(geom, timeCode, "colors", _numPoints);
>>>>>>> 33340c508fdb6d0031fe4ee59c86f29fc1234688
>>>>>>> 48678e15845f92630eb30401ac68da8c9b23c42b

  // extent attribute
  _ComputeBoundingBox(geom);
  WriteExtentSample(t);

  // xform attribute
  WriteTransformSample(t);

  // visibility attribute
  WriteVisibilitySample(t);
<<<<<<< HEAD
=======
<<<<<<< HEAD

=======
  
>>>>>>> 33340c508fdb6d0031fe4ee59c86f29fc1234688
>>>>>>> 48678e15845f92630eb30401ac68da8c9b23c42b
}

bool X2UExportPoints::_GetNumPoints(const Geometry& geom)
{
<<<<<<< HEAD
  ICEAttribute nbPointsAttr = geom.GetICEAttributeFromName(L"NbPoints");
=======
<<<<<<< HEAD
  ICEAttribute nbPointsAttr = geom.GetICEAttributeFromName(L"NbPoints");
=======
  ICEAttribute nbPointsAttr = geom.GetICEAttributeFromName("NbPoints");
>>>>>>> 33340c508fdb6d0031fe4ee59c86f29fc1234688
>>>>>>> 48678e15845f92630eb30401ac68da8c9b23c42b
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
<<<<<<< HEAD
=======
<<<<<<< HEAD
>>>>>>> 48678e15845f92630eb30401ac68da8c9b23c42b
    ICEAttribute pointPositionAttr = geom.GetICEAttributeFromName(L"PointPosition");
    CICEAttributeDataArrayVector3f pointPositions;
    pointPositionAttr.GetDataArray(pointPositions);

    _bbox = X2UComputeBoundingBox<float>(&pointPositions[0][0], _numPoints);
<<<<<<< HEAD
=======
=======
    ICEAttribute pointPositionAttr = geom.GetICEAttributeFromName("PointPosition");
    CICEAttributeDataArrayVector3f pointPositions;
    pointPositionAttr.GetDataArray(pointPositions);

    _bbox = X2UComputeBoundingBox<float>((float*)&pointPositions[0], _numPoints);
>>>>>>> 33340c508fdb6d0031fe4ee59c86f29fc1234688
>>>>>>> 48678e15845f92630eb30401ac68da8c9b23c42b
  }
  else
    _bbox = GfBBox3d(GfRange3d(GfVec3d(0), GfVec3d(0)));
}


void X2UExportPoints::InitColorAttribute(const Geometry& geom)
{
<<<<<<< HEAD
=======
<<<<<<< HEAD
>>>>>>> 48678e15845f92630eb30401ac68da8c9b23c42b

  CRefArray attributes = geom.GetICEAttributes();
  int iceAttrIndex;
  ICEAttribute iceAttr = X2UGetICEAttributeFromArray(attributes, L"Color", iceAttrIndex);
  bool havePointColor = false;
  if (iceAttrIndex>=0)
<<<<<<< HEAD
=======
=======
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
>>>>>>> 33340c508fdb6d0031fe4ee59c86f29fc1234688
>>>>>>> 48678e15845f92630eb30401ac68da8c9b23c42b
  {
    UsdGeomPrimvar displayColorPrimvar =
      UsdGeomGprim(_prim).CreateDisplayColorPrimvar(UsdGeomTokens->vertex);

    UsdAttribute displayColorAttr = displayColorPrimvar.GetAttr();

    _attributes["colors"] =
      X2UExportAttribute(
        displayColorAttr,
<<<<<<< HEAD
        iceAttrIndex

=======
<<<<<<< HEAD
        iceAttrIndex
=======
        L"Color"
>>>>>>> 33340c508fdb6d0031fe4ee59c86f29fc1234688
>>>>>>> 48678e15845f92630eb30401ac68da8c9b23c42b
      );
    
    // set default value
    if (_numPoints)
    {
      CICEAttributeDataArrayColor4f colors;
<<<<<<< HEAD
      iceAttr.GetDataArray(colors);
=======
<<<<<<< HEAD
      iceAttr.GetDataArray(colors);
=======
      pointColorAttr.GetDataArray(colors);
>>>>>>> 33340c508fdb6d0031fe4ee59c86f29fc1234688
>>>>>>> 48678e15845f92630eb30401ac68da8c9b23c42b
      size_t numElements = colors.GetCount();
      _attributes["colors"].WriteSample((const void*)&colors[0], numElements, UsdTimeCode::Default());
    }

    havePointColor = true;
    
  }

  if (!havePointColor)
  {
    Property displayProp;
<<<<<<< HEAD
=======
<<<<<<< HEAD
>>>>>>> 48678e15845f92630eb30401ac68da8c9b23c42b

    if (_xObj.GetPropertyFromName(L"Display", displayProp) == CStatus::OK) {
      VtArray<GfVec3f> dstColors(1);
      dstColors[0] = X2UGetDisplayColorFromShadingNetwork(_xObj);
<<<<<<< HEAD

=======
=======
    X3DObject xsiObj(_ref);
    if (xsiObj.GetPropertyFromName(L"Display", displayProp) == CStatus::OK) {
      VtArray<GfVec3f> dstColors(1);
      dstColors[0] = X2UGetDisplayColorFromShadingNetwork(xsiObj);
>>>>>>> 33340c508fdb6d0031fe4ee59c86f29fc1234688
>>>>>>> 48678e15845f92630eb30401ac68da8c9b23c42b
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
<<<<<<< HEAD
=======
<<<<<<< HEAD
 
=======
  */
>>>>>>> 33340c508fdb6d0031fe4ee59c86f29fc1234688
>>>>>>> 48678e15845f92630eb30401ac68da8c9b23c42b
}

