#pragma once

#include "common.h"
#include "prim.h"
#include "mesh.h"

// Scene
class X2UExportScene {
  public:
    void Init(const std::string& folder, const std::string& filename, const CRef& root);
    void TimeInfos();
    void Process();
    void WriteSample(double t);
    void Save();

    void Recurse(const CRef& ref, const std::string& parentPath);

  private:
    X3DObject                       _root;
    X2UExportTimeInfos              _timeInfos;
    X2UExportPrimSharedPtrList      _prims;
    UsdStageRefPtr                  _stage;
    std::string                     _folder;
    std::string                     _filename;
    std::vector<X2UExportScene>     _models;
};

typedef std::vector<X2UExportScene> X2UExportSceneList;
