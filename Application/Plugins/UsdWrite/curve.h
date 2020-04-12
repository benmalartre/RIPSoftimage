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
  X2U_CURVE_HAIR
};

// Mesh 
class X2UCurve final : public X2UPrim {
public:
  X2UCurve(std::string path, const CRef& ref);
  ~X2UCurve();

  void Init(UsdStageRefPtr& stage) override;
  void WriteSample(double t) override;

  void InitFromNurbsCurve();
  //void InitFromStrands();
  //void InitFromHair();

private:
  bool                _haveColors;
  bool                _haveNormals;
  bool                _haveWidths;
  bool                _haveUVs;
  X2UCurveSourceType  _srcType;
};

typedef std::shared_ptr<X2UCurve> X2UCurveSharedPtr;