#pragma once

#include "common.h"
#include "attribute.h"

typedef std::map<ULONG, SdfPath> X2UObjectPathMap;

// Prim base class
class X2UPrim {
public:
  X2UPrim(std::string path, const CRef& ref);
  virtual ~X2UPrim();

  virtual void Init(UsdStageRefPtr& stage, size_t options=0) = 0;
  virtual void WriteSample(double t )= 0;
  virtual void Term(UsdStageRefPtr& stage, const X2UObjectPathMap& objPathMap) {};

  inline X2UAttribute& GetAttribute(std::string name) {
    return _attributes[name];
  };
  inline ULONG GetID() { return _xID; };
  inline UsdPrim GetPrim() { return _prim; };
  inline SdfPath GetPath() { return _prim.GetPath(); };

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
  X2UPrim *               _parent;
  X3DObject               _xObj;
  ULONG                   _xID;
  Primitive               _xPrim;
  UsdPrim                 _prim;
  std::string             _fullname;
  X2UAttributeMap         _attributes;
  GfBBox3d                _bbox;
  UsdGeomXformOp          _xformOp;
};

typedef std::shared_ptr<X2UPrim> X2UPrimSharedPtr;
typedef std::vector<X2UPrimSharedPtr> X2UPrimSharedPtrList;

