#include "prim.h"
#include "utils.h"

U2XPrim::U2XPrim(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path)
{
  _prim = stage->GetPrimAtPath(path);
}

U2XPrim::~U2XPrim()
{
}

void U2XPrim::GetBoundingBox(pxr::UsdGeomBBoxCache& bboxCache)
{
  _bbox = bboxCache.ComputeWorldBound(_prim);
}

void U2XPrim::Init()
{
  size_t szp = NUM_TEST_POINTS * 4 * sizeof(float);
  size_t szi = NUM_TEST_INDICES * sizeof(int);

  pxr::VtArray<pxr::GfVec3f> colors(NUM_TEST_POINTS);
  U2XRandomXform<pxr::GfMatrix4d>(_xform);
  U2XRandomColor(colors);
  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);
  _vbos.resize(3);
  glGenBuffers(3, &_vbos[0]);
  glBindBuffer(GL_ARRAY_BUFFER, _vbos[0]);
  glBufferData(GL_ARRAY_BUFFER, szp, &TEST_POINTS[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, _vbos[1]);
  glBufferData(GL_ARRAY_BUFFER, szp, &TEST_NORMALS[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, _vbos[2]);
  glBufferData(GL_ARRAY_BUFFER, szp, &colors[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(0);

  // generate element buffer object
  glGenBuffers(1, &_ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, szi, NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, szi, &TEST_INDICES[0]);

  glBindVertexArray(0);
}

void U2XPrim::Term()
{
  if (_vao)glDeleteVertexArrays(1, &_vao);
  if (_ebo)glDeleteBuffers(1, &_ebo);
  if(_vbos.size())glDeleteBuffers(_vbos.size(), &_vbos[0]);
}

void U2XPrim::Update(double t)
{

}

void U2XPrim::Draw()
{

}


