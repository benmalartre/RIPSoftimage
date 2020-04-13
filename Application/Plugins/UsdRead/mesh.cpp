#include "mesh.h"
#include "utils.h"
#include <pxr/usd/usdGeom/mesh.h>

U2XMesh::U2XMesh(const pxr::UsdPrim& prim)
  : U2XPrim(prim)
{

}

U2XMesh::~U2XMesh()
{

}

void U2XMesh::Init()
{
  pxr::UsdTimeCode timeCode(pxr::UsdTimeCode::Default());
  GetVisibility(timeCode);
  GetXform(timeCode);

  pxr::UsdGeomMesh mesh(_prim);
  pxr::UsdAttribute pointsAttr = mesh.GetPointsAttr();
  pxr::UsdAttribute faceVertexCountsAttr = mesh.GetFaceVertexCountsAttr();
  pxr::UsdAttribute faceVertexIndicesAttr = mesh.GetFaceVertexIndicesAttr();

  pointsAttr.Get(&_points, timeCode);
  if(!_points.size()) pointsAttr.Get(&_points, pxr::UsdTimeCode::EarliestTime());
  faceVertexCountsAttr.Get(&_counts, timeCode);
  faceVertexIndicesAttr.Get(&_indices, timeCode);

  if (faceVertexIndicesAttr.GetNumTimeSamples() > 0)
    _varyingTopology = true;

  U2XTriangulateMesh(_counts, _indices, _samples);

  _topology.numElements = _samples.size();
  _topology.type = U2XTopology::Type::TRIANGLES;
  _topology.samples = (const int*)&_samples[0][0];
  _vao.SetTopologyPtr(&_topology);

  _vao.SetNeedReallocate(true);
  _vao.SetNumElements(_samples.size());
  _vao.SetHaveChannel(CHANNEL_POSITION);
  _vao.SetHaveChannel(CHANNEL_NORMAL);

  // points
  U2XVertexBuffer* pointsBuffer =
    _vao.CreateBuffer(
      CHANNEL_POSITION,
      _points.size(),
      _samples.size(),
      INTERPOLATION_VERTEX);
  _vao.SetBuffer(CHANNEL_POSITION, pointsBuffer);
  pointsBuffer->SetRawInputDatas((const char*)&_points[0], _points.size());

  // normals
  U2XComputeVertexNormals(_points, _counts, _indices, _samples, _normals);
  U2XVertexBuffer* normalsBuffer =
    _vao.CreateBuffer(
      CHANNEL_NORMAL,
      _normals.size(),
      _samples.size(),
      INTERPOLATION_VERTEX);
  _vao.SetBuffer(CHANNEL_NORMAL, normalsBuffer);
  normalsBuffer->SetRawInputDatas((const char*)&_normals[0], _normals.size());
 
 
}

void U2XMesh::Term()
{

}

void U2XMesh::Update(double t)
{
  pxr::UsdTimeCode timeCode(t);
  GetVisibility(timeCode);
  GetXform(timeCode);

  pxr::UsdGeomMesh mesh(_prim);
  pxr::UsdAttribute pointsAttr = mesh.GetPointsAttr();
  pointsAttr.Get(&_points, timeCode);

  U2XVertexBuffer* pointsBuffer = _vao.GetBuffer(CHANNEL_POSITION);
  if (_points.size() != pointsBuffer->GetNumInputElements())
    pointsBuffer->SetNeedReallocate(true);
  pointsBuffer->SetRawInputDatas((const char*)&_points[0], _points.size());
  size_t hash = pointsBuffer->ComputeHash((const char*)&_points[0]);

  if (hash != pointsBuffer->GetHash())pointsBuffer->SetNeedUpdate(true);
  _vao.UpdateState();
}

void U2XMesh::Prepare()
{
  if(_vao.GetNeedReallocate())_vao.Reallocate();
  if(_vao.GetNeedUpdate())_vao.Populate();
}

void U2XMesh::Draw(GLuint modelUniform)
{
 
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  // model matrix
  glUniformMatrix4fv(
    modelUniform,
    1,
    GL_FALSE,
    GetMatrix()
  );

  _vao.Draw();
}