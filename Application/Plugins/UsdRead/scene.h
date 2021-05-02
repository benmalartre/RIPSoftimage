#pragma once

#include "common.h"
#include "stage.h"
#include "prim.h"
#include <map>

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
    LOG("SEARCH ID : " + CString(objectId));
    for (const auto& stage : stages)LOG("IN :" + CString(stage.first));
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
    LOG("SET USD PRIMITIVE IN CACHE : " + CString(objectId));
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
        LOG("DELETE FUCKIN INVALID ITEM!");
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

extern U2XPrimitiveManager U2X_PRIMITIVES;
extern pxr::UsdStageCache U2X_USDSTAGE_CACHE;