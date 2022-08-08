#pragma once

#include "common.h"
#include "prim.h"

// Mesh 
class X2UMesh final : public X2UPrim {
public:
  X2UMesh(std::string path, const CRef& ref);
  ~X2UMesh();

  void Init(UsdStageRefPtr& stage) override;
  void WriteSample(double t) override;
  void InitColorAttribute();
  void InitNormalsAttribute();
  void InitUVsAttribute();
  void WriteColorSample(const PolygonMesh& mesh, double t);

  bool _GetNodesColors(const PolygonMesh& geom, VtArray<GfVec3f>& ioArray);
  bool _GetNodesNormals(const PolygonMesh& geom, VtArray<GfVec3f>& ioArray);
  bool _GetNodesUVs(const PolygonMesh& geom, VtArray<GfVec2f>& ioArray);

private:
  bool          _haveColors;
  bool          _haveNormals;
  bool          _haveUVs;
};

typedef std::shared_ptr<X2UMesh> X2UMeshSharedPtr;