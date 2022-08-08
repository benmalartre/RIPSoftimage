#pragma once

#include "common.h"
#include "prim.h"
#include "xform.h"
#include "mesh.h"
#include "curve.h"
#include "points.h"
#include "instancer.h"
#include "camera.h"

struct X2UPrototype {
  ULONG     ID;
  SdfPath   path;
};

// Model
class X2UModel {
  public:
    X2UModel(const std::string& folder, const std::string& filename, const CRef& root);
    ~X2UModel();
    virtual void Init(size_t options);
    virtual void Save();

    ULONG GetID() { return _root.GetObjectID(); };
    void SetPath(const std::string& path) { _path = SdfPath(path); };
    SdfPath GetPath() { return _path; };
    void _Recurse(const CRef& ref, const std::string& parentPath);
    void _WriteSample(double t);
    void _Finalize();
    inline std::string _GetRootName() { return _rootName; }

  protected:
    X3DObject                 _root;
    SdfPath                   _path;
    std::string               _rootName;
    X2UXformSharedPtr         _rootXform;
    X2UPrimSharedPtrList      _prims;
    UsdStageRefPtr            _stage;
    std::string               _folder;
    std::string               _filename;
    std::vector<X2UModel>     _models;
    X2UObjectPathMap          _xObjPathMap;
    std::vector<X2UPrototype> _prototypes;
    size_t                    _options;

};

typedef std::vector<X2UModel> X2UModelList;
