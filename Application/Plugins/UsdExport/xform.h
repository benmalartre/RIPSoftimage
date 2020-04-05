#pragma once

#include "common.h"
#include "prim.h"

// Xform 
class X2UExportXform final : public X2UExportPrim {
public:
  X2UExportXform(std::string path, const CRef& ref);
  ~X2UExportXform();

  void Init(UsdStageRefPtr& stage) override;
  void WriteSample(double t) override;
  void InitDisplayColor() override;
};

typedef std::shared_ptr<X2UExportXform> X2UExportXformSharedPtr;