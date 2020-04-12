#include "common.h"
#include <xsi_application.h>
#include <xsi_property.h>
#include <xsi_ppglayout.h>
#include <xsi_ppgeventcontext.h>

SICALLBACK UsdWriteUI_Define(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  CustomProperty oCustomProperty;
  Parameter oParam;
  oCustomProperty = ctxt.GetSource();
  oCustomProperty.AddParameter(L"Folder", CValue::siString, siPersistable | siKeyable, L"", L"", L"", oParam);
  oCustomProperty.AddParameter(L"FileName", CValue::siString, siPersistable | siKeyable, L"", L"", L"", oParam);
  oCustomProperty.AddParameter(L"TimeMode", CValue::siUInt1, siPersistable | siKeyable, L"", L"", 0l, 0l, 1l, 0l, 1l, oParam);
  oCustomProperty.AddParameter(L"ExportPolygonMeshes", CValue::siBool, siPersistable, L"", L"", false, CValue(), CValue(), CValue(), CValue(), oParam);
  oCustomProperty.AddParameter(L"ExportPointClouds", CValue::siBool, siPersistable, L"", L"", false, CValue(), CValue(), CValue(), CValue(), oParam);
  oCustomProperty.AddParameter(L"ExportCurves", CValue::siBool, siPersistable, L"", L"", false, CValue(), CValue(), CValue(), CValue(), oParam);
  oCustomProperty.AddParameter(L"ExportCameras", CValue::siBool, siPersistable, L"", L"", false, CValue(), CValue(), CValue(), CValue(), oParam);
  oCustomProperty.AddParameter(L"ExportLights", CValue::siBool, siPersistable, L"", L"", false, CValue(), CValue(), CValue(), CValue(), oParam);
  oCustomProperty.AddParameter(L"StartFrame", CValue::siFloat, siPersistable, L"", L"", 1l, -10000l, 10000l, -10000l, 10000l, oParam);
  oCustomProperty.AddParameter(L"EndFrame", CValue::siFloat, siPersistable, L"", L"", 100l, -10000l, 10000l, -10000l, 10000l, oParam);
  return CStatus::OK;
}

SICALLBACK UsdWriteUI_DefineLayout(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  PPGLayout layout;
  PPGItem item;
  layout = ctxt.GetSource();
  layout.Clear();
  layout.AddTab(L"Main");
  layout.AddGroup(L"File System");
  layout.AddItem(L"Folder");
  layout.AddItem(L"FileName");
  layout.EndGroup();
  layout.AddGroup(L"Time");
  layout.AddItem(L"TimeMode");
  layout.AddItem(L"StartFrame");
  layout.AddItem(L"EndFrame");
  layout.EndGroup();
  layout.AddGroup(L"Export Options");
  layout.AddItem(L"ExportPolygonMeshes", L"Meshes");
  layout.AddItem(L"ExportPointClouds", L"Points");
  layout.AddItem(L"ExportCurves", L"Curves");
  layout.AddItem(L"ExportCameras", L"Cameras");
  layout.AddItem(L"ExportLights", L"Lights");
  layout.EndGroup();

  layout.AddGroup(L"");
  layout.AddButton(L"Write", L"Write");
  layout.EndGroup();
  return CStatus::OK;
}

SICALLBACK UsdWriteUI_PPGEvent(const CRef& in_ctxt)
{
  Application app;
  PPGEventContext ctxt(in_ctxt);

  PPGEventContext::PPGEvent eventID = ctxt.GetEventID();

  if (eventID == PPGEventContext::siOnInit)
  {
    CRefArray props = ctxt.GetInspectedObjects();

    for (LONG i = 0; i<props.GetCount(); i++)
    {
      CustomProperty prop(props[i]);
      app.LogMessage(L"OnInit called for " + prop.GetFullName(), siVerboseMsg);
    }

    /* If you regenerate the layout then call this:
    ctxt.PutAttribute(L"Refresh",true);
    */
  }
  else if (eventID == PPGEventContext::siOnClosed)
  {
    // Get all inspected CustomProperty objects
    CRefArray props = ctxt.GetInspectedObjects();

    for (LONG i = 0; i<props.GetCount(); i++)
    {
      CustomProperty prop(props[i]);
      app.LogMessage(L"OnClosed called for " + prop.GetFullName(), siVerboseMsg);
    }
  }
  else if (eventID == PPGEventContext::siButtonClicked)
  {
    // If there are multiple buttons 
    // we can use this attribute to figure out which one was clicked.
    CValue buttonPressed = ctxt.GetAttribute(L"Button");

    // Get all inspected CustomProperty objects
    CRefArray props = ctxt.GetInspectedObjects();

    for (LONG i = 0; i<props.GetCount(); i++)
    {
      CustomProperty prop(props[i]);
      app.LogMessage(L"Button pressed: " + buttonPressed.GetAsText() + CString(" while inspecting ") + prop.GetFullName());
      if (buttonPressed.GetAsText().IsEqualNoCase(L"Write"))
      {
        LOG("FUCKIN PRESSED :D!!");
        CValueArray args;
        //args.Add(CValue(L"This Is A Fuckin Test..."));
        Application().ExecuteCommand(L"UsdWrite", args, CValue());
      }
        
    }
  }
  else if (eventID == PPGEventContext::siTabChange)
  {
    // Retrieve the label of the tab that is now active
    CValue tabLabel = ctxt.GetAttribute(L"Tab");

    // Get all inspected CustomProperty objects
    CRefArray props = ctxt.GetInspectedObjects();

    for (LONG i = 0; i<props.GetCount(); i++)
    {
      CustomProperty prop(props[i]);
      app.LogMessage(L"Tab changed to: " + tabLabel.GetAsText() + CString(" while inspecting ") + prop.GetFullName());
    }
  }
  else if (eventID == PPGEventContext::siParameterChange)
  {
    Parameter changed = ctxt.GetSource();
    CustomProperty prop = changed.GetParent();
    CString paramName = changed.GetScriptName();

    app.LogMessage(L"Parameter Changed: " + paramName, siVerboseMsg);
  }

  return CStatus::OK;
}

