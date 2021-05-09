#include <assert.h>
#include <xsi_decl.h>
#include <xsi_status.h>
#include <xsi_application.h>
#include <xsi_selection.h>
#include <xsi_graphicsequencer.h>
#include <xsi_graphicsequencercontext.h>
#include "utils.h"
#include "callback.h"
#include "engine.h"
#include "scene.h"

#include <GL/gl.h>

U2XEngine* HYDRA_ENGINE = NULL;

extern U2XPrimitiveManager U2X_PRIMITIVES;


static void _InitializeHydraEngine() {
  if (!HYDRA_ENGINE) {
    pxr::SdfPathVector excludedPaths;
    HYDRA_ENGINE = new U2XEngine(pxr::SdfPath("/"), excludedPaths);
    
    
    pxr::GlfSimpleMaterial material;
    pxr::GlfSimpleLight light;
    light.SetAmbient(pxr::GfVec4f(0.25, 0.25, 0.25, 1.0));
    light.SetPosition(pxr::GfVec4f(24, 32, 8, 1));
    pxr::GlfSimpleLightVector lights;
    lights.push_back(light);

    material.SetAmbient(pxr::GfVec4f(0.2, 0.2, 0.2, 1.0));
    
    HYDRA_ENGINE->SetLightingState(lights,
      material,
      pxr::GfVec4f(0.5, 0.5, 0.5, 1.0));
    
  }
}

static pxr::GfMatrix4d _GetViewMatrix(const Camera& camera)
{
  MATH::CMatrix4 viewMatrix = camera.GetKinematics().GetGlobal().GetTransform().GetMatrix4();
  viewMatrix.InvertInPlace();
  return *(pxr::GfMatrix4d*)&viewMatrix;
}

static pxr::GfMatrix4d _GetProjectionMatrix(const Camera& camera)
{
  bool orthographic = bool(camera.GetParameterValue(L"proj") == 0);
  double fov = camera.GetParameterValue(L"fov");
  double aspect = camera.GetParameterValue(L"aspect");
  double znear = camera.GetParameterValue(L"near");
  double zfar = camera.GetParameterValue(L"far");
  if (!orthographic) {
    double m_invf = 1.0 / std::tan(pxr::GfDegreesToRadians(fov) * 0.5f);
    pxr::GfMatrix4d projectionMatrix(1.0);

    projectionMatrix[0][0] = m_invf;
    projectionMatrix[1][1] = m_invf * aspect;
    projectionMatrix[2][2] = (zfar + znear) / (znear - zfar);
    projectionMatrix[3][2] = (2 * zfar * znear) / (znear - zfar);
    projectionMatrix[2][3] = -1;
    projectionMatrix[3][3] = 0;

    return projectionMatrix;
  }
  else {
    pxr::GfMatrix4d projectionMatrix(1.0);
    return projectionMatrix;
  }
}

void UsdHydraDisplayCallback_Init( XSI::CRef sequencerContext, LPVOID *userData )
{
  XSI::GraphicSequencerContext graphicSequencerContext = sequencerContext;
  assert ( graphicSequencerContext.IsValid() );
  XSI::CGraphicSequencer sequencer = graphicSequencerContext.GetGraphicSequencer ();
  sequencer.RegisterDisplayCallback(
    L"UsdHydraDisplayCallback", 
    0, 
    XSI::siPostBeginFrame, 
    XSI::siAll, 
    XSI::CString()
  );

  _InitializeHydraEngine();

  //HYDRA_STAGE = pxr::UsdStage::Open("C:/Users/graph/Documents/bmal/src/USD_ASSETS/Kitchen_set/Kitchen_set.usd");
  //HYDRA_STAGE = pxr::UsdStage::Open("C:\\Users\\graph\\Documents\\bmal\\src\\USD_ASSETS\\Attic_NVIDIA\\Attic_NVIDIA.usd");
  //HYDRA_STAGE = pxr::UsdStage::Open("C:\\Users\\graph\\Documents\\bmal\\src\\USD_ASSETS\\Attic_NVIDIA\\Props\\tv.usd");
}

void UsdHydraDisplayCallback_Execute( XSI::CRef sequencerContext, LPVOID *userData )
{
  //
  // Call OpenGL to clear the frame buffer
  //

  XSI::GraphicSequencerContext graphicSequencerContext = sequencerContext;
  assert(graphicSequencerContext.IsValid());
  XSI::CGraphicSequencer sequencer = graphicSequencerContext.GetGraphicSequencer();

  UINT x, y, width, height;
  sequencer.GetViewportSize(x, y, width, height);
  double aspectRatio = (double)width / (double)height;

  CRef cameraRef = sequencer.GetCamera();
  Camera camera(cameraRef);

  HYDRA_ENGINE->SetCameraState(
    _GetViewMatrix(camera),
    _GetProjectionMatrix(camera)
  );

  HYDRA_ENGINE->SetRenderViewport(
    pxr::GfVec4f(x, y, width, height)
  );

  double currentTime = CTime().GetTime();
  pxr::UsdImagingGLRenderParams renderParams;
  renderParams.frame = pxr::UsdTimeCode(currentTime);
  renderParams.complexity = 1.0f;
  renderParams.drawMode = pxr::UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH;
  renderParams.showGuides = true;
  renderParams.showRender = true;
  renderParams.showProxy = true;
  renderParams.forceRefresh = false;
  renderParams.cullStyle = pxr::UsdImagingGLCullStyle::CULL_STYLE_NOTHING;
  renderParams.gammaCorrectColors = false;
  renderParams.enableIdRender = false;
  renderParams.enableSampleAlphaToCoverage = true;
  renderParams.highlight = true;
  renderParams.enableSceneMaterials = true;
  //_renderParams.colorCorrectionMode = ???
  renderParams.clearColor = pxr::GfVec4f(0.0, 0.0, 0.0, 1.0);


  glViewport(0, 0, width, height);
  // clear to black
  glClearColor(0.25f, 0.25f, 0.25f, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /*
  for (const auto& stage : U2X_PRIMITIVES.stages) {
    CustomPrimitive prim = Application().GetObjectFromID(stage.first);
    stage.second->Update(prim);
    HYDRA_ENGINE->Render(stage.second->Get()->GetPseudoRoot(), renderParams);
  }
  */
  U2X_SCENE->Update();
  HYDRA_ENGINE->Render(U2X_SCENE->GetStage()->GetPseudoRoot(), renderParams);
  glDisable(GL_DEPTH_TEST);
  /*
  CValueArray framebufferInfo = sequencer.GetFramebufferInfo();

  for (size_t i = 0; i < framebufferInfo.GetCount(); ++i) {
    LOG(framebufferInfo[i]);
  }



  /*
  XSI::GraphicSequencerContext graphicSequencerContext = sequencerContext;
  assert(graphicSequencerContext.IsValid());
  XSI::CGraphicSequencer sequencer = graphicSequencerContext.GetGraphicSequencer();


  //
  // Grab the current selection list
  //

  using namespace XSI;
  Application app;
  Selection sel = app.GetSelection();
  CRefArray array(sel.GetArray());

  //
  // Render the scene in hidden line
  //

  sequencer.RenderSceneUsingMode(siHiddenLineRemoval, siRenderDefault);

  //
  // Now render over the selection list using the material attached to the realtime port
  //

 // sequencer.RenderListUsingMode(array, siRealtimePortMaterial);

  //
  // Note that for the sake of simplicity, we are rendering the object using built in passes 
  // (HiddenLine and RealtimePort).But you have much more flexibility when using RenderSceneUsingMaterial 
  // and provide your own shader ie: in_pSequencer->RenderSceneUsingMaterial ( L"MySuperShader", siRenderDefault );
  //

  //
  // Using Custom Display passes become very interesting when they are use in conjuncture with pixel buffers
  // (ie. accelerated offscreen OpenGL buffers). You can create your pbuffer, make it the current rendering
  // context and use the Sequencer to render the scene. Then, you can composite this pbuffer with other 
  // buffers, use multipass, blur them. etc. etc.
  //
  // Using this technique, you can do stuff like realtime shadows, reflections or other scene-wide effects.
  //

  return;
  */

}

void UsdHydraDisplayCallback_Term( XSI::CRef sequencerContext, LPVOID *userData )
{
	// 
	// You should free user data and local memory here. 
	//
	
}

void UsdHydraDisplayCallback_InitInstance( XSI::CRef sequencerContext, LPVOID *userData )
{
}

void UsdHydraDisplayCallback_TermInstance( XSI::CRef sequencerContext, LPVOID *userData )
{
}
