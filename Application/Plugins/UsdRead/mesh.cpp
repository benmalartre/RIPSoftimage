#include "mesh.h"
#include "utils.h"
#include <pxr/usd/usdGeom/mesh.h>

U2XMesh::U2XMesh(const pxr::UsdPrim& prim, U2XPrim* parent)
  : U2XPrim(prim, parent)
{

}

U2XMesh::~U2XMesh()
{

}

void U2XMesh::Init()
{
  pxr::UsdTimeCode timeCode(pxr::UsdTimeCode::Default());
  GetVisibility(timeCode, true);
  GetXform(timeCode);

  pxr::UsdGeomMesh mesh(_prim);
  pxr::UsdAttribute pointsAttr = mesh.GetPointsAttr();
  pxr::UsdAttribute faceVertexCountsAttr = mesh.GetFaceVertexCountsAttr();
  pxr::UsdAttribute faceVertexIndicesAttr = mesh.GetFaceVertexIndicesAttr();

  _pointsVarying = false;
  if (pointsAttr.GetNumTimeSamples() > 1)_pointsVarying = true;
  _topoVarying = false;
  if (faceVertexIndicesAttr.GetNumTimeSamples() > 1)_topoVarying = true;

  pointsAttr.Get(&_points, timeCode);
  if(!_points.size()) pointsAttr.Get(&_points, pxr::UsdTimeCode::EarliestTime());
  faceVertexCountsAttr.Get(&_counts, timeCode);
  faceVertexIndicesAttr.Get(&_indices, timeCode);

  U2XTriangulateMesh(_counts, _indices, _samples);

  _topology.numElements = _samples.size();
  _topology.type = U2XTopology::Type::TRIANGLES;
  _topology.samples = (const int*)&_samples[0][0];
  _vao.SetTopologyPtr(&_topology);

  _vao.SetNeedReallocate(true);
  _vao.SetNumElements(_samples.size());
  _vao.SetHaveChannel(CHANNEL_POSITION);
  _vao.SetHaveChannel(CHANNEL_NORMAL);

  size_t hash;
  // points
  U2XVertexBuffer* pointsBuffer =
    _vao.CreateBuffer(
      CHANNEL_POSITION,
      _points.size(),
      _samples.size(),
      INTERPOLATION_VERTEX);
  _vao.SetBuffer(CHANNEL_POSITION, pointsBuffer);
  pointsBuffer->SetRawInputDatas((const char*)&_points[0], _points.size());
  pointsBuffer->ComputeHash((const char*)&_points[0]);

  // normals
  pxr::UsdAttribute normalsAttr = mesh.GetNormalsAttr();

  U2XComputeVertexNormals(_points, _counts, _indices, _samples, _normals);
  U2XVertexBuffer* normalsBuffer =
    _vao.CreateBuffer(
      CHANNEL_NORMAL,
      _normals.size(),
      _samples.size(),
      INTERPOLATION_VERTEX);
  _vao.SetBuffer(CHANNEL_NORMAL, normalsBuffer);
  normalsBuffer->SetRawInputDatas((const char*)&_normals[0], _normals.size());
  normalsBuffer->ComputeHash((const char*)&_normals[0]);

  // colors
  pxr::TfToken colorAttrName("displayColor");
  U2XAttributeType attrType = HasAttribute(colorAttrName);
  
  if (attrType == ATTR_PRIMVAR)
  {
    U2XAttribute colorAttribute = CreateAttribute(colorAttrName, attrType);
    const pxr::UsdAttribute& attr = colorAttribute.Get();

    attr.Get<pxr::VtArray<pxr::GfVec3f>>(&_colors, pxr::UsdTimeCode::Default());
    if (_colors.size())
    {
      _vao.SetHaveChannel(CHANNEL_COLOR);

      U2XVertexBuffer* colorsBuffer =
        _vao.CreateBuffer(
          CHANNEL_COLOR,
          _colors.size(),
          _samples.size(),
          colorAttribute.GetInterpolation());
      _vao.SetBuffer(CHANNEL_COLOR, colorsBuffer);
      colorsBuffer->SetRawInputDatas((const char*)&_colors[0][0], _colors.size());
      colorsBuffer->ComputeHash((const char*)&_colors[0][0]);
    }
    /*
    U2XAttribute colorAttribute = CreateAttribute(colorAttrName, attrType);

    const VtValue& values = colorAttribute.Get(pxr::UsdTimeCode::EarliestTime());
    if (values.IsHolding<pxr::VtArray<pxr::GfVec3f>>())
    {
      const pxr::VtArray<pxr::GfVec3f>& colors = values.UncheckedGet<pxr::VtArray<GfVec3f>>();

      if (colors.size())
      {
        _vao.SetHaveChannel(CHANNEL_COLOR);

        U2XVertexBuffer* colorsBuffer =
          _vao.CreateBuffer(
            CHANNEL_COLOR,
            colors.size(),
            _samples.size(),
            colorAttribute.GetInterpolation());
        _vao.SetBuffer(CHANNEL_COLOR, colorsBuffer);
        colorsBuffer->SetRawInputDatas((const char*)&colors[0][0], colors.size());
        colorsBuffer->ComputeHash((const char*)&colors[0][0]);
      }
      */

  }
}

void U2XMesh::Term()
{

}

void U2XMesh::Update(double t)
{
  
  pxr::UsdTimeCode timeCode(t);
  GetVisibility(timeCode);
  bool pointsPositionUpdated = false;
  bool topoUpdated = false;

  pxr::UsdGeomMesh mesh(_prim);
  size_t hash;
  // topo
  if (_topoVarying)
  {
    LOG("TOPO VARYING FOR : " + CString(_prim.GetName().GetText()));
    pxr::UsdAttribute faceVertexCountsAttr = mesh.GetFaceVertexCountsAttr();
    pxr::UsdAttribute faceVertexIndicesAttr = mesh.GetFaceVertexIndicesAttr();

    faceVertexCountsAttr.Get(&_counts, timeCode);
    faceVertexIndicesAttr.Get(&_indices, timeCode);

    U2XTriangulateMesh(_counts, _indices, _samples);

    _topology.numElements = _samples.size();
    _topology.samples = (const int*)&_samples[0][0];
    _vao.SetTopologyPtr(&_topology);

    _vao.SetNeedReallocate(true);
    _vao.SetNumElements(_samples.size());
    pointsPositionUpdated = true;
    topoUpdated = true;
  }

  // points
  if (_pointsVarying || topoUpdated)
  {
    pxr::UsdAttribute pointsAttr = mesh.GetPointsAttr();
    
    pointsAttr.Get(&_points, timeCode);

    U2XVertexBuffer* pointsBuffer = _vao.GetBuffer(CHANNEL_POSITION);
    if (_points.size() != pointsBuffer->GetNumInputElements())
      pointsBuffer->SetNeedReallocate(true);
    pointsBuffer->SetRawInputDatas((const char*)&_points[0], _points.size());
    hash = pointsBuffer->GetHash();
    if (hash != pointsBuffer->ComputeHash((const char*)&_points[0]))
    {
      pointsBuffer->SetNeedUpdate(true);
      pointsPositionUpdated = true;
    }
    if (topoUpdated)pointsBuffer->SetNumOutputElements(_vao.GetNumElements());

    if (pointsPositionUpdated || topoUpdated)
    {
      // normals
      U2XComputeVertexNormals(_points, _counts, _indices, _samples, _normals);
      U2XVertexBuffer* normalsBuffer = _vao.GetBuffer(CHANNEL_NORMAL);
      if (_normals.size() != normalsBuffer->GetNumInputElements())
        normalsBuffer->SetNeedReallocate(true);
      normalsBuffer->SetRawInputDatas((const char*)&_normals[0], _normals.size());
      hash = normalsBuffer->GetHash();

      if (hash != normalsBuffer->ComputeHash((const char*)&_normals[0]))
      {
        normalsBuffer->SetNeedUpdate(true);
      }
      if (topoUpdated)normalsBuffer->SetNumOutputElements(_vao.GetNumElements());
    }

    if (topoUpdated || _colorsVarying)
    {
      // colors
      U2XComputeVertexColors(_points, _colors);
      U2XVertexBuffer* colorsBuffer = _vao.GetBuffer(CHANNEL_COLOR);
      if (_colors.size() != colorsBuffer->GetNumInputElements())
        colorsBuffer->SetNeedReallocate(true);
      colorsBuffer->SetRawInputDatas((const char*)&_colors[0], _colors.size());
      hash = colorsBuffer->GetHash();

      if (hash != colorsBuffer->ComputeHash((const char*)&_colors[0]))
      {
        colorsBuffer->SetNeedUpdate(true);
      }
      if (topoUpdated)colorsBuffer->SetNumOutputElements(_vao.GetNumElements());
    }
    
   
  }
  
  _vao.UpdateState();
}

void U2XMesh::Prepare()
{
  if(_vao.GetNeedReallocate())_vao.Reallocate();
  if(_vao.GetNeedUpdate())_vao.Populate();
}

void U2XMesh::Draw()
{
  _vao.Draw();
}