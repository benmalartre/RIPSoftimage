#include "curve.h"
#include "utils.h"

X2UExportCurve::X2UExportCurve(std::string path, const CRef& ref)
  : X2UExportPrim(path, ref)
  , _haveNormals(false)
  , _haveWidths(false)
  , _haveColors(false)
  , _haveUVs(false)
{
}

X2UExportCurve::~X2UExportCurve()
{ 
}

void X2UExportCurve::Init(UsdStageRefPtr& stage)
{
  UsdGeomBasisCurves crv = UsdGeomBasisCurves::Define(stage, SdfPath(_fullname));
  _prim = crv.GetPrim();

  // xform attribute
  InitTransformAttribute();

  // visibility attribute
  InitVisibilityAttribute();


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
  */

  crv.CreatePointsAttr();
  crv.CreateCurveVertexCountsAttr();
  crv.CreateWrapAttr();

  NurbsCurveList curveList = _xPrim.GetGeometry();// DBL_MAX, siConstructionModeSecondaryShape);
  CNurbsCurveRefArray curves = curveList.GetCurves();
  size_t numCurves = curves.GetCount();

  crv.CreateCurveVertexCountsAttr();
  for (size_t i = 0; i < curves.GetCount(); ++i)
  {
    NurbsCurve curve(curves[i]);
    CKnotArray knots = curve.GetKnots();
    CControlPointRefArray points = curve.GetControlPoints();

    //crv.CreatePointsAttr()
  }
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

void X2UExportCurve::WriteSample(double t)
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