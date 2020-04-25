#pragma once

#include "common.h"
#include "prim.h"

#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/cylinder.h>
#include <pxr/usd/usdGeom/capsule.h>
#include <pxr/usd/usdGeom/cone.h>



struct X2UInstancerShape {
  XSI::siICEShapeType   type;
  int                   reference;
};

// Mesh 
class X2UInstancer final : public X2UPrim {
public:
  X2UInstancer(std::string path, const CRef& ref);
  ~X2UInstancer();

  void Init(UsdStageRefPtr& stage) override;
  void WriteSample(double t) override;
  void Term(UsdStageRefPtr& stage, const X2UObjectPathMap& objPathMap) override;

  int AddInstanceShape(const XSI::MATH::CShape& shape);
  void WritePrototypesSample(const UsdTimeCode& timeCode);
  void AddPrototypesRelationship(UsdStageRefPtr& stage, const std::map<ULONG, SdfPath>& objPathMap);
  void GetPrototypesObjectIDs(CLongArray& ids);


private:
  bool _GetNumPoints(const Geometry& geom);
  void _ComputeBoundingBox(const Geometry& geom);
  size_t                          _numPoints;
  bool                            _haveColors;
  bool                            _haveNormals;
  bool                            _haveWidths;
  bool                            _haveUVs;
  std::vector<X2UInstancerShape>  _shapes;
};

typedef std::shared_ptr<X2UInstancer> X2UInstancerSharedPtr;