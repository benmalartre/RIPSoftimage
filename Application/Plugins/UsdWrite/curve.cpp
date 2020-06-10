#include "curve.h"
#include "utils.h"


X2UCurve::X2UCurve(std::string path, const CRef& ref, X2UCurveSourceType srcType)
  : X2UPrim(path, ref)
  , _haveNormals(false)
  , _haveWidths(false)
  , _haveColors(false)
  , _haveUVs(false)
  , _srcType(srcType)
{
}

X2UCurve::~X2UCurve()
{ 
}

void X2UCurve::Init(UsdStageRefPtr& stage)
{
  UsdTimeCode timeCode = UsdTimeCode::Default();
  UsdGeomBasisCurves crv = UsdGeomBasisCurves::Define(stage, SdfPath(_fullname));
  _prim = crv.GetPrim();

  // xform attribute
  InitTransformAttribute();

  // visibility attribute
  InitVisibilityAttribute();
  switch (_srcType) {
  case X2U_CURVE_NURBS:
    InitFromNurbsCurve();
    break;
  case X2U_CURVE_STRANDS:
    InitFromStrands();
    break;
  case X2U_CURVE_HAIRS:
    InitFromHairs();
    break;
  }

  // points attribute

  /*
  // points attribute
  {
    CDoubleArray points;
    accessor.GetVertexPositions(points);
    size_t numPoints = accessor.GetVertexCount();

    _bbox = ComputeBoundingBox<double>(&points[0], numPoints);

    _attributes["points"] =
      X2UExportAttribute(
        mesh.CreatePointsAttr(VtValue(), true),
        X2U_DATA_VECTOR3,
        X2U_PRECISION_DOUBLE,
        true);

    // set default value
    _attributes["points"].WriteSample((const void*)&points[0],
      numPoints, UsdTimeCode::Default());
  }
  
  LOG("CURVE WRITE INIT :D");
  UsdAttribute typeAttr = crv.CreateTypeAttr();
  UsdAttribute pointsAttr = crv.CreatePointsAttr();
  UsdAttribute widthsAttr = crv.CreateWidthsAttr();
  UsdAttribute curveVertexCountsAttr = crv.CreateCurveVertexCountsAttr();
  UsdAttribute wrapAttr = crv.CreateWrapAttr();
  UsdAttribute basisAttr = crv.CreateBasisAttr();

  NurbsCurveList curveList = _xPrim.GetGeometry();
  CNurbsCurveRefArray curves = curveList.GetCurves();
  size_t numCurves = curves.GetCount();

  NurbsCurve curve(curves[0]);
  LONG degree;
  curve.GetDegree(degree);

  TfToken curveType;
  // linear or cubic
  if (degree == 1) curveType = UsdGeomTokens->linear;
  else curveType = UsdGeomTokens->cubic;
  typeAttr.Set(curveType, timeCode);

  // points
  CControlPointRefArray points = curveList.GetControlPoints();
  MATH::CVector4Array positions;
  if (points.GetArray(positions) == CStatus::OK)
  {
    VtArray<GfVec3f> _positions(positions.GetCount());
    for (int i = 0; i < _positions.size(); ++i)
    {
      _positions[i] = GfVec3f(
        positions[i].GetX(),
        positions[i].GetY(),
        positions[i].GetZ()
      );
    }
    pointsAttr.Set(_positions, timeCode);
  }
  
  // vertex counts
  VtArray<int> curveVertexCounts(curves.GetCount());
  for (size_t i = 0; i < curves.GetCount(); ++i)
  {
    NurbsCurve curve(curves[i]);
    CKnotArray knots = curve.GetKnots();
    CControlPointRefArray points = curve.GetControlPoints();
    curveVertexCounts[i] = points.GetCount();
    //crv.CreatePointsAttr()
  }
  curveVertexCountsAttr.Set(curveVertexCounts, timeCode);

  // widths
  VtArray<float> widths(1);
  widths[0] = 0.5;
  widthsAttr.Set(widths, timeCode);
  UsdGeomPrimvar widthsPrimVar(widthsAttr);
  widthsPrimVar.SetInterpolation(UsdGeomTokens->constant);
  */

  /*
  NurbsCurve curve = curveList.GetCurves().GetItem(0);
  CControlPointRefArray cpArray = curve.GetControlPoints();
  CKnotArray knotArray = curve.GetKnots();
  CNurbsCurveData curveData;
  cpArray.GetArray(curveData.m_aControlPoints);
  knotArray.GetClosed(curveData.m_bClosed);
  X3DObject circleCopyWithoutKnots;
  root.AddNurbsCurve(curveData, siSINurbs, L"", circleCopyWithoutKnots);

  // xform attribute
  {
    MATH::CTransformation xfo = obj.GetKinematics().GetLocal().GetTransform();
    MATH::CMatrix4 srcMatrix = xfo.GetMatrix4();
    GfMatrix4d dstMatrix;
    X2UConvertMatrix4DoubleToDouble(srcMatrix, dstMatrix);
    _xformOp = curves.AddTransformOp();
    _xformOp.Set<GfMatrix4d>(dstMatrix);
  }

  // points attribute
  {
    CDoubleArray points;
    accessor.GetVertexPositions(points);
    size_t numPoints = accessor.GetVertexCount();

    _bbox = ComputeBoundingBox<double>(&points[0], numPoints);
    VtArray<GfVec3f> vtArray(numPoints);
    X2UCastTuppledData<GfVec3d, GfVec3f>(
      (GfVec3d*)&points[0], vtArray.data(), numPoints, 3, 3);

    _attributes["points"] =
      X2UExportAttribute(
        mesh.CreatePointsAttr(VtValue(vtArray), true),
        X2U_DATA_VECTOR3,
        X2U_PRECISION_DOUBLE,
        true);
  }


  // faceVertexCounts attribute
  {
    CLongArray faceVertexCounts;
    accessor.GetPolygonVerticesCount(faceVertexCounts);
    size_t numFaceVertexCounts = faceVertexCounts.GetCount();

    _attributes["faceVertexCounts"] =
      X2UExportAttribute(
        mesh.CreateFaceVertexCountsAttr(VtValue(vtArray), true),
        X2U_DATA_INT,
        X2U_PRECISION_SINGLE,
        true);
  }


  // faceVertexIndices attribute
  {
    CLongArray faceVertexIndices;
    accessor.GetVertexIndices(faceVertexIndices);
    size_t numFaceVertexIndices = faceVertexIndices.GetCount();

    _attributes["faceVertexIndices"] =
      X2UExportAttribute(
        mesh.CreateFaceVertexIndicesAttr(VtValue(vtArray), true),
        X2U_DATA_INT,
        X2U_PRECISION_SINGLE,
        true);
  }


  // extent attribute
  {
    _attributes["extent"] =
      X2UExportAttribute(
        mesh.CreateExtentAttr(VtValue(), true),
        X2U_DATA_VECTOR3,
        X2U_PRECISION_DOUBLE,
        true);
  }

  // display color
  GetDisplayColor();
  */
}

void X2UCurve::InitFromNurbsCurve()
{

}

void X2UCurve::InitFromStrands()
{
  UsdTimeCode timeCode = UsdTimeCode::Default();
  UsdGeomBasisCurves crv(_prim);
  UsdAttribute typeAttr = crv.CreateTypeAttr();
  UsdAttribute pointsAttr = crv.CreatePointsAttr();
  UsdAttribute widthsAttr = crv.CreateWidthsAttr();
  UsdAttribute curveVertexCountsAttr = crv.CreateCurveVertexCountsAttr();
  UsdAttribute wrapAttr = crv.CreateWrapAttr();
  UsdAttribute basisAttr = crv.CreateBasisAttr();

  ICEAttribute pointPositionAttr = _xPrim.GetICEAttributeFromName("PointPosition");
  size_t numPoints = pointPositionAttr.GetElementCount();
  ICEAttribute pointSizeAttr = _xPrim.GetICEAttributeFromName("Size");
  bool havePointSize = (pointSizeAttr.IsValid() && pointSizeAttr.GetElementCount() == numPoints);
  ICEAttribute pointColorAttr = _xPrim.GetICEAttributeFromName("Color");
  bool havePointColor = (pointColorAttr.IsValid() && pointColorAttr.GetElementCount() == numPoints);

  TfToken curveType = UsdGeomTokens->linear;
  // linear or cubic
  //if (degree == 1) curveType = UsdGeomTokens->linear;
  //else curveType = UsdGeomTokens->cubic;
  typeAttr.Set(curveType, timeCode);

  ICEAttribute strandPositionsAttr = _xPrim.GetICEAttributeFromName("StrandPosition");
  CICEAttributeDataArray2D< MATH::CVector3f > strandPositions;
  strandPositionsAttr.GetDataArray2D(strandPositions);
  size_t numStrands = strandPositions.GetCount();

  // points
  VtArray<GfVec3f> curveVertexPositions;
  VtArray<int> curveVertexCounts(numStrands);
  for (int i = 0; i < strandPositions.GetCount(); ++i) {
    CICEAttributeDataArrayVector3f positions;
    strandPositions.GetSubArray(i, positions);
    curveVertexCounts[i] = positions.GetCount();
    size_t base = curveVertexPositions.size();
    size_t np = positions.GetCount();
    curveVertexPositions.resize(base + np);
    memcpy(&curveVertexPositions[base], &positions[0], np * sizeof(pxr::GfVec3f));
  }

  pointsAttr.Set(curveVertexPositions, timeCode);
  curveVertexCountsAttr.Set(curveVertexCounts, timeCode);

  size_t numCurveVertex = curveVertexPositions.size();

  ICEAttribute strandSizesAttr = _xPrim.GetICEAttributeFromName("StrandSize");
  bool haveStrandSizes = 
    X2UCheckICEAttributeDataArray2DSize<float>(strandSizesAttr, numPoints, numCurveVertex);

  ICEAttribute strandOrientationsAttr = _xPrim.GetICEAttributeFromName("StrandOrientation");
  bool haveStrandOrientations =
    X2UCheckICEAttributeDataArray2DSize<MATH::CRotationf>(strandOrientationsAttr, numPoints, numCurveVertex);

  ICEAttribute strandNormalsAttr = _xPrim.GetICEAttributeFromName("StrandNormal");
  bool haveStrandNormals = 
    X2UCheckICEAttributeDataArray2DSize<MATH::CVector3f>(strandNormalsAttr, numPoints, numCurveVertex);

  ICEAttribute strandUpVectorsAttr = _xPrim.GetICEAttributeFromName("StrandUpVector");
  bool haveStrandUpVectors =
    X2UCheckICEAttributeDataArray2DSize<MATH::CVector3f>(strandUpVectorsAttr, numPoints, numCurveVertex);

  // widths
  if (haveStrandSizes) {
    VtArray<float> curveVertexSizes(curveVertexPositions.size());
    CICEAttributeDataArray2D< float > strandSizes;
    strandSizesAttr.GetDataArray2D(strandSizes);
    size_t baseIdx = 0;
    if (havePointSize) {
      CICEAttributeDataArrayFloat pointSize;
      pointSizeAttr.GetDataArray(pointSize);
      for (int i = 0; i < strandSizes.GetCount(); ++i) {
        CICEAttributeDataArrayFloat sizes;
        strandSizes.GetSubArray(i, sizes);
        size_t np = sizes.GetCount();
        for (size_t j = 0; j < np; ++j) {
          curveVertexSizes[baseIdx + j] = sizes[j] * pointSize[i];
        }
        baseIdx += np;
      }
    }
    else {
      for (int i = 0; i < strandSizes.GetCount(); ++i) {
        CICEAttributeDataArrayFloat sizes;
        strandSizes.GetSubArray(i, sizes);
        size_t np = sizes.GetCount();
        memcpy(&curveVertexSizes[baseIdx], &sizes[0], np * sizeof(float));
        baseIdx += np;
      }
    }
    widthsAttr.Set(curveVertexSizes, timeCode);
    UsdGeomPrimvar widthsPrimVar(widthsAttr);
    widthsPrimVar.SetInterpolation(UsdGeomTokens->vertex);
  }
  else if (havePointSize) {
    VtArray<float> curveSizes(curveVertexCounts.size());
    CICEAttributeDataArrayFloat pointSize;
    pointSizeAttr.GetDataArray(pointSize);
    memcpy(&curveSizes[0], &pointSize[0], curveVertexCounts.size() * sizeof(float));
    widthsAttr.Set(curveSizes, timeCode);
    UsdGeomPrimvar widthsPrimVar(widthsAttr);
    widthsPrimVar.SetInterpolation(UsdGeomTokens->uniform);
  }
  else {
    VtArray<float> widths(1);
    widths[0] = 0.01;
    widthsAttr.Set(widths, timeCode);
    UsdGeomPrimvar widthsPrimVar(widthsAttr);
    widthsPrimVar.SetInterpolation(UsdGeomTokens->constant);
  }
  
}

void X2UCurve::InitFromHairs()
{

}

void X2UCurve::WriteSample(double t)
{
  UsdTimeCode timeCode(t);
  LOG("WRITE SAMPLE FOR CURVE : " + _xObj.GetFullName());
  /*
  PolygonMesh mesh = obj.GetActivePrimitive().GetGeometry(t);
  CGeometryAccessor accessor = mesh.GetGeometryAccessor2
  (
    siConstructionMode::siConstructionModeSecondaryShape,
    siSubdivisionRuleType::siCatmullClark
  );

  // xform
  {
    MATH::CTransformation xfo = obj.GetKinematics().GetLocal().GetTransform();
    MATH::CMatrix4 srcMatrix = xfo.GetMatrix4();
    GfMatrix4d dstMatrix;
    X2UConvertMatrix4DoubleToDouble(srcMatrix, dstMatrix);
    _xformOp.Set<GfMatrix4d>(dstMatrix, timeCode);
  }

  // points
  {
    CDoubleArray points;
    accessor.GetVertexPositions(points);

    size_t numPoints = accessor.GetVertexCount();
    _bbox = ComputeBoundingBox<double>(&points[0], numPoints);
    X2UExportAttribute& item = GetAttribute("points");
    item.WriteSample((const void*)&points[0], numPoints, timeCode);
  }

  // face vertex indices
  {
    CLongArray faceVertexIndices;
    accessor.GetVertexIndices(faceVertexIndices);
    size_t numFaceVertexIndices = faceVertexIndices.GetCount();
    X2UExportAttribute& item = GetAttribute("faceVertexIndices");
    item.WriteSample((const void*)&faceVertexIndices[0], numFaceVertexIndices, timeCode);
  }

  // face vertex counts
  {
    CLongArray faceVertexCounts;
    accessor.GetPolygonVerticesCount(faceVertexCounts);
    size_t numFaceVertexCounts = faceVertexCounts.GetCount();
    X2UExportAttribute& item = GetAttribute("faceVertexCounts");
    item.WriteSample((const void*)&faceVertexCounts[0], numFaceVertexCounts, timeCode);
  }

  // extent 
  {
    X2UExportAttribute& item = GetAttribute("extent");
    item.WriteSample((const void*)&_bbox.GetRange().GetMin()[0], 2, timeCode);
  }

  // displayColor
  {
    X2UExportAttribute& item = GetAttribute("displayColor");
    if (_haveColors)
    {
      if (item.FromICE())
      {
        item.SetSourceAttribute(mesh.GetICEAttributeFromName("Color"));
        item.WriteSample(timeCode);
      }
      else
      {
        ClusterProperty vertexColor = mesh.GetCurrentVertexColor();
        if (vertexColor.IsValid())
        {
          CFloatArray colors;
          vertexColor.GetValues(colors);
          item.WriteSample(&colors[0], colors.GetCount() / 4, timeCode);
        }
      }
    }
    else
    {
      GfVec4f color(_displayColorR, _displayColorG, _displayColorB, 1.f);
      item.WriteSample(&color[0], 1, timeCode);
    }
  }
  */
}

/*
void X2UExportCurve::InitColorAttribute()
{
 
}

void X2UExportCurve::WriteColorSample(double t)
{

}
*/
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