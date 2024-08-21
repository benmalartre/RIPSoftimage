#include <assert.h>
#include <xsi_decl.h>
#include <xsi_color.h>
#include <xsi_light.h>
#include <xsi_ogllight.h>
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
#include <pxr/imaging/garch/glApi.h>

U2XEngine* HYDRA_ENGINE = NULL;
extern U2XScene* U2X_SCENE;
U2XScene* LAST_U2X_SCENE = NULL;


static void _GetLightsFromScene(pxr::GlfSimpleLightVector& lights) 
{
  lights.clear();
  Model root = Application().GetActiveSceneRoot();
  XSI::CSIObjectRefArray xsiLights = root.FindObjects( siLightID );
 
  for ( LONG i=0; i<xsiLights.GetCount(); i++ )
  {
    XSI::OGLLight glLight(XSI::Light(xsiLights[i]).GetOGLLight());
    XSI::siLightType lightType = glLight.GetType();
    XSI::MATH::CVector3 lightPosition = glLight.GetLightPosition();
    XSI::CColor lightColor = glLight.GetColor();

    pxr::GlfSimpleLight light;
    if(lightType == XSI::siLightInfinite)light.SetIsDomeLight(false);
    else light.SetPosition(pxr::GfVec4f(lightPosition[0], lightPosition[1], lightPosition[2], 1));

    light.SetDiffuse(pxr::GfVec4f(lightColor.r,lightColor.g, lightColor.b, 1.0));
    
    lights.push_back(light);
  }

}

static void _InitializeHydraEngine() {
  if (!HYDRA_ENGINE) {
    pxr::SdfPathVector excludedPaths;
    HYDRA_ENGINE = new U2XEngine(pxr::SdfPath("/"), excludedPaths);
    
    pxr::GlfSimpleLightVector lights;
    _GetLightsFromScene(lights); 

    pxr::GlfSimpleMaterial material;
    material.SetAmbient(pxr::GfVec4f(0.2, 0.2, 0.2, 1.0));
    
    HYDRA_ENGINE->SetLightingState(lights,
      material,
      pxr::GfVec4f(0.5, 0.5, 0.5, 1.0));
    
  }
}

static void _TerminateHydraEngine() {
  if(HYDRA_ENGINE)delete HYDRA_ENGINE;
  HYDRA_ENGINE = NULL;
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

void UsdHydraDisplayCallback_Init( XSI::CRef context, LPVOID *data )
{
  XSI::GraphicSequencerContext sequencerContext = context;
  assert ( sequencerContext.IsValid() );
  XSI::CGraphicSequencer sequencer = sequencerContext.GetGraphicSequencer ();
  sequencer.RegisterDisplayCallback(
    L"UsdHydraDisplayCallback", 
    0, 
    XSI::siPostBeginFrame, 
    XSI::siAll, 
    L"UsdHydra"
  );
  LOG("Hydra Init Done");
}

void UsdHydraDisplayCallback_Execute( XSI::CRef context, LPVOID *data )
{
LOG("Hydra Display Callback");
  
  if (U2X_SCENE != LAST_U2X_SCENE) {
    _TerminateHydraEngine();
    _InitializeHydraEngine();
    LAST_U2X_SCENE = U2X_SCENE;
  }
if (U2X_SCENE->IsEmpty())return;
  XSI::GraphicSequencerContext sequencerContext = context;
  assert(sequencerContext.IsValid());
  XSI::CGraphicSequencer sequencer = sequencerContext.GetGraphicSequencer();

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

  pxr::GlfSimpleMaterial material;
  material.SetDiffuse(pxr::GfVec4f(1.0, 1.0, 1.0, 1.0));
  material.SetAmbient(pxr::GfVec4f(0.2, 0.2, 0.2, 1.0));

  pxr::GlfSimpleLightVector lights;
  _GetLightsFromScene(lights); 

  HYDRA_ENGINE->SetLightingState(lights,
    material,
    pxr::GfVec4f(0.5, 0.5, 0.5, 1.0));

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
  renderParams.enableLighting = true;
  renderParams.enableSceneMaterials = true;
  //_renderParams.colorCorrectionMode = ???
  renderParams.clearColor = pxr::GfVec4f(0.0, 0.0, 0.0, 1.0);


  glViewport(0, 0, width, height);
  // clear to black
  glClearColor(0.25f, 0.25f, 0.25f, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  U2X_SCENE->Update();
  HYDRA_ENGINE->Render(U2X_SCENE->GetSceneStage()->GetPseudoRoot(), renderParams);
  glDisable(GL_DEPTH_TEST);

}

void UsdHydraDisplayCallback_Term( XSI::CRef sequencerContext, LPVOID *userData )
{
 
}

void UsdHydraDisplayCallback_InitInstance( XSI::CRef sequencerContext, LPVOID *userData )
{
}

void UsdHydraDisplayCallback_TermInstance( XSI::CRef sequencerContext, LPVOID *userData )
{
  _TerminateHydraEngine();
}
