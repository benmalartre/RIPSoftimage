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

using namespace XSI::MATH; 
using namespace XSI; 

#define LOG(msg) Application().LogMessage(msg);


static const char *VERTEX_SHADER =
"#version 330 core                                        \n"
"uniform mat4 model;                                      \n"
"uniform mat4 view;                                       \n"
"uniform mat4 projection;                                 \n"
"                                                         \n"
"in vec3 position;                                        \n"
"in vec3 normal;                                          \n"
"in vec3 color;                                           \n"
"out vec3 vertex_color;                                   \n"
"out vec3 vertex_normal;                                  \n"
"void main(){                                             \n"
"    vertex_normal = (model * vec4(normal, 0.0)).xyz;     \n"
"    vertex_color = color;                                \n"
"    vec3 p = vec3(view * model * vec4(position,1.0));    \n"
"    gl_Position = projection * vec4(p,1.0);              \n"
"}";

static const char *FRAGMENT_SHADER =
"#version 330 core                                        \n"
"in vec3 vertex_color;                                    \n"
"in vec3 vertex_normal;                                   \n"
"out vec4 outColor;                                       \n"
"void main()                                              \n"
"{                                                        \n"
" vec3 color = vertex_color + vertex_normal;              \n" 
"	outColor = vec4(vertex_color,1.0);                      \n"
"}";

/*s
struct U2XStageManager
{
  U2XStage& Get(CustomPrimitive& in_prim)
  {
    const ObjectID objectID = in_prim.GetObjectID();
    const LONG evalID = in_prim.GetEvaluationID();
    U2XStage& stage = _cache[objectID];
    
    pxr::VtArray<pxr::GfVec3f> points;
    pxr::VtArray<pxr::GfVec3f> colors;
    //if (stage._lastEvalID != evalID)
    //{
      
      CParameterRefArray& params = in_prim.GetParameters();
      CString filename = params.GetValue(L"Filename");
      if (filename != stage._filename)
      {
        stage._needReload = true;
        stage._filename = filename;
        stage._numQuads = (int)((float)rand()/(float)RAND_MAX) * 5 + 2;
      }
      
      if (stage.GetNumPrims() == 0)
      {
        int N = 6;// (int)((float)rand() / (float)RAND_MAX) * 5 + 2;
        stage._prims.reserve(N);
        for (int i = 0; i < N; ++i) {
          U2XPrim* prim = new X2UPrim(stage.Get(), pxr::SdfPath("/"));
          stage._prims.push_back(new X2U)
        }
      }
      else
      {
        pxr::VtArray<pxr::GfVec3f> colors;
        for (int i = 0; i < stage._prims.size(); ++i) {
          
        }
        
      }
      stage._lastEvalID = evalID;
    //}
    return stage;
  }

  void Drop(CustomPrimitive& in_prim)
  {
    const ObjectID objectID = in_prim.GetObjectID();
    _cache.erase(objectID);
  }

  void Clear()
  {
    _cache.clear();
    glFlush();
  }

private:
  typedef ULONG ObjectID;

  std::map<ObjectID, U2XStage> _cache;
};

static U2XStageManager _stages;
*/

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
SICALLBACK UsdPrimitiveSceneOpen_OnEvent(const CRef& in_ref)
{
  //_stages.Clear();
  return CStatus::False;
}

SICALLBACK UsdPrimitiveObjectRemoved_OnEvent(const CRef& in_ref)
{
  //_stages.Clear();
  return CStatus::False;
}

SICALLBACK UsdPrimitiveNewScene_OnEvent(const CRef& in_ref)
{
  //_stages.Clear();
  return CStatus::False;
}

static bool GL_EXTENSIONS_LOADED;
static U2XGLSLProgram GLSL_PROGRAM;

SICALLBACK XSILoadPlugin( PluginRegistrar& in_reg )
{
	in_reg.PutAuthor(L"benmalartre");
	in_reg.PutName(L"UsdPrimitive Plugin");
	in_reg.PutVersion(1,0);
	in_reg.RegisterPrimitive(L"UsdPrimitive");
  in_reg.RegisterEvent("UsdPrimitiveObjectRemoved", siOnObjectRemoved);
  in_reg.RegisterEvent("UsdPrimitiveSceneOpen", siOnBeginSceneOpen);
  in_reg.RegisterEvent("UsdPrimitiveNewScene", siOnBeginNewScene);
  GL_EXTENSIONS_LOADED = false;
	return CStatus::OK;
}

SICALLBACK XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
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
  //LOG("CTXT " + CString(ctxt.))
	oPDef = oFactory.CreateParamDef(L"File",CValue::siString,L"");
	oCustomPrimitive.AddParameter(oPDef,oParam);
  oPDef = oFactory.CreateParamDef(L"Time", CValue::siDouble, L"");
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
	oLayout.AddItem(L"File");
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

SICALLBACK UsdPrimitive_Draw( CRef& in_ctxt )
{
  GLint currentPgm;
  glGetIntegerv(GL_CURRENT_PROGRAM, &currentPgm);
  GLint currentVao;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVao);

  glPointSize(5);

  if (!GL_EXTENSIONS_LOADED)
  {
    glewInit();
    GL_EXTENSIONS_LOADED = true;
    GLSL_PROGRAM.Build("Simple", VERTEX_SHADER, FRAGMENT_SHADER);
    GLuint pgm = GLSL_PROGRAM.Get();
    // bind shader program
    glUseProgram(pgm);
    glBindAttribLocation(pgm, 0, "position");
    glBindAttribLocation(pgm, 1, "normal");
    glBindAttribLocation(pgm, 2, "color");
    glLinkProgram(pgm);

  }

  GLint renderMode;
  glGetIntegerv(GL_RENDER_MODE, &renderMode);
  if (renderMode == GL_SELECT)LOG("SELECTION RENDER MODE :D!!!");
  

  GLint pgm = GLSL_PROGRAM.Get();
  glUseProgram(pgm);

  GLfloat model[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, model);
  GLfloat proj[16];
  glGetFloatv(GL_PROJECTION_MATRIX, proj);

  GLuint modelUniform = glGetUniformLocation(pgm, "model");
  GLuint viewUniform = glGetUniformLocation(pgm, "view");
  GLuint projUniform = glGetUniformLocation(pgm, "projection");

  // view matrix
  glUniformMatrix4fv(
    viewUniform,
    1,
    GL_FALSE,
    model
  );

  // projection matrix
  glUniformMatrix4fv(
    projUniform,
    1,
    GL_FALSE,
    proj
  );

	Context ctxt( in_ctxt );
	CustomPrimitive prim = ctxt.GetSource();

  /*
  U2XStage& stage = _stages.Get(prim);
  for (int i = 0; i < stage._prims.size(); ++i)
  {
    // model matrix
    glUniformMatrix4fv(
      modelUniform,
      1,
      GL_FALSE,
      &stage._prims[i]->_xfo[0][0]
    );

    glBindVertexArray(stage._prims[i]->_vao);
    //glDrawArrays(GL_POINTS, 0, NUM_TEST_POINTS);
    glDrawElements(GL_TRIANGLES, NUM_TEST_INDICES, GL_UNSIGNED_INT, NULL);
   
  }
  */
  glBindVertexArray(currentVao);
  glUseProgram(currentPgm);

	return CStatus::OK;
}

SICALLBACK UsdPrimitive_BoundingBox( CRef& in_ctxt )
{
  LOG("CUSTOM PRIMITIVE COMPUTE BOUNDING BOX!!!");
	Context ctxt( in_ctxt );
	CustomPrimitive prim = ctxt.GetSource();
  Geometry geom = prim.GetGeometry();


	double lowerx, lowery, lowerz, upperx, uppery, upperz;
  lowerx = lowery = lowerz = -100.0;
  upperx = uppery = upperz = 100.0;
	// TODO: Update the bounding box attributes.
	ctxt.PutAttribute("LowerBoundX", lowerx);
	ctxt.PutAttribute("LowerBoundY", lowery);
	ctxt.PutAttribute("LowerBoundZ", lowerz);
	ctxt.PutAttribute("UpperBoundX", upperx);
	ctxt.PutAttribute("UpperBoundX", uppery);
	ctxt.PutAttribute("UpperBoundX", upperz);

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

