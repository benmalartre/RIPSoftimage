#pragma once

#include "prim.h"

// Xform 
class U2XXform final : public U2XPrim {
public:
  U2XXform(const pxr::UsdPrim& prim, U2XPrim* parent);
  ~U2XXform();

  void Init() override;
  void Term() override;
  void Update(double t, bool reinitialize=false) override;
  void Prepare() override;
  void Draw() override;
};
