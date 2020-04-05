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
  virtual void InitDisplayColor() = 0;

  inline X2UExportAttribute& GetAttribute(std::string name) {
    return _attributes[name];
  };
  inline UsdPrim GetPrim() { return _prim; };

protected:
  CRef                    _ref;
  UsdPrim                 _prim;
  std::string             _fullname;
  X2UExportAttributeMap   _attributes;
  GfBBox3d                _bbox;
  UsdGeomXformOp          _xformOp;
};

typedef std::shared_ptr<X2UExportPrim> X2UExportPrimSharedPtr;
typedef std::vector<X2UExportPrimSharedPtr> X2UExportPrimSharedPtrList;

