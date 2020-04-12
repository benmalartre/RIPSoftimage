#pragma once

#include "common.h"
#include "prim.h"
#include "xform.h"
#include "mesh.h"
#include "curve.h"
#include "points.h"

// Model
class X2UModel {
  public:
    X2UModel(const std::string& folder, const std::string& filename, const CRef& root);
    ~X2UModel();
    virtual void Init();
    virtual void Save();

    void _Recurse(const CRef& ref, const std::string& parentPath);
    void _WriteSample(double t);
    inline std::string _GetRootName() { return _rootName; }

  protected:
    X3DObject                       _root;
    std::string                     _rootName;
    X2UXformSharedPtr         _rootXform;
    X2UPrimSharedPtrList      _prims;
    UsdStageRefPtr                  _stage;
    std::string                     _folder;
    std::string                     _filename;
    std::vector<X2UModel>     _models;
};

typedef std::vector<X2UModel> X2UModelList;
