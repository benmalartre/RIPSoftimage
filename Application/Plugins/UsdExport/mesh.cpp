#include "mesh.h"
#include "utils.h"

X2UExportMesh::X2UExportMesh()
  : _haveAuthoredNormals(false)
  , _haveVertexColor(false)
{
}

X2UExportMesh::~X2UExportMesh()
{
}

void X2UExportMesh::Init(UsdStageRefPtr& stage, std::string path, const CRef& ref)
{
  UsdGeomMesh mesh = UsdGeomMesh::Define(stage, SdfPath(path));
  _type = X2U_MESH;
  _ref = ref;
  _fullname = path;
  _prim = mesh.GetPrim();

  X3DObject obj(_ref);
  PolygonMesh xsiMesh = obj.GetActivePrimitive().GetGeometry();
  CGeometryAccessor accessor = xsiMesh.GetGeometryAccessor2
  (
    siConstructionMode::siConstructionModeModeling,
    siSubdivisionRuleType::siCatmullClark
  );

  // xform attribute
  {
    MATH::CTransformation xfo = obj.GetKinematics().GetLocal().GetTransform();
    MATH::CMatrix4 srcMatrix = xfo.GetMatrix4();
    GfMatrix4d dstMatrix;
    X2UConvertMatrix4DoubleToDouble(srcMatrix, dstMatrix);
    _xformOp = mesh.AddTransformOp();
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
    
    VtArray<int> vtArray(numFaceVertexCounts);
    X2UCopyData<LONG, int>(
      (LONG*)&faceVertexCounts[0], vtArray.data(), numFaceVertexCounts);
    
    _attributes["faceVertexCounts"] =
      X2UExportAttribute(
        mesh.CreateFaceVertexCountsAttr(VtValue(vtArray), true),
        X2U_DATA_LONG,
        X2U_PRECISION_SINGLE,
        true);
  }
  

  // faceVertexIndices attribute
  {
    CLongArray faceVertexIndices;
    accessor.GetVertexIndices(faceVertexIndices);
    size_t numFaceVertexIndices = faceVertexIndices.GetCount();
    
    VtArray<int> vtArray(numFaceVertexIndices);
    X2UCopyData<LONG, int>(
      (LONG*)&faceVertexIndices[0], vtArray.data(), numFaceVertexIndices);
  
    _attributes["faceVertexIndices"] =
      X2UExportAttribute(
        mesh.CreateFaceVertexIndicesAttr(VtValue(vtArray), true),
        X2U_DATA_LONG,
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
 
}

//ClusterProperty GetCurrentVertexColor( void ) const;

void X2UExportMesh::WriteSample(double t)
{
  UsdTimeCode timeCode(t);
  X3DObject obj(_ref);
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
    if (item.FromICE())
    {
      item.SetSourceAttribute(mesh.GetICEAttributeFromName("Color"));
      item.WriteSample(timeCode);
    }
    else
    {
      if (_haveVertexColor)
      {
        ClusterProperty vertexColor = mesh.GetCurrentVertexColor();
        if (vertexColor.IsValid())
        {
          CFloatArray colors;
          vertexColor.GetValues(colors);
          item.WriteSample(&colors[0], colors.GetCount() / 4, timeCode);
        }
      }
      else
      {
        GfVec4f color(_displayColorR, _displayColorG, _displayColorB, 1.f);
        item.WriteSample(&color[0], 1, timeCode);
      }
    }
  }
}

void X2UExportMesh::GetDisplayColor()
{
  _haveVertexColor = false;
  X3DObject obj(_ref);
  // check for color from ICE Attribute
  ICEAttribute srcColorAttr = obj.GetActivePrimitive().GetICEAttributeFromName(L"Color");
  if (srcColorAttr.IsDefined())
  {
    // create attribute
    UsdGeomPrimvar dstColorPrimvar = UsdGeomMesh(_prim).CreateDisplayColorPrimvar(UsdGeomTokens->vertex);
    UsdAttribute dstColorAttr = dstColorPrimvar.GetAttr();
    
    // set default value
    CICEAttributeDataArrayColor4f colors;
    srcColorAttr.GetDataArray(colors);
    size_t numElements = colors.GetCount();
    VtArray<GfVec3f> vtArray(numElements);
    X2UCastTuppledData<GfVec4f, GfVec3f>((GfVec4f*)&colors[0], vtArray.data(), numElements, 4, 3);
    dstColorAttr.Set(VtValue(vtArray));
    
    // store in attribute map
    _attributes["displayColor"] =
      X2UExportAttribute(
        dstColorAttr,
        srcColorAttr
      );
  
    _haveVertexColor = true;
  }
  
  // check for vertex color map
  if (!_haveVertexColor)
  {
    PolygonMesh mesh = X3DObject(_ref).GetActivePrimitive().GetGeometry();
    ClusterProperty vertexColor = mesh.GetCurrentVertexColor();
    if (vertexColor.IsValid())
    {
      size_t numElements = vertexColor.GetElements().GetCount() / 4;
      
      // create attribute
      UsdGeomPrimvar dstColorPrimvar = 
        UsdGeomMesh(_prim).CreateDisplayColorPrimvar(UsdGeomTokens->faceVarying, numElements);
      UsdAttribute dstColorAttr = dstColorPrimvar.GetAttr();
      
      // set default value
      CFloatArray colors;
      vertexColor.GetValues(colors);
      VtArray<GfVec3f> vtArray(numElements);
      X2UCastTuppledData<GfVec4f, GfVec3f>((GfVec4f*)&colors[0], vtArray.data(), numElements, 4, 3);
      dstColorAttr.Set(VtValue(vtArray));
      
      // store in attribute map
      _attributes["displayColor"] =
        X2UExportAttribute(
          dstColorAttr,
          X2U_DATA_COLOR4,
          X2U_PRECISION_SINGLE,
          true);

      _haveVertexColor = true;
    }
  }

  // fallback to wireframe color
  if (!_haveVertexColor)
  {
    Property displayProp;
    if (obj.GetPropertyFromName(L"Display", displayProp) == CStatus::OK) {
      GfVec3f displayColor = GetDisplayColorFromShadingNetwork(obj);

      _displayColorR = displayColor[0];
      _displayColorG = displayColor[1];
      _displayColorB = displayColor[2];

      UsdAttribute dstColorAttr = 
        UsdGeomMesh(_prim).CreateDisplayColorAttr();

      _attributes["displayColor"] =
        X2UExportAttribute(
          dstColorAttr,
          X2U_DATA_COLOR4,
          X2U_PRECISION_SINGLE,
          false);
    }
  }
  
}