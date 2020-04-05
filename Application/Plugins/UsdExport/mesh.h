#pragma once

#include "common.h"
#include "prim.h"

// Mesh 
class X2UExportMesh final : public X2UExportPrim {
public:
  X2UExportMesh();
  ~X2UExportMesh();

  void Init(UsdStageRefPtr& stage, std::string path, const CRef& ref) override;
  void WriteSample(double t) override;
  void InitDisplayColor() override;
  void InitNormals();
  void InitUVs();

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