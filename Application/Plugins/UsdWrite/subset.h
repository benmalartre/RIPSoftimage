#pragma once

#include "common.h"
#include "prim.h"

// Mesh 
class X2USubset final : public X2UPrim {
public:
  X2USubset(std::string path, const CRef& ref);
  ~X2USubset();

  void Init(UsdStageRefPtr& stage) override;
  void WriteSample(double t) override;

private:
  TfToken _elementType;
  TfToken _familyName;
};

typedef std::shared_ptr<X2USubset> X2USubsetSharedPtr;