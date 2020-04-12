#pragma once

#include "prim.h"


// Mesh 
class U2XMesh final : public U2XPrim {
public:
  U2XMesh(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path);
  ~U2XMesh();

  void Init() override;
  void Term() override;
  void Update(double t) override;
  void Draw() override;

private:
  pxr::VtArray<pxr::GfVec3f>   _points;
  pxr::VtArray<pxr::GfVec3f>   _normals;
  pxr::VtArray<pxr::GfVec3f>   _colors;
  pxr::VtArray<int>            _counts;
  pxr::VtArray<int>            _indices;
  pxr::VtArray<int>            _samples;
};

typedef std::shared_ptr<U2XMesh> U2XMeshSharedPtr;