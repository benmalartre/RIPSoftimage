#pragma once

#include "common.h"
#include "prim.h"

// Xform 
class X2UXform final : public X2UPrim {
public:
  X2UXform(std::string path, const CRef& ref);
  ~X2UXform();

  void Init(UsdStageRefPtr& stage) override;
  void WriteSample(double t) override;
};

typedef std::shared_ptr<X2UXform> X2UXformSharedPtr;