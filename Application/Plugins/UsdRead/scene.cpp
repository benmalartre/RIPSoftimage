#include "scene.h"
#include "stage.h"

#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/xform.h>

U2XPrimitiveManager U2X_PRIMITIVES;
pxr::UsdStageCache  U2X_USDSTAGE_CACHE;
U2XScene*           U2X_SCENE;

U2XScene::U2XScene()
{
  _stage = pxr::UsdStage::CreateInMemory("U2XScene");
  _rootPath = pxr::SdfPath(pxr::TfToken("/"));
  LOG("CONTSRUCTOR U2XScene CALLED : " + CString(this));
}

U2XScene::~U2XScene()
{
  LOG("DESTRUCTOR U2XScene CALLED : " + CString(this));
  LOG("DELETE THE WHOLE FUCKIN WORLD :D!");
  _prims.clear();
  _stage = nullptr;
}


pxr::UsdPrim U2XScene::GetRootPrim(ULONG objectId)
{
  return _stage->GetPrimAtPath(pxr::SdfPath("/U2XRoot" + std::to_string(objectId)));
}

void U2XScene::AddStage(U2XStage* stage)
{
  pxr::SdfPath rootPath(pxr::TfToken("/U2XRoot" + std::to_string(stage->GetObjectID())));
  pxr::UsdPrim root = pxr::UsdGeomXform::Define(_stage, rootPath).GetPrim();
  pxr::UsdPrim ref = _stage->OverridePrim(rootPath.AppendChild(pxr::TfToken("ref")));
  ref.GetReferences().AddReference(stage->GetRootLayer()->GetIdentifier());
  _prims.push_back(root);
}

void U2XScene::ReloadStage(U2XStage* stage)
{

}

void U2XScene::Update()
{
  for (const auto& stage : U2X_PRIMITIVES.stages) {
    CustomPrimitive prim = Application().GetObjectFromID(stage.first);
    stage.second->Update(prim);
  }
}