// UsdPrimitive Plugin
#include <xsi_application.h>
#include <xsi_context.h>
#include <xsi_pluginregistrar.h>
#include <xsi_status.h>
#ifndef linux
#define WIN32_LEAN_AND_MEAN
#include <windows.h> // Needed for OpenGL on windows
#endif
#include <GL/glew.h>
#include <GL/gl.h>

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
#include "window.h"

using namespace XSI::MATH; 
using namespace XSI; 

#define LOG(msg) Application().LogMessage(msg);


static const char *VERTEX_SHADER =
"#version 330                                             \n"
"uniform mat4 model;                                      \n"
"uniform mat4 view;                                       \n"
"uniform mat4 projection;                                 \n"
"                                                         \n"
"in vec3 position;                                        \n"
"in vec3 normal;                                          \n"
"in vec3 color;                                           \n"
"out vec3 vertex_normal;                                  \n"
"out vec3 vertex_color;                                   \n"
"void main(){                                             \n"
"    vertex_normal = (model * vec4(normal, 0.0)).xyz;     \n"
"    vertex_color = color;                                \n"
"    vec3 p = (view * model * vec4(position, 1.0)).xyz;   \n"
"    gl_Position = projection * vec4(p, 1.0);             \n"
"}";

static const char *FRAGMENT_SHADER =
"#version 330                                             \n"
"in vec3 vertex_normal;                                   \n"
"in vec3 vertex_color;                                    \n"
"out vec4 outColor;                                       \n"
"void main()                                              \n"
"{                                                        \n"
" vec3 color = vertex_color * vertex_normal;              \n" 
"	outColor = vec4(color, 1.0);                     \n"
"}";


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
  }

  void CheckCache()
  {
    for (auto& cache : _cache)
    {
      ObjectID objectID = cache.first;
      ProjectItem item = Application().GetObjectFromID(objectID);
      if (!item.IsValid())
      {
        if(cache.second)delete cache.second;
        _cache.erase(objectID);
      }
    }
  }

  void Clear()
  {
    for (auto& cache : _cache)
      if(cache.second)delete cache.second;
    _cache.clear();
  }

private:
  typedef ULONG ObjectID;

  std::map<ObjectID, U2XStage*> _cache;
};

static U2XPrimitiveManager _usdPrimitives;
static pxr::UsdStageCache _usdStageCache;

////////////////////////////////////////////////////////////////////////////
// Event used to evict dead objects from the cache.
//
// Currently crudely empty the cache. A more refined version would
// analyze the data provided in each event to only invalidate
// the affected objects.
//
// Other events would need to be treated too to cover all possible
// cases where the objects become invalid.
//
SICALLBACK UsdReadSceneOpen_OnEvent(const CRef& in_ref)
{
  _usdPrimitives.Clear();
  return CStatus::False;
}

SICALLBACK UsdReadObjectRemoved_OnEvent(const CRef& in_ref)
{
  Context ctxt(in_ref);
  _usdPrimitives.CheckCache();
  return CStatus::False;
}

SICALLBACK UsdReadNewScene_OnEvent(const CRef& in_ref)
{
  _usdPrimitives.Clear();
  return CStatus::False;
}

bool GL_EXTENSIONS_LOADED;
U2XGLSLProgram* GLSL_PROGRAM;

U2XWindow* U2X_HIDDEN_WINDOW;

SICALLBACK XSILoadPlugin( PluginRegistrar& in_reg )
{
	in_reg.PutAuthor(L"benmalartre");
	in_reg.PutName(L"UsdRead");
	in_reg.PutVersion(1,0);
	in_reg.RegisterPrimitive(L"UsdPrimitive");
  in_reg.RegisterEvent("UsdReadObjectRemoved", siOnObjectRemoved);
  in_reg.RegisterEvent("UsdReadSceneOpen", siOnBeginSceneOpen);
  in_reg.RegisterEvent("UsdReadNewScene", siOnBeginNewScene);

  in_reg.RegisterCustomDisplay(L"UsdExplorer");


  GL_EXTENSIONS_LOADED = false;
  U2X_HIDDEN_WINDOW = NULL;

  UsdStageCacheContext context(_usdStageCache);
	return CStatus::OK;
}

SICALLBACK XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
  delete GLSL_PROGRAM;
	CString strPluginName;
	strPluginName = in_reg.GetName();
  
	Application().LogMessage(strPluginName + L" has been unloaded.",siVerboseMsg);
	return CStatus::OK;
}

SICALLBACK UsdPrimitive_Define( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	CustomPrimitive oCustomPrimitive;
	Parameter oParam;
	CRef oPDef;
	Factory oFactory = Application().GetFactory();
	oCustomPrimitive = ctxt.GetSource();
	oPDef = oFactory.CreateParamDef(L"Filename",CValue::siString,L"");
	oCustomPrimitive.AddParameter(oPDef,oParam);
  oPDef = oFactory.CreateParamDef(L"Time", CValue::siDouble, 1.0);
  oCustomPrimitive.AddParameter(oPDef, oParam);

	return CStatus::OK;
}

// Tip: Use the "Refresh" option on the Property Page context menu to 
// reload your script changes and re-execute the DefineLayout callback.
SICALLBACK UsdPrimitive_DefineLayout( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	PPGLayout oLayout;
	PPGItem oItem;
	oLayout = ctxt.GetSource();
	oLayout.Clear();
	oLayout.AddItem(L"Filename");
  oLayout.AddItem(L"Time");
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

  U2XStage* stage = _usdPrimitives.Get(prim);
  if (!stage)
  {
    stage = new U2XStage();
    _usdPrimitives.Set(prim, stage);
  }

  stage->Update(prim);

  ctxt.PutAttribute("LowerBoundX", U2XGetBoundingBoxComponent(stage->GetBBox(), BBOX_LOWER_X));
  ctxt.PutAttribute("LowerBoundY", U2XGetBoundingBoxComponent(stage->GetBBox(), BBOX_LOWER_Y));
  ctxt.PutAttribute("LowerBoundZ", U2XGetBoundingBoxComponent(stage->GetBBox(), BBOX_LOWER_Z));
  ctxt.PutAttribute("UpperBoundX", U2XGetBoundingBoxComponent(stage->GetBBox(), BBOX_UPPER_X));
  ctxt.PutAttribute("UpperBoundY", U2XGetBoundingBoxComponent(stage->GetBBox(), BBOX_UPPER_Y));
  ctxt.PutAttribute("UpperBoundZ", U2XGetBoundingBoxComponent(stage->GetBBox(), BBOX_UPPER_Z));
  
  return CStatus::OK;

}


SICALLBACK UsdPrimitive_Draw( CRef& in_ctxt )
{
  Context ctxt(in_ctxt);
  CustomPrimitive prim = ctxt.GetSource();
  U2XStage* stage = _usdPrimitives.Get(prim);
  if (!stage)return CStatus::Fail;

  GLint currentPgm;
  glGetIntegerv(GL_CURRENT_PROGRAM, &currentPgm);
  GLint currentVao;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVao);
  GLint currentDepthTest;
  glGetIntegerv(GL_DEPTH_TEST, &currentDepthTest);

  if (!GL_EXTENSIONS_LOADED)
  {
    glewInit();
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

    /*
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    */
    // view matrix
    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, view);

    // projection matrix
    glUniformMatrix4fv(projUniform, 1, GL_FALSE, proj);

    stage->Draw();

  }
  glBindVertexArray(currentVao);
  glUseProgram(currentPgm);

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

