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
class X2UScene;
class X2UModel {
  public:
    X2UModel(const std::string& folder, const std::string& filename, 
      const CRef& root, X2UModel* parent=NULL);
    ~X2UModel();
    virtual void Init();
    virtual void Save();

    ULONG GetID() { return _root.GetObjectID(); };
    void SetPath(const std::string& path) { _path = SdfPath(path); };
    SdfPath GetPath() { return _path; };

    X2UScene* GetScene();
    X2UModel* GetParent();

  protected:
    inline std::string        _GetRootName() { return _rootName; }
    void                      _Recurse(const CRef& ref, const std::string& parentPath, size_t options);
    void                      _WriteSample(double t);
    void                      _Finalize();

    X2UModel*                 _parent;
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
    bool                      _selected;

};

typedef std::vector<X2UModel> X2UModelList;
