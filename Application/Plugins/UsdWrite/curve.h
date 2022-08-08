#pragma once

#include "common.h"
#include "prim.h"
#include <xsi_knot.h>
#include <xsi_controlpoint.h>
#include <xsi_nurbscurve.h>
#include <xsi_nurbscurvelist.h>

enum X2UCurveSourceType {
  X2U_CURVE_NURBS,
  X2U_CURVE_STRANDS,
  X2U_CURVE_HAIRS
};

// Mesh 
class X2UCurve final : public X2UPrim {
public:
  X2UCurve(std::string path, const CRef& ref, X2UCurveSourceType srcType);
  ~X2UCurve();

  void Init(UsdStageRefPtr& stage, size_t options) override;
  void WriteSample(double t) override;

  void InitFromNurbsCurve();
  void InitFromStrands();
  void InitFromHairs();

private:
  bool                _haveColors;
  bool                _haveNormals;
  bool                _haveWidths;
  bool                _haveUVs;
  X2UCurveSourceType  _srcType;
};

typedef std::shared_ptr<X2UCurve> X2UCurveSharedPtr;