#pragma once

#include "common.h"
#include "prim.h"

// Points 
class X2USkeleton final : public X2UPrim {
public:

  X2USkeleton(std::string path, const CRef& ref);
  ~X2USkeleton();

  void Init(UsdStageRefPtr& stage) override;
  void WriteSample(double t) override;

private:
  CRef                        _skelRoot;
  std::vector<pxr::TfToken>   _skelTopology;
};

typedef std::shared_ptr<X2USkeleton> X2USkeletonSharedPtr;