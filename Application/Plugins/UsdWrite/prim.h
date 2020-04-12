#pragma once

#include "common.h"
#include "attribute.h"

// Prim base class
class X2UExportPrim {
public:
  X2UExportPrim(std::string path, const CRef& ref);
  virtual ~X2UExportPrim();

  virtual void Init(UsdStageRefPtr& stage) = 0;
  virtual void WriteSample(double t )= 0;

  inline X2UExportAttribute& GetAttribute(std::string name) {
    return _attributes[name];
  };
  inline UsdPrim GetPrim() { return _prim; };

  void InitExtentAttribute();
  void InitTransformAttribute();
  void InitVisibilityAttribute();

  void WriteExtentSample(double t);
  void WriteTransformSample(double t);
  void WriteVisibilitySample(double t);

  bool InitAttributeFromICE(
    const Geometry& geom,
    const CString& iceAttrName,
    const CString& usdAttrName,
    SdfValueTypeName usdDataType
  );

  void WriteSampleFromICE(const Geometry& geom, UsdTimeCode t, const std::string& attrName);

protected:
  X3DObject               _xObj;
  Primitive               _xPrim;
  UsdPrim                 _prim;
  std::string             _fullname;
  X2UExportAttributeMap   _attributes;
  GfBBox3d                _bbox;
  UsdGeomXformOp          _xformOp;
};

typedef std::shared_ptr<X2UExportPrim> X2UExportPrimSharedPtr;
typedef std::vector<X2UExportPrimSharedPtr> X2UExportPrimSharedPtrList;

