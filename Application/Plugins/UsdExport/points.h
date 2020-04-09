#pragma once

#include "common.h"
#include "prim.h"

// Points 
class X2UExportPoints final : public X2UExportPrim {
public:
  X2UExportPoints(std::string path, const CRef& ref);
  ~X2UExportPoints();

  void Init(UsdStageRefPtr& stage) override;
  void WriteSample(double t) override;
  void InitColorAttribute(const Geometry& geom);

private:
  
  //void _WriteColorSample(const Geometry& geom, double t);
  bool _GetNumPoints(const Geometry& geom);
  void _ComputeBoundingBox(const Geometry& geom);
  size_t  _numPoints;
  std::map<std::string, ICEAttribute> _iceattributes;

  bool _haveIDs;
  bool _haveNormals;
  bool _haveColors;
  bool _haveWidths;
  bool _haveVelocities;

};

typedef std::shared_ptr<X2UExportPoints> X2UExportPointSharedPtr;