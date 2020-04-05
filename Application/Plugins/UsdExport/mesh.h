#pragma once

#include "common.h"
#include "prim.h"

// Mesh 
class X2UExportMesh final : public X2UExportPrim {
public:
  X2UExportMesh(std::string path, const CRef& ref);
  ~X2UExportMesh();

  void Init(UsdStageRefPtr& stage) override;
  void WriteSample(double t) override;
  void InitDisplayColorAttribute() override;
  void InitNormalsAttribute();
  void InitUVsAttribute();

  bool _GetNodesColors(const PolygonMesh& geom, VtArray<GfVec3f>& ioArray);
  bool _GetNodesNormals(const PolygonMesh& geom, VtArray<GfVec3f>& ioArray);
  bool _GetNodesUVs(const PolygonMesh& geom, VtArray<GfVec2f>& ioArray);

private:
  bool          _haveColors;
  bool          _haveNormals;
  bool          _haveUVs;
  VtArray<int>  _polygonNodeMapping;
};

typedef std::shared_ptr<X2UExportMesh> X2UExportMeshSharedPtr;