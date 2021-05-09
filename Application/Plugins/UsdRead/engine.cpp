#include <pxr/imaging/garch/glApi.h>
#include <pxr/imaging/glf/contextCaps.h>
#include <pxr/imaging/glf/glContext.h>
#include <pxr/base/tf/envSetting.h>
#include <pxr/base/tf/diagnostic.h>
#include <pxr/base/tf/callContext.h>
#include "utils.h"
#include "engine.h"

#include <iostream>


PXR_NAMESPACE_USING_DIRECTIVE

TF_DEFINE_ENV_SETTING(U2X_ENGINE_DEBUG_SCENE_DELEGATE_ID, "/",
  "usd to softimage scene delegate id");

pxr::SdfPath const&
_GetUsdImagingDelegateId()
{
  static pxr::SdfPath const delegateId =
    pxr::SdfPath(pxr::TfGetEnvSetting(U2X_ENGINE_DEBUG_SCENE_DELEGATE_ID));

  return delegateId;
}


U2XEngine::U2XEngine(const pxr::HdDriver& driver)
  : U2XEngine(pxr::SdfPath::AbsoluteRootPath(),
    {},
    {},
    _GetUsdImagingDelegateId(),
    driver
  )
{
}

U2XEngine::U2XEngine(
  const pxr::SdfPath& rootPath,
  const pxr::SdfPathVector& excludedPaths,
  const pxr::SdfPathVector& invisedPaths,
  const pxr::SdfPath& sceneDelegateID,
  const pxr::HdDriver& driver)
  : pxr::UsdImagingGLEngine(
    rootPath,
    excludedPaths,
    invisedPaths,
    sceneDelegateID,
    driver)
{
  LOG("U2XENGINE CONSTRUCTOR CALLED!!!");
  
  //_InitGL();

  if (IsHydraEnabled()) {

    // _renderIndex, _taskController, and _sceneDelegate are initialized
    // by the plugin system.
    if (!SetRendererPlugin(_GetDefaultRendererPluginId())) {
      TF_CODING_ERROR("No renderer plugins found! "
        "Check before creation.");
    }
  }
  else {
    TF_CODING_ERROR("Hydra is NOT supported! ");
  }

}

U2XEngine::~U2XEngine() = default;

/*
U2XEngine::Fuck()
{
  pxr::GfVec2i renderResolution(GetWidth(), GetHeight());
  _drawTarget = pxr::GlfDrawTarget::New(renderResolution);
  _drawTarget->Bind();

  _drawTarget->AddAttachment("color",
    GL_RGBA, GL_FLOAT, GL_RGBA);
  _drawTarget->AddAttachment("depth",
    GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F);
  _drawTarget->Unbind();
}
*/

