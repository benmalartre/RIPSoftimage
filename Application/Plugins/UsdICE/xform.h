#ifndef U2I_XFORM_H
#define U2I_XFORM_H

#include "prim.h"
#include "node.h"

// Xform 
class U2IXform final : public U2IPrim {
public:
  U2IXform(const pxr::UsdPrim& prim, U2IPrim* parent);
  ~U2IXform();

  void Init() override;
  void Term() override;
  void Update(double t, bool forceUpdate) override;
  void Prepare() override;
  void Draw() override;
};

#endif