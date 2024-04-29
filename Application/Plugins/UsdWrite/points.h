#pragma once

#include "common.h"
#include "prim.h"

// Points 
class X2UPoints final : public X2UPrim {
public:
  X2UPoints(std::string path, const CRef& ref);
  ~X2UPoints();

  void Init(UsdStageRefPtr& stage) override;
  void WriteSample(double t) override;
  void InitColorAttribute(const Geometry& geom);

private:
  bool _GetNumPoints(const Geometry& geom);
  void _ComputeBoundingBox(const Geometry& geom);
  size_t  _numPoints;

  bool _haveIDs;
  bool _haveNormals;
  bool _haveColors;
  bool _haveWidths;
  bool _haveVelocities;

};

typedef std::shared_ptr<X2UPoints> X2UPointsSharedPtr;