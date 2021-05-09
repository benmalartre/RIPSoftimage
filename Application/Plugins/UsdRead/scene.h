#pragma once

#include "common.h"
#include "stage.h"
#include "prim.h"
#include <map>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/stage.h>


struct U2XSceneLight
{
  unsigned long   _objectID;
  unsigned short  _lightType;
  pxr::GfVec3f    _position;
  pxr::GfVec3f    _direction;
  float           _intensity;
};

struct U2XSceneState
{
  std::vector<U2XSceneLight> _lights;
};

struct U2XPrimitiveManager
{
  typedef ULONG ObjectID;
  U2XStage* Get(CustomPrimitive& in_prim)
  {
    auto& it = stages.find(in_prim.GetObjectID());
    if (it != stages.end())return it->second;
    else return NULL;
  }

  U2XStage* Get(ObjectID objectId)
  {
    auto& it = stages.find(objectId);
    if (it != stages.end())return it->second;
    else return NULL;
  }

  U2XStage* GetByIndex(size_t idx)
  {
    std::map<ObjectID, U2XStage*>::iterator it(stages.begin());
    std::advance(it, idx);
    if (it != stages.end())return it->second;
    else return NULL;
  }

  void Set(CustomPrimitive& in_prim, U2XStage* stage)
  {
    ULONG objectId = in_prim.GetObjectID();
    stages[objectId] = stage;
    stage->SetObjectID(objectId);
  }

  void CheckCache()
  {
    for (auto& stage : stages)
    {
      ObjectID objectId = stage.first;
      ProjectItem item = Application().GetObjectFromID(objectId);
      if (!item.IsValid())
      {
        if (stage.second)delete stage.second;
        stages.erase(objectId);
      }
    }
  }

  void ClearCache()
  {
    for (auto& stage : stages)
      if (stage.second)delete stage.second;
    stages.clear();
  }

  size_t NumStages() { return stages.size(); };

  std::map<ObjectID, U2XStage*> stages;
};

class U2XStage;
class U2XScene {
public:
  U2XScene();
  ~U2XScene();

  void Update();
  void AddStage(CustomPrimitive& prim, U2XStage* stage);
  void ReloadStage(U2XStage* stage);
  void SyncStagesCache();
  pxr::UsdStageRefPtr GetSceneStage() { return _stage; };
  U2XStage* GetStage(CustomPrimitive& prim);
  U2XStage* GetStage(ULONG objectId);
  pxr::UsdPrim GetRootPrim(ULONG objectId);
  pxr::UsdPrim GetRefPrim(ULONG objectId);

private:
  pxr::UsdStageRefPtr       _stage;
  std::vector<pxr::UsdPrim> _prims;
  pxr::SdfPath              _rootPath;
  U2XPrimitiveManager       _manager;
};

//extern pxr::UsdStageCache   U2X_USDSTAGE_CACHE;
extern U2XScene*            U2X_SCENE;
