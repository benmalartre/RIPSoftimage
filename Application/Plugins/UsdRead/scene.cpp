#include "scene.h"
#include "stage.h"

#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/xform.h>

//U2XPrimitiveManager U2X_PRIMITIVES;
//pxr::UsdStageCache  U2X_USDSTAGE_CACHE;
U2XScene*           U2X_SCENE;

U2XScene::U2XScene()
{
  LOG("CONSTRUCTOR U2XSCENE CALLED");
  _stage = pxr::UsdStage::CreateInMemory("U2XScene");
  _rootPath = pxr::SdfPath(pxr::TfToken("/"));
}

U2XScene::~U2XScene()
{
  LOG("DESTRUCTOR U2XSCENE CALLED");
  _manager.ClearCache();
  _prims.clear();
  _stage = nullptr;
}


pxr::UsdPrim U2XScene::GetRootPrim(ULONG objectId)
{
  return _stage->GetPrimAtPath(pxr::SdfPath("/U2XRoot" + std::to_string(objectId)));
}

pxr::UsdPrim U2XScene::GetRefPrim(ULONG objectId)
{
  return _stage->GetPrimAtPath(pxr::SdfPath("/U2XRoot" + std::to_string(objectId)+"/ref"));
}

void U2XScene::AddStage(CustomPrimitive& prim, U2XStage* stage)
{
  _manager.Set(prim, stage);
  pxr::SdfPath rootPath(pxr::TfToken("/U2XRoot" + std::to_string(stage->GetObjectID())));
  pxr::UsdPrim root = pxr::UsdGeomXform::Define(_stage, rootPath).GetPrim();
  pxr::UsdPrim ref = _stage->OverridePrim(rootPath.AppendChild(pxr::TfToken("ref")));
  //ref.GetReferences().AddReference(stage->GetRootLayer()->GetIdentifier());
  _prims.push_back(root);
}

void U2XScene::ReloadStage(U2XStage* stage)
{
  pxr::UsdPrim ref = GetRefPrim(stage->GetObjectID());
  ref.GetReferences().ClearReferences();
  ref.GetReferences().AddReference(stage->GetRootLayer()->GetIdentifier());
}

U2XStage* U2XScene::GetStage(CustomPrimitive& prim)
{
  return _manager.Get(prim);
}

U2XStage* U2XScene::GetStage(ULONG objectId)
{
  return _manager.Get(objectId);
}

void U2XScene::SyncStagesCache()
{
  _manager.CheckCache();
}

void U2XScene::Update()
{
  for (const auto& stage : _manager.stages) {
    CustomPrimitive prim = Application().GetObjectFromID(stage.first);
    stage.second->Update(prim);
  }
}