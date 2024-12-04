#pragma once

#include "common.h"
#include "attribute.h"

typedef std::map<ULONG, SdfPath> X2UObjectPathMap;

struct X2UGeomSubsetDesc {
  CRef      cluster;
  SdfPath   path;
};

typedef std::vector<X2UGeomSubsetDesc> X2UGeomSubsetDescs;

// Prim base class
class X2UPrim {
public:
  X2UPrim(std::string path, const CRef& ref);
  virtual ~X2UPrim();

  virtual void Init(UsdStageRefPtr& stage) = 0;
  virtual void WriteSample(double t )= 0;
  virtual void Term(UsdStageRefPtr& stage, const X2UObjectPathMap& objPathMap) {};

  inline X2UAttribute& GetAttribute(std::string name) {
    return _attributes[name];
  };
  inline ULONG GetID() { return _xID; };
  inline UsdPrim GetPrim() { return _prim; };
  inline SdfPath GetPath() { return _prim.GetPath(); };

  void BuildGeomSubsetsDescs(const CStringArray& names);
  const X2UGeomSubsetDescs& GetSubsetDescs(){return _subsets;};

  void InitExtentAttribute();
  void InitTransformAttribute();
  void InitVisibilityAttribute();
  void InitExtraAttributes();

  void WriteExtentSample(double t);
  void WriteTransformSample(double t);
  void WriteVisibilitySample(double t);
  void WriteExtraAttributes(double t);

  X2UAttribute* InitAttributeFromICE(
    const Geometry& geom,
    const CString& iceAttrName,
    const CString& usdAttrName,
    SdfValueTypeName usdDataType
  );

  void WriteSampleFromICE(const Geometry& geom, UsdTimeCode t, const std::string& attrName);

protected:
  X2UPrim *               _parent;
  CRef                    _ref;
  X3DObject               _xObj;
  ULONG                   _xID;
  Primitive               _xPrim;
  UsdPrim                 _prim;
  std::string             _fullname;
  X2UAttributeMap         _attributes;
  X2UAttributePtrList     _extraAttributes;
  X2UGeomSubsetDescs      _subsets;
  GfBBox3d                _bbox;
  UsdGeomXformOp          _xformOp;
};

typedef std::shared_ptr<X2UPrim> X2UPrimSharedPtr;
typedef std::vector<X2UPrimSharedPtr> X2UPrimSharedPtrList;

