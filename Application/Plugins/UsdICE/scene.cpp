#include "scene.h"
#include "stage.h"

#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/xform.h>

pxr::UsdStageCache  U2I_USDSTAGE_CACHE;
U2IScene*           U2I_SCENE;
ULONG               U2I_SCENE_ID = 0;

U2IScene::U2IScene()
{
  _stage = pxr::UsdStage::CreateInMemory("U2IScene"+std::to_string(U2I_SCENE_ID));
  _rootPath = pxr::SdfPath(pxr::TfToken("/"));
  U2I_SCENE_ID++;
}

U2IScene::~U2IScene()
{
  _manager.ClearCache();
  _prims.clear();
}


pxr::UsdPrim U2IScene::GetRootPrim(ULONG objectId)
{
  return _stage->GetPrimAtPath(pxr::SdfPath("/U2IRoot" + std::to_string(objectId)));
}

pxr::UsdPrim U2IScene::GetRefPrim(ULONG objectId)
{
  return _stage->GetPrimAtPath(pxr::SdfPath("/U2IRoot" + std::to_string(objectId)+"/ref"));
}

void U2IScene::AddStage(CustomPrimitive& prim, U2IStage* stage)
{
  _manager.Set(prim, stage);
  pxr::SdfPath rootPath(pxr::TfToken("/U2IRoot" + std::to_string(stage->GetObjectID())));
  pxr::UsdPrim root = pxr::UsdGeomXform::Define(_stage, rootPath).GetPrim();
  pxr::UsdPrim ref = _stage->OverridePrim(rootPath.AppendChild(pxr::TfToken("ref")));
  _prims.push_back(root);
}

void U2IScene::ReloadStage(U2IStage* stage)
{
  pxr::UsdPrim ref = GetRefPrim(stage->GetObjectID());
  ref.GetReferences().ClearReferences();
  ref.GetReferences().AddReference(stage->GetRootLayer()->GetIdentifier());
}

U2IStage* U2IScene::GetStage(CustomPrimitive& prim)
{
  return _manager.Get(prim);
}

U2IStage* U2IScene::GetStage(ULONG objectId)
{
  return _manager.Get(objectId);
}

void U2IScene::SyncStagesCache()
{
  _manager.CheckCache();
}

void U2IScene::Update()
{
  for (const auto& stage : _manager.stages) {
    CustomPrimitive prim = Application().GetObjectFromID(stage.first);
    stage.second->Update(prim);
  }
}