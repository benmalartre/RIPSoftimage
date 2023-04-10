#pragma once

#include "common.h"
#include "prim.h"
#include <xsi_camera.h>

// Mesh 
class X2UCamera final : public X2UPrim {
public:
  X2UCamera(X2UModel* model, std::string path, const CRef& ref);
  ~X2UCamera();

  void Init(UsdStageRefPtr& stage) override;
  void WriteSample(double t) override;

};

typedef std::shared_ptr<X2UCamera> X2UCameraSharedPtr;