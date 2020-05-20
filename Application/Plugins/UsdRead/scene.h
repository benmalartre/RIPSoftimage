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
  U2XStage* Get(CustomPrimitive& in_prim)
  {
    auto& it = _cache.find(in_prim.GetObjectID());
    if (it != _cache.end())return it->second;
    else return NULL;
  }
  void Set(CustomPrimitive& in_prim, U2XStage* stage)
  {
    _cache[in_prim.GetObjectID()] = stage;
    stage->SetObjectID(in_prim.GetObjectID());
  }

  void CheckCache()
  {
    for (auto& cache : _cache)
    {
      ObjectID objectID = cache.first;
      ProjectItem item = Application().GetObjectFromID(objectID);
      if (!item.IsValid())
      {
        if (cache.second)delete cache.second;
        _cache.erase(objectID);
      }
    }
  }

  void ClearCache()
  {
    for (auto& cache : _cache)
      if (cache.second)delete cache.second;
    _cache.clear();
  }

private:
  typedef ULONG ObjectID;

  std::map<ObjectID, U2XStage*> _cache;
};

extern U2XPrimitiveManager U2X_PRIMITIVES;
extern pxr::UsdStageCache U2X_USDSTAGE_CACHE;