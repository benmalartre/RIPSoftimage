// UsdPrimitive Plugin
#include <xsi_application.h>
#include <xsi_context.h>
#include <xsi_pluginregistrar.h>
#include <xsi_status.h>
#ifndef linux
#define WIN32_LEAN_AND_MEAN
#include <windows.h> // Needed for OpenGL on windows
#endif

#include <xsi_math.h>
#include <xsi_argument.h>
#include <xsi_customprimitive.h>
#include <xsi_customprimitivecontext.h>
#include <xsi_factory.h>
#include <xsi_ppglayout.h>
#include <xsi_ppgeventcontext.h>
#include <xsi_geometry.h>
#include <xsi_iceattribute.h>
#include <xsi_iceattributedataarray.h>
#include <map>
#include <vector>

#include "utils.h"
#include "shader.h"
#include "prim.h"
#include "stage.h"
#include "scene.h"
#include "window.h"
#include "callback.h"


using namespace XSI::MATH; 
using namespace XSI; 

bool GL_EXTENSIONS_LOADED;
U2XGLSLProgram* GLSL_PROGRAM;
extern U2XScene* U2X_SCENE;

static void _InitializeGL()
{
  GarchGLApiLoad();
  GL_EXTENSIONS_LOADED = true;

  GLSL_PROGRAM = new U2XGLSLProgram();
  GLSL_PROGRAM->Build("Simple", VERTEX_SHADER, FRAGMENT_SHADER);
  GLuint pgm = GLSL_PROGRAM->Get();
  // bind shader program
  glUseProgram(pgm);
  glBindAttribLocation(pgm, CHANNEL_POSITION, "position");
  glBindAttribLocation(pgm, CHANNEL_NORMAL, "normal");
  glBindAttribLocation(pgm, CHANNEL_COLOR, "color");
  glLinkProgram(pgm);
}

SICALLBACK XSILoadPlugin( PluginRegistrar& in_reg )
{
  in_reg.PutAuthor(L"benmalartre");
  in_reg.PutName(L"UsdRead");
  in_reg.PutVersion(1,0);
  in_reg.RegisterPrimitive(L"UsdPrimitive");
  in_reg.RegisterProperty(L"UsdPrimitive");

  in_reg.RegisterEvent("UsdReadValueChange", siOnValueChange);
  in_reg.RegisterEvent("UsdReadObjectAdded", siOnObjectAdded);
  in_reg.RegisterEvent("UsdReadObjectRemoved", siOnObjectRemoved);
  in_reg.RegisterEvent("UsdReadSceneOpen", siOnBeginSceneOpen);
  in_reg.RegisterEvent("UsdReadNewScene", siOnBeginNewScene);
  in_reg.RegisterEvent("UsdReadTimeChange", siOnTimeChange);

  in_reg.RegisterCustomDisplay(L"UsdExplorer");
  in_reg.RegisterDisplayCallback( L"UsdHydraDisplayCallback" );

  GL_EXTENSIONS_LOADED = false;
  U2X_HIDDEN_WINDOW = NULL;
  _InitializeGL();
  UsdStageCacheContext context(U2X_USDSTAGE_CACHE);
  U2X_SCENE = new U2XScene();

  return CStatus::OK;
}

SICALLBACK XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
  if (U2X_HIDDEN_WINDOW)delete U2X_HIDDEN_WINDOW;
  delete GLSL_PROGRAM;
  CString strPluginName;
  strPluginName = in_reg.GetName();
  delete U2X_SCENE;
  GarchGLApiUnload();
  Application().LogMessage(strPluginName + L" has been unloaded.", siVerboseMsg);
  return CStatus::OK;
}

SICALLBACK UsdPrimitive_Define( CRef& in_ctxt )
{
  Context ctxt( in_ctxt );
  CustomPrimitive prim;
  Parameter param;
  CRef ref;
  Factory factory = Application().GetFactory();
  prim = ctxt.GetSource();
  ref = factory.CreateParamDef(L"Filename", CValue::siString, L"");
  prim.AddParameter(ref, param);
  ref = factory.CreateParamDef(L"Time", CValue::siDouble, 1.0);
  prim.AddParameter(ref, param);
  ref = factory.CreateParamDef(L"Update", CValue::siBool, false);
  prim.AddParameter(ref, param);


  return CStatus::OK;
}

// Tip: Use the "Refresh" option on the Property Page context menu to 
// reload your script changes and re-execute the DefineLayout callback.
SICALLBACK UsdPrimitive_DefineLayout( CRef& in_ctxt )
{
  Context ctxt( in_ctxt );
  PPGLayout layout;
  PPGItem item;
  layout = ctxt.GetSource();
  layout.Clear();
  item = layout.AddItem(L"Filename", L"Filename", siControlFilePath);
  layout.AddItem(L"Time");

  return CStatus::OK;
}

SICALLBACK UsdPrimitive_PPGEvent( const CRef& in_ctxt )
{
  // This callback is called when events happen in the user interface
  // This is where you implement the "logic" code.
  // If the value of a parameter changes but the UI is not shown then this
  // code will not execute.  Also this code is not re-entrant, so any changes
  // to parameters inside this code will not result in further calls to this function
  Application app ;

  // The context object is used to determine exactly what happened
  // We don't use the same "PPG" object that is used from Script-based logic code 
  // but through the C++ API we can achieve exactly the same functionality.
  PPGEventContext ctxt( in_ctxt ) ;

  PPGEventContext::PPGEvent eventID = ctxt.GetEventID() ;

  if ( eventID == PPGEventContext::siOnInit )
  {
    // This event meant that the UI was just created.
    // It gives us a chance to set some parameter values.
    // We could even change the layout completely at this point.

    // Get all inspected CustomPrimitive objects
    CRefArray props = ctxt.GetInspectedObjects();

    for (LONG i=0; i<props.GetCount( ); i++)
    {
      CustomPrimitive prop( props[i] );
      app.LogMessage( L"OnInit called for " + prop.GetFullName(), siVerboseMsg ) ;
    }

    /* If you regenerate the layout then call this:
    ctxt.PutAttribute(L"Refresh",true);
    */
  }
  else if ( eventID == PPGEventContext::siOnClosed )
  {
    // This event meant that the UI was just closed by the user.

    // Get all inspected CustomPrimitive objects
    CRefArray props = ctxt.GetInspectedObjects();
    for (LONG i=0; i<props.GetCount( ); i++)
    {
      CustomPrimitive prop( props[i] );
      app.LogMessage( L"OnClosed called for " + prop.GetFullName(), siVerboseMsg ) ;
    }
  }
  else if ( eventID == PPGEventContext::siButtonClicked )
  {
    // If there are multiple buttons 
    // we can use this attribute to figure out which one was clicked.
    CValue buttonPressed = ctxt.GetAttribute( L"Button" ) ;	

    // Get all inspected CustomPrimitive objects
    CRefArray props = ctxt.GetInspectedObjects();
    for (LONG i=0; i<props.GetCount( ); i++)
    {
      CustomPrimitive prop( props[i] );
      app.LogMessage( L"Button pressed: " + buttonPressed.GetAsText() + CString(" while inspecting " ) + prop.GetFullName() );
    }
  }
  else if ( eventID == PPGEventContext::siTabChange )
  {
    // We will be called when the PPG is first opened
    // and every time the tab changes

    // Retrieve the label of the tab that is now active
    CValue tabLabel = ctxt.GetAttribute( L"Tab" ) ;

    // Get all inspected CustomPrimitive objects
    CRefArray props = ctxt.GetInspectedObjects();
    for (LONG i=0; i<props.GetCount( ); i++)
    {
      CustomPrimitive prop( props[i] );
      app.LogMessage( L"Tab changed to: " + tabLabel.GetAsText() + CString(" while inspecting " ) + prop.GetFullName() );
    }
  }
  else if ( eventID == PPGEventContext::siParameterChange )
  {
    // For this event the Source of the event is the parameter
    // itself
    Parameter changed = ctxt.GetSource() ;	
    CustomPrimitive prop = changed.GetParent() ;	
    CString paramName = changed.GetScriptName() ; 

    app.LogMessage( L"Parameter Changed: " + paramName, siVerboseMsg ) ;
  }
  return CStatus::OK ;
}

SICALLBACK UsdPrimitive_BoundingBox(CRef& in_ref)
{
  Context ctxt(in_ref);

  CustomPrimitive prim(ctxt.GetSource());
  if (!prim.IsValid())return CStatus::Fail;
  U2XStage* stage = U2X_PRIMITIVES.Get(prim);
  
  if (!stage)
  {
    stage = new U2XStage();
    U2X_PRIMITIVES.Set(prim, stage);
  }

  stage->Update(prim);

  const pxr::GfBBox3d& bbox = stage->GetBBox();
  ctxt.PutAttribute("LowerBoundX", U2XGetBoundingBoxComponent(bbox, BBOX_LOWER_X));
  ctxt.PutAttribute("LowerBoundY", U2XGetBoundingBoxComponent(bbox, BBOX_LOWER_Y));
  ctxt.PutAttribute("LowerBoundZ", U2XGetBoundingBoxComponent(bbox, BBOX_LOWER_Z));
  ctxt.PutAttribute("UpperBoundX", U2XGetBoundingBoxComponent(bbox, BBOX_UPPER_X));
  ctxt.PutAttribute("UpperBoundY", U2XGetBoundingBoxComponent(bbox, BBOX_UPPER_Y));
  ctxt.PutAttribute("UpperBoundZ", U2XGetBoundingBoxComponent(bbox, BBOX_UPPER_Z));
  
  return CStatus::OK;

}


SICALLBACK UsdPrimitive_Draw( CRef& in_ctxt )
{
  /*
  Context ctxt(in_ctxt);
  CustomPrimitive prim = ctxt.GetSource();
  U2XStage* stage = U2X_PRIMITIVES.Get(prim);
  if (!stage)return CStatus::Fail;

  GLint currentPgm;
  GLint currentVao;
  GLint currentDepthTest;
  GLint currentDepthFunc;

  glGetIntegerv(GL_CURRENT_PROGRAM, &currentPgm);
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVao);
  glGetIntegerv(GL_DEPTH_TEST, &currentDepthTest);
  glGetIntegerv(GL_DEPTH_FUNC, &currentDepthFunc);

  if (stage->IsLoaded())
  {
    GLint pgm = GLSL_PROGRAM->Get();
    glUseProgram(pgm);

    GLfloat view[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, view);
    GLfloat proj[16];
    glGetFloatv(GL_PROJECTION_MATRIX, proj);


    GLuint modelUniform = glGetUniformLocation(pgm, "model");
    GLuint viewUniform = glGetUniformLocation(pgm, "view");
    GLuint projUniform = glGetUniformLocation(pgm, "projection");
    
    glDepthFunc(GL_LESS);
    
    //glEnable(GL_DEPTH_TEST);
    //glDepthMask(GL_TRUE);
    //glDisable(GL_CULL_FACE);
    //glDisable(GL_BLEND);
    
    // view matrix
    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, view);

    // projection matrix
    glUniformMatrix4fv(projUniform, 1, GL_FALSE, proj);

    stage->Draw();
  }
  glBindVertexArray(currentVao);
  glUseProgram(currentPgm);
  glBindVertexArray(currentVao);
  
  if (currentDepthTest)glEnable(GL_DEPTH_TEST);
  else glDisable(GL_DEPTH_TEST);
  glDepthFunc(currentDepthFunc);
  */
  return CStatus::OK;
}

SICALLBACK UsdPrimitive_ConvertToGeom( CRef& in_ctxt )
{
  CustomPrimitiveContext ctxt( in_ctxt );
  CustomPrimitive oCustomPrimitive = ctxt.GetSource();

  Geometry oGeometry = ctxt.GetGeometry();
  // TODO: Update the geometry here.
  return CStatus::OK;
}

