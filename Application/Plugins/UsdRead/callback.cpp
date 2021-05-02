//***************************************************************************************
//
// File supervisor: Dominic Laflamme
//
//	Copyright 2008 Autodesk, Inc.  All rights reserved.  
//	Use of this software is subject to the terms of the Autodesk license agreement 
//	provided at the time of installation or download, or which otherwise accompanies 
//	this software in either electronic or hard copy form.   
//***************************************************************************************

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

#include <GL/gl.h>

U2XEngine* HYDRA_ENGINE = NULL;

pxr::UsdStageRefPtr HYDRA_STAGE;


static void _InitializeHydraEngine() {
  if (!HYDRA_ENGINE) {
    pxr::SdfPathVector excludedPaths;
    HYDRA_ENGINE = new U2XEngine(pxr::SdfPath("/"), excludedPaths);
    
    pxr::GlfSimpleMaterial material;
    pxr::GlfSimpleLight light;
    light.SetAmbient(pxr::GfVec4f(0.25, 0.25, 0.25, 1));
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
  // store the camera data
  double fov = camera.GetParameterValue(L"fov");
  double aspect = camera.GetParameterValue(L"aspect");
  double znear = camera.GetParameterValue(L"near");
  double zfar = camera.GetParameterValue(L"far");

  // const float fAspectR = (float)prim.GetParameterValue(L"aspect",time);
  // const float fAspect = (float)prim.GetParameterValue(L"projplanewidth",time)
  // / (float)prim.GetParameterValue(L"projplaneheight",time);
  /*
  mCameraSample.setFocalLength(prim.GetParameterValue(L"projplanedist", time));
  mCameraSample.setVerticalAperture(
    float(prim.GetParameterValue(L"projplaneheight", time)) * 2.54f);
  mCameraSample.setHorizontalAperture(
    float(prim.GetParameterValue(L"projplanewidth", time)) * 2.54f);
  */

  double m_invf = 1.0 / std::tan(pxr::GfDegreesToRadians(fov) * 0.5);
  znear = pxr::GfMax(znear, 0.000001);
  pxr::GfMatrix4d projectionMatrix(1.0);

  projectionMatrix[0][0] = m_invf / aspect;
  projectionMatrix[1][1] = m_invf;
  projectionMatrix[2][2] = (zfar + znear) / (znear - zfar);
  projectionMatrix[3][2] = (2 * zfar * znear) / (znear - zfar);
  projectionMatrix[2][3] = -1;
  projectionMatrix[3][3] = 0;

  return projectionMatrix;
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
  HYDRA_STAGE = pxr::UsdStage::Open("C:\\Users\\graph\\Documents\\bmal\\src\\USD_ASSETS\\Attic_NVIDIA\\Attic_NVIDIA.usd");

}

void UsdHydraDisplayCallback_Execute( XSI::CRef sequencerContext, LPVOID *userData )
{
  //
  // Call OpenGL to clear the frame buffer
  //

  ::glClearColor ( 0.2f, 0.3f, 0.4f, 1.0f );
  ::glClear ( GL_COLOR_BUFFER_BIT );

  XSI::GraphicSequencerContext graphicSequencerContext = sequencerContext;
  assert(graphicSequencerContext.IsValid());
  XSI::CGraphicSequencer sequencer = graphicSequencerContext.GetGraphicSequencer();

  UINT startX, startY, width, height;
  sequencer.GetViewportSize(startX, startY, width, height);
  double aspectRatio = (double)width / (double)height;

  CRef cameraRef = sequencer.GetCamera();
  Camera camera(cameraRef);

  pxr::GfVec2i renderResolution(width, height);
  pxr::GlfDrawTargetRefPtr drawTarget = pxr::GlfDrawTarget::New(renderResolution);
  drawTarget->Bind();

  drawTarget->AddAttachment("color",
    GL_RGBA, GL_FLOAT, GL_RGBA);
  drawTarget->AddAttachment("depth",
    GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F);
  drawTarget->Unbind();

  HYDRA_ENGINE->SetRenderViewport(
    pxr::GfVec4d(
      0.0,
      0.0,
      static_cast<double>(width),
      static_cast<double>(height))
  );
  /*_engine->SetRenderViewport(
    pxr::GfVec4f(x, y, w , h)
  );*/

  HYDRA_ENGINE->SetCameraState(
    _GetViewMatrix(camera),
    _GetProjectionMatrix(camera)
  );

  pxr::UsdImagingGLRenderParams renderParams;
  renderParams.frame = pxr::UsdTimeCode(CTime().GetTime());
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

  HYDRA_ENGINE->Render(HYDRA_STAGE->GetPseudoRoot(), renderParams);
  drawTarget->Unbind();

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
