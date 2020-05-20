#pragma once

#include "prim.h"

// Mesh 
class U2XMesh final : public U2XPrim {
public:
  U2XMesh(const pxr::UsdPrim& prim, U2XPrim* parent);
  ~U2XMesh();

  void Init() override;
  void Term() override;
  void Update(double t, bool reinitialize=false) override;
  void Prepare() override;
  void Draw() override;

private:
  pxr::VtArray<pxr::GfVec3f>   _points;
  pxr::VtArray<pxr::GfVec3f>   _normals;
  pxr::VtArray<pxr::GfVec3f>   _colors;
  pxr::VtArray<int>            _counts;
  pxr::VtArray<int>            _indices;
  pxr::VtArray<pxr::GfVec3i>   _samples;
  U2XTopology                  _topology;
  bool                         _haveNormals;
  bool                         _haveColors;
  bool                         _haveUVs;
};
