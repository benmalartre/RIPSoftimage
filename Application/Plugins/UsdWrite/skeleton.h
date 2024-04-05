#pragma once

#include "common.h"
#include "prim.h"

// Points 
class X2USkeleton final : public X2UPrim {
public:
  struct _Child {
    CRef      _child;
    CRefArray _children;
  };
  X2USkeleton(std::string path, const CRef& ref);
  ~X2USkeleton();

  void Init(UsdStageRefPtr& stage) override;
  void WriteSample(double t) override;

private:
  CRef                    _skelRoot;
  std::vector<_Child>     _children;
};

typedef std::shared_ptr<X2USkeleton> X2USkeletonSharedPtr;