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
  void GetDisplayColor() override;

private:
  bool _haveVertexColor;
  bool _haveAuthoredNormals;

};

typedef std::shared_ptr<X2UExportMesh> X2UExportMeshSharedPtr;