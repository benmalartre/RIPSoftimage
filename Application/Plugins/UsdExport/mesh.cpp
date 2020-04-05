#include "mesh.h"
#include "utils.h"

X2UExportMesh::X2UExportMesh(std::string path, const CRef& ref)
  : X2UExportPrim(path, ref)
  , _haveNormals(false)
  , _haveColors(false)
  , _haveUVs(false)
{
}

X2UExportMesh::~X2UExportMesh()
{
}

void X2UExportMesh::Init(UsdStageRefPtr& stage)
{
  UsdGeomMesh mesh = UsdGeomMesh::Define(stage, SdfPath(_fullname));
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
    GfMatrix4d dstMatrix;
    GetLocalTransformAtTime(obj, dstMatrix);
    _xformOp = mesh.AddTransformOp();
    UsdAttribute xfoAttr = _xformOp.GetAttr();

    _attributes["xform"] =
      X2UExportAttribute(
        xfoAttr,
        X2U_DATA_MATRIX4,
        X2U_PRECISION_DOUBLE,
        false);

    // set default value
    _attributes["xform"].WriteSample((const void*)&dstMatrix, 1, UsdTimeCode::Default());
  }

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
  

  // faceVertexCounts attribute
  {
    CLongArray faceVertexCounts;
    accessor.GetPolygonVerticesCount(faceVertexCounts);
    size_t numFaceVertexCounts = faceVertexCounts.GetCount();
    
    _attributes["faceVertexCounts"] =
      X2UExportAttribute(
        mesh.CreateFaceVertexCountsAttr(VtValue(), true),
        X2U_DATA_INT,
        X2U_PRECISION_SINGLE,
        true);

    // set default value
    _attributes["faceVertexCounts"].WriteSample((const void*)&faceVertexCounts[0], 
      numFaceVertexCounts, UsdTimeCode::Default());
  }
  

  // faceVertexIndices attribute
  {
    CLongArray faceVertexIndices;
    accessor.GetVertexIndices(faceVertexIndices);
    size_t numFaceVertexIndices = faceVertexIndices.GetCount();
  
    _attributes["faceVertexIndices"] =
      X2UExportAttribute(
        mesh.CreateFaceVertexIndicesAttr(VtValue(), true),
        X2U_DATA_INT,
        X2U_PRECISION_SINGLE,
        true);

    // set default value
    _attributes["faceVertexIndices"].WriteSample((const void*)&faceVertexIndices[0], 
      numFaceVertexIndices, UsdTimeCode::Default());
  }
  

  // extent attribute
  {
    _attributes["extent"] =
      X2UExportAttribute(
        mesh.CreateExtentAttr(VtValue(), true),
        X2U_DATA_VECTOR3,
        X2U_PRECISION_DOUBLE,
        true);

    // set default value
    _attributes["extent"].WriteSample((const void*)&_bbox.GetRange().GetMin(), 
      2, UsdTimeCode::Default());
  }
  
  // display color
  InitDisplayColor();

  // normals
  InitNormals();

  // uvs
  InitUVs();
 
}

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
    GfMatrix4d dstMatrix;
    GetLocalTransformAtTime(obj, dstMatrix, t);
    X2UExportAttribute& item = GetAttribute("xform");
    item.WriteSample((const void*)&dstMatrix, 1, timeCode);
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
        VtArray<GfVec3f> colors;
        if(_GetNodesColors(mesh, colors))
        {
          item.WriteSample((void*)colors.data(), colors.size(), timeCode);
        }
      }
    }
   
    else
    {
      /*
      GfVec4f color(_displayColorR, _displayColorG, _displayColorB, 1.f);
      item.WriteSample(&color[0], 1, timeCode);
      */
    }
  }

  // normals
  {
    X2UExportAttribute& item = GetAttribute("normals");
    VtArray<GfVec3f> normals;
    if (_GetNodesNormals(mesh, normals))
    {
      item.WriteSample((void*)normals.data(), normals.size(), timeCode);
    }
  }

  // uvs
  {
    if (_haveUVs)
    {
      X2UExportAttribute& item = GetAttribute("uvs");
      VtArray<GfVec2f> uvs;
      if (_GetNodesUVs(mesh, uvs))
      {
        item.WriteSample((void*)uvs.data(), uvs.size(), timeCode);
      }
    }
  }
}

void X2UExportMesh::InitDisplayColor()
{
  _haveColors = false;
  X3DObject xsiObj(_ref);
  Primitive xsiPrim = xsiObj.GetActivePrimitive();

  // check for color from ICE Attribute
  ICEAttribute srcColorAttr = xsiPrim.GetICEAttributeFromName(L"Color");
  if (srcColorAttr.IsDefined())
  {
    // create attribute
    UsdGeomPrimvar dstColorPrimvar = UsdGeomMesh(_prim).CreateDisplayColorPrimvar(UsdGeomTokens->vertex);
    UsdAttribute dstColorAttr = dstColorPrimvar.GetAttr();
    
    // set default value
    CICEAttributeDataArrayColor4f colors;
    srcColorAttr.GetDataArray(colors);
    size_t numElements = colors.GetCount();
    
    // store in attribute map
    _attributes["displayColor"] =
      X2UExportAttribute(
        dstColorAttr,
        srcColorAttr
      );

    // set default value
    _attributes["displayColor"].WriteSample((const void*)&colors[0], colors.GetCount(), UsdTimeCode::Default());
  
    _haveColors = true;
  }
  
  // check for vertex color map
  if (!_haveColors)
  {
    VtArray<GfVec3f> dstColors;
    PolygonMesh xsiMesh = xsiPrim.GetGeometry();
    if(_GetNodesColors(xsiMesh, dstColors))
    {
      // create attribute
      UsdGeomPrimvar dstColorPrimvar = 
        UsdGeomMesh(_prim).CreateDisplayColorPrimvar(UsdGeomTokens->faceVarying, dstColors.size());
      UsdAttribute dstColorAttr = dstColorPrimvar.GetAttr();
      
      // store in attribute map
      _attributes["displayColor"] =
        X2UExportAttribute(
          dstColorAttr,
          X2U_DATA_VECTOR3,
          X2U_PRECISION_SINGLE,
          true);

      // set default value
      _attributes["displayColor"].WriteSample((const void*)dstColors.data(), dstColors.size(), UsdTimeCode::Default());

      _haveColors = true;
    }
  }
  
  // fallback to diffuse color
  if (!_haveColors)
  {
    Property displayProp;
    if (xsiObj.GetPropertyFromName(L"Display", displayProp) == CStatus::OK) {
      VtArray<GfVec3f> dstColors(1);
      dstColors[0] = GetDisplayColorFromShadingNetwork(xsiObj);
      UsdAttribute dstColorAttr = UsdGeomMesh(_prim).CreateDisplayColorAttr();

      _attributes["displayColor"] =
        X2UExportAttribute(
          dstColorAttr,
          X2U_DATA_COLOR4,
          X2U_PRECISION_SINGLE,
          true);

      // set default value
      _attributes["displayColor"].WriteSample((const void*)dstColors.data(), dstColors.size(), UsdTimeCode::Default());
    }
  }
}

void X2UExportMesh::InitNormals()
{
  _haveNormals = false;
  X3DObject xsiObj(_ref);
  Primitive xsiPrim = xsiObj.GetActivePrimitive();

  VtArray<GfVec3f> dstNormals;
  PolygonMesh xsiMesh = xsiPrim.GetGeometry();
  if (_GetNodesNormals(xsiMesh, dstNormals))
  {
    // create attribute
    UsdAttribute dstNormalsAttr = 
      UsdGeomMesh(_prim).CreateNormalsAttr(VtValue(), true);

    // store in attribute map
    _attributes["normals"] =
      X2UExportAttribute(
        dstNormalsAttr,
        X2U_DATA_VECTOR3,
        X2U_PRECISION_SINGLE,
        true);

    // set default value
    _attributes["normals"].WriteSample((const void*)dstNormals.data(), dstNormals.size(), UsdTimeCode::Default());

    _haveNormals = true;
  }
}

void X2UExportMesh::InitUVs()
{
  _haveUVs = false;
  X3DObject xsiObj(_ref);
  Primitive xsiPrim = xsiObj.GetActivePrimitive();

  VtArray<GfVec2f> dstUVs;
  PolygonMesh xsiMesh = xsiPrim.GetGeometry();
  if (_GetNodesUVs(xsiMesh, dstUVs))
  {
    // create uv primvar
    UsdGeomPrimvar dstUVPrimvar = 
      UsdGeomMesh(_prim).CreatePrimvar(TfToken("st"), SdfValueTypeNames->TexCoord2dArray, UsdGeomTokens->faceVarying);

    // get attribute
    UsdAttribute dstUVsAttr = dstUVPrimvar.GetAttr();

    // store in attribute map
    _attributes["uvs"] =
      X2UExportAttribute(
        dstUVsAttr,
        X2U_DATA_VECTOR2,
        X2U_PRECISION_SINGLE,
        true);

    // set default value
    _attributes["uvs"].WriteSample((const void*)dstUVs.data(), dstUVs.size(), UsdTimeCode::Default());

    _haveUVs = true;
  }
}


bool X2UExportMesh::_GetNodesColors(const PolygonMesh& mesh, VtArray<GfVec3f>& ioArray)
{
  // geometry accessor
  CGeometryAccessor accessor = mesh.GetGeometryAccessor();
  CRefArray vertexColors = accessor.GetVertexColors();
  if (vertexColors.GetCount())
  {
    ClusterProperty vertexColor = vertexColors[0];
    CLongArray nodeIndices;
    accessor.GetNodeIndices(nodeIndices);

    // get the values
    CFloatArray colorValues;
    vertexColor.GetValues(colorValues);
    size_t numElements = colorValues.GetCount() / 4;
    ioArray.resize(numElements);
    for (size_t i = 0; i < numElements; i++)
    {
      size_t nodeIndex = nodeIndices[i];
      ioArray[i] = GfVec3f(
        colorValues[nodeIndex * 4],
        colorValues[nodeIndex * 4 + 1],
        colorValues[nodeIndex * 4 + 2]
      );
    }
    return true;
  }
  return false;
}

bool X2UExportMesh::_GetNodesNormals(const PolygonMesh& mesh, VtArray<GfVec3f>& ioArray)
{
  // geometry accessor
  CGeometryAccessor accessor = mesh.GetGeometryAccessor();

  // get the values
  CFloatArray normals;
  accessor.GetNodeNormals(normals);

  CLongArray nodeIndices;
  accessor.GetNodeIndices(nodeIndices);

  size_t numElements = normals.GetCount() / 3;
  ioArray.resize(numElements);
  memcpy((void*)ioArray.data(), &normals[0], numElements * 3 * sizeof(float));
  
  return true;

}

bool X2UExportMesh::_GetNodesUVs(const PolygonMesh& mesh, VtArray<GfVec2f>& ioArray)
{
  // geometry accessor
  CGeometryAccessor accessor = mesh.GetGeometryAccessor();
  CRefArray uvs = accessor.GetUVs();
  if (uvs.GetCount())
  {
    ClusterProperty uv = uvs[0];
    CLongArray nodeIndices;
    accessor.GetNodeIndices(nodeIndices);

    // get the values
    CFloatArray uvValues;
    uv.GetValues(uvValues);
    size_t numElements = uvValues.GetCount() / 3;
    ioArray.resize(numElements);
    for (size_t i = 0; i < numElements; i++)
    {
      size_t nodeIndex = nodeIndices[i];
      ioArray[i] = GfVec2f(
        uvValues[nodeIndex * 3],
        uvValues[nodeIndex * 3 + 1]
      );
    }
    return true;
  }
  return false;
}