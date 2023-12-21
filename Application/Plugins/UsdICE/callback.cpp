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

bool GL_EXTENSIONS_LOADED;

U2IEngine* HYDRA_ENGINE = NULL;
extern U2IScene* U2I_SCENE;
U2IScene* LAST_U2I_SCENE = NULL;
bool USDHYDRAPASSINITIALIZED = false;

static void _InitializeHydraEngine() {
  if (!HYDRA_ENGINE) {
    LOG("Initialize HYDRA ENGINE !!");
    pxr::SdfPathVector excludedPaths;
    HYDRA_ENGINE = new U2IEngine(pxr::SdfPath("/"), excludedPaths);
    
    
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

static void _InitializeGL()
{
  GarchGLApiLoad();
  GL_EXTENSIONS_LOADED = true;
}


void UsdHydra_Init ( XSI::CRef in_pSequencerContext, LPVOID *in_pUserData )
{
  GL_EXTENSIONS_LOADED = false;
  _InitializeGL();

	// Cast the CRef into a GraphicSequencerContext SDK object
	XSI::GraphicSequencerContext l_vGraphicSequencerContext = in_pSequencerContext;
	
	// make sure the cast succeeded
	assert ( l_vGraphicSequencerContext.IsValid() );

	// get the sequencer from the context object
	XSI::CGraphicSequencer in_pSequencer = l_vGraphicSequencerContext.GetGraphicSequencer ();

	// use the sequencer to register the callback
	in_pSequencer.RegisterDisplayCallback(
    L"UsdHydra",
    0,
    XSI::siPostPass,
    XSI::siCustom,
    L""
  );
}

void UsdHydra_InitInstance( XSI::CRef in_pSequencerContext, LPVOID *in_pUserData )
{
}

void UsdHydra_TermInstance( XSI::CRef in_pSequencerContext, LPVOID *in_pUserData )
{
}

void UsdHydra_Execute( XSI::CRef sequencerContext, LPVOID *userData )
{
  LOG("OoooooooooOOOOOOO");
 
  if (U2I_SCENE != LAST_U2I_SCENE) {
    _TerminateHydraEngine();
    _InitializeHydraEngine();
    LAST_U2I_SCENE = U2I_SCENE;
  }
  if (!U2I_SCENE) return;
  XSI::GraphicSequencerContext graphicSequencerContext = sequencerContext;
  assert(graphicSequencerContext.IsValid());
  XSI::CGraphicSequencer sequencer = graphicSequencerContext.GetGraphicSequencer();

  UINT x, y, width, height;
  sequencer.GetViewportSize(x, y, width, height);

  double aspectRatio = (double)width / (double)height;

  CRef cameraRef = sequencer.GetCamera();
  Camera camera(cameraRef);

  LOG("Hydra engine : " + CString(CValue(HYDRA_ENGINE)));

  LOG("SET CAMERA STATE..");
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

  U2I_SCENE->Update();
  HYDRA_ENGINE->Render(U2I_SCENE->GetSceneStage()->GetPseudoRoot(), renderParams);
  glDisable(GL_DEPTH_TEST);

}

void UsdHydra_Term( XSI::CRef sequencerContext, LPVOID *userData )
{
  _TerminateHydraEngine();
}
