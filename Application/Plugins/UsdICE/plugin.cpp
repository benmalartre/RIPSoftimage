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
#include <xsi_operator.h>
#include <xsi_iceattribute.h>
#include <xsi_iceattributedataarray.h>
#include <map>
#include <vector>

#include "utils.h"
#include "stage.h"
#include "scene.h"
#include "callback.h"


using namespace XSI::MATH;
using namespace XSI;

extern U2IScene* U2I_SCENE;

#ifndef unix
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
#endif

XSI::CStatus RegisterUsdStageNode(XSI::PluginRegistrar& in_reg);
XSI::CStatus RegisterUsdSphereNode(XSI::PluginRegistrar& in_reg);
XSI::CStatus RegisterUsdCubeNode(XSI::PluginRegistrar& in_reg);

SICALLBACK XSILoadPlugin(PluginRegistrar& in_reg)
{
  in_reg.PutAuthor(L"benmalartre");
  in_reg.PutName(L"UsdICE");
  in_reg.PutVersion(1, 0);

  in_reg.RegisterEvent("UsdICEValueChange", siOnValueChange);
  in_reg.RegisterEvent("UsdICEObjectAdded", siOnObjectAdded);
  in_reg.RegisterEvent("UsdICEObjectRemoved", siOnObjectRemoved);
  in_reg.RegisterEvent("UsdICESceneOpen", siOnBeginSceneOpen);
  in_reg.RegisterEvent("UsdICENewScene", siOnBeginNewScene);
  in_reg.RegisterEvent("UsdICETimeChange", siOnTimeChange);

  //
	// Here we tell the Plugin Registrar that this DLL exports 
	// a display callback Called MyClearScreenBuffer
	//
	in_reg.RegisterDisplayCallback( L"UsdHydra" );

  RegisterUsdStageNode(in_reg);
  RegisterUsdSphereNode(in_reg);
  RegisterUsdCubeNode(in_reg);

  pxr::UsdStageCacheContext context(U2I_USDSTAGE_CACHE);
  U2I_SCENE = new U2IScene();
  
  return CStatus::OK;
}

SICALLBACK XSIUnloadPlugin(const PluginRegistrar& in_reg)
{
  delete U2I_SCENE;
  GarchGLApiUnload();
  CString strPluginName;
  strPluginName = in_reg.GetName();
  Application().LogMessage(strPluginName + L" has been unloaded.", siVerboseMsg);
  return CStatus::OK;
}

SICALLBACK UsdPrimitive_Define(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
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
SICALLBACK UsdPrimitive_DefineLayout(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  PPGLayout layout;
  PPGItem item;
  layout = ctxt.GetSource();
  layout.Clear();
  item = layout.AddItem(L"Filename", L"Filename", siControlFilePath);
  layout.AddItem(L"Time");

  return CStatus::OK;
}

SICALLBACK UsdPrimitive_PPGEvent(const CRef& in_ctxt)
{
  // This callback is called when events happen in the user interface
  // This is where you implement the "logic" code.
  // If the value of a parameter changes but the UI is not shown then this
  // code will not execute.  Also this code is not re-entrant, so any changes
  // to parameters inside this code will not result in further calls to this function
  Application app;

  // The context object is used to determine exactly what happened
  // We don't use the same "PPG" object that is used from Script-based logic code 
  // but through the C++ API we can achieve exactly the same functionality.
  PPGEventContext ctxt(in_ctxt);

  PPGEventContext::PPGEvent eventID = ctxt.GetEventID();

  if (eventID == PPGEventContext::siOnInit)
  {
    // This event meant that the UI was just created.
    // It gives us a chance to set some parameter values.
    // We could even change the layout completely at this point.

    // Get all inspected CustomPrimitive objects
    CRefArray props = ctxt.GetInspectedObjects();

    for (LONG i = 0; i < props.GetCount(); i++)
    {
      CustomPrimitive prop(props[i]);
      app.LogMessage(L"OnInit called for " + prop.GetFullName(), siVerboseMsg);
    }

    /* If you regenerate the layout then call this:
    ctxt.PutAttribute(L"Refresh",true);
    */
  }
  else if (eventID == PPGEventContext::siOnClosed)
  {
    // This event meant that the UI was just closed by the user.

    // Get all inspected CustomPrimitive objects
    CRefArray props = ctxt.GetInspectedObjects();
    for (LONG i = 0; i < props.GetCount(); i++)
    {
      CustomPrimitive prop(props[i]);
      app.LogMessage(L"OnClosed called for " + prop.GetFullName(), siVerboseMsg);
    }
  }
  else if (eventID == PPGEventContext::siButtonClicked)
  {
    // If there are multiple buttons 
    // we can use this attribute to figure out which one was clicked.
    CValue buttonPressed = ctxt.GetAttribute(L"Button");

    // Get all inspected CustomPrimitive objects
    CRefArray props = ctxt.GetInspectedObjects();
    for (LONG i = 0; i < props.GetCount(); i++)
    {
      CustomPrimitive prop(props[i]);
      app.LogMessage(L"Button pressed: " + buttonPressed.GetAsText() + CString(" while inspecting ") + prop.GetFullName());
    }
  }
  else if (eventID == PPGEventContext::siTabChange)
  {
    // We will be called when the PPG is first opened
    // and every time the tab changes

    // Retrieve the label of the tab that is now active
    CValue tabLabel = ctxt.GetAttribute(L"Tab");

    // Get all inspected CustomPrimitive objects
    CRefArray props = ctxt.GetInspectedObjects();
    for (LONG i = 0; i < props.GetCount(); i++)
    {
      CustomPrimitive prop(props[i]);
      app.LogMessage(L"Tab changed to: " + tabLabel.GetAsText() + CString(" while inspecting ") + prop.GetFullName());
    }
  }
  else if (eventID == PPGEventContext::siParameterChange)
  {
    // For this event the Source of the event is the parameter
    // itself
    Parameter changed = ctxt.GetSource();
    CustomPrimitive prop = changed.GetParent();
    CString paramName = changed.GetScriptName();

    app.LogMessage(L"Parameter Changed: " + paramName, siVerboseMsg);
  }
  return CStatus::OK;
}