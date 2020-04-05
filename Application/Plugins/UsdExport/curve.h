#pragma once

#include "common.h"
#include "prim.h"

// Mesh 
class X2UExportCurve final : public X2UExportPrim {
public:
  X2UExportCurve(std::string path, const CRef& ref);
  ~X2UExportCurve();

  void Init(UsdStageRefPtr& stage) override;
  void WriteSample(double t) override;
  void InitDisplayColor() override;

private:
  bool _haveColors;
  bool _haveNormals;
  bool _haveWidths;
  bool _haveUVs;
};

typedef std::shared_ptr<X2UExportCurve> X2UExportCurveSharedPtr;