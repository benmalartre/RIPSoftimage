#pragma once

#include "common.h"
#include "attribute.h"

// Prim type
enum X2UExportPrimType {
  X2U_XFORM,
  X2U_MESH,
  X2U_POINT,
  X2U_CURVE,
  X2U_CAMERA,
  X2U_LIGHT
};

// Prim base class
class X2UExportPrim {
public:
  X2UExportPrim();
  virtual ~X2UExportPrim();

  virtual void Init(UsdStageRefPtr& stage, std::string path, const CRef& ref) = 0;
  virtual void WriteSample(double t )= 0;
  virtual void GetDisplayColor() = 0;

  inline X2UExportPrimType GetType() { return _type; };
  inline X3DObject GetXSI3DObject() { return X3DObject(_ref); };
  inline X2UExportAttribute& GetAttribute(std::string name) {
    return _attributes[name];
  };

 

protected:
  CRef                    _ref;
  UsdPrim                 _prim;
  X2UExportPrimType       _type;
  std::string             _fullname;
  X2UExportAttributeMap   _attributes;
  GfBBox3d                _bbox;
  UsdGeomXformOp          _xformOp;
  float                   _displayColorR;
  float                   _displayColorG;
  float                   _displayColorB;
};

typedef std::shared_ptr<X2UExportPrim> X2UExportPrimSharedPtr;
typedef std::vector<X2UExportPrimSharedPtr> X2UExportPrimSharedPtrList;
//typedef std::vector<X2UExportPrim> X2UExportPrimList;

