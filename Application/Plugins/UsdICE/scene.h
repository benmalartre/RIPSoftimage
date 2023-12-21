#ifndef U2I_SCENE_H
#define U2I_SCENE_H

#include "common.h"
#include "stage.h"
#include "prim.h"
#include <map>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/stage.h>


struct U2ISceneLight
{
  unsigned long   _objectID;
  unsigned short  _lightType;
  pxr::GfVec3f    _position;
  pxr::GfVec3f    _direction;
  float           _intensity;
};

struct U2ISceneState
{
  std::vector<U2ISceneLight> _lights;
};

struct U2IPrimitiveManager
{
  typedef ULONG ObjectID;
  U2IStage* Get(CustomPrimitive& in_prim)
  {
    auto& it = stages.find(in_prim.GetObjectID());
    if (it != stages.end())return it->second;
    else return NULL;
  }

  U2IStage* Get(ObjectID objectId)
  {
    auto& it = stages.find(objectId);
    if (it != stages.end())return it->second;
    else return NULL;
  }

  U2IStage* GetByIndex(size_t idx)
  {
    std::map<ObjectID, U2IStage*>::iterator it(stages.begin());
    std::advance(it, idx);
    if (it != stages.end())return it->second;
    else return NULL;
  }

  void Set(CustomPrimitive& in_prim, U2IStage* stage)
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

  std::map<ObjectID, U2IStage*> stages;
};

class U2IStage;
class U2IScene {
public:
  U2IScene();
  ~U2IScene();

  void Update();
  void AddStage(CustomPrimitive& prim, U2IStage* stage);
  void ReloadStage(U2IStage* stage);
  void SyncStagesCache();
  pxr::UsdStageWeakPtr GetSceneStage() { return _stage; };
  U2IStage* GetStage(CustomPrimitive& prim);
  U2IStage* GetStage(ULONG objectId);
  pxr::UsdPrim GetRootPrim(ULONG objectId);
  pxr::UsdPrim GetRefPrim(ULONG objectId);

private:
  pxr::UsdStageRefPtr       _stage;
  std::vector<pxr::UsdPrim> _prims;
  pxr::SdfPath              _rootPath;
  U2IPrimitiveManager       _manager;
};

extern pxr::UsdStageCache   U2I_USDSTAGE_CACHE;
extern U2IScene*            U2I_SCENE;

#endif // U2I_SCENE_H