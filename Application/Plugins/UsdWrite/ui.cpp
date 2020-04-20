#include "common.h"
#include <xsi_application.h>
#include <xsi_property.h>
#include <xsi_ppglayout.h>
#include <xsi_ppgeventcontext.h>

CString kFolder = "Folder";
CString kFilename = "Filename";
CString kTimeMode = "TimeMode";
CString kExportMeshes = "ExportMeshes";
CString kExportPoints = "ExportPoints";
CString kExportCurves = "ExportCurves";
CString kExportCameras = "ExportCameras";
CString kExportLights = "ExportLights";
CString kStartFrame = "StartFrame";
CString kEndFrame = "EndFrame";
CString kWrite = "Write";

SICALLBACK UsdWriteUI_Define(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  CustomProperty prop;
  Parameter param;
  prop = ctxt.GetSource();
  prop.AddParameter(kFolder, CValue::siString, siPersistable | siKeyable, "", "", "", param);
  prop.AddParameter(kFilename, CValue::siString, siPersistable | siKeyable, "", "", "", param);
  prop.AddParameter(kTimeMode, CValue::siUInt1, siPersistable | siKeyable, "", "", 0l, 0l, 1l, 0l, 1l, param);
  prop.AddParameter(kExportMeshes, CValue::siBool, siPersistable, "", "", false, CValue(), CValue(), CValue(), CValue(), param);
  prop.AddParameter(kExportPoints, CValue::siBool, siPersistable, "", "", false, CValue(), CValue(), CValue(), CValue(), param);
  prop.AddParameter(kExportCurves, CValue::siBool, siPersistable, "", "", false, CValue(), CValue(), CValue(), CValue(), param);
  prop.AddParameter(kExportCameras, CValue::siBool, siPersistable, "", "", false, CValue(), CValue(), CValue(), CValue(), param);
  prop.AddParameter(kExportLights, CValue::siBool, siPersistable, "", "", false, CValue(), CValue(), CValue(), CValue(), param);
  prop.AddParameter(kStartFrame, CValue::siFloat, siPersistable, "", "", 1l, -10000l, 10000l, -10000l, 10000l, param);
  prop.AddParameter(kEndFrame, CValue::siFloat, siPersistable, "", "", 100l, -10000l, 10000l, -10000l, 10000l, param);
  return CStatus::OK;
}

SICALLBACK UsdWriteUI_DefineLayout(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  PPGLayout layout;
  PPGItem item;
  layout = ctxt.GetSource();
  layout.Clear();
  layout.AddTab("Main");
  layout.AddGroup("File System");
  layout.AddItem(kFolder, "", siControlFolder);
  layout.AddItem(kFilename);
  layout.EndGroup();
  layout.AddGroup("Time");
  CValueArray timeModeItems(4);
  timeModeItems[0] = "From Scene";
  timeModeItems[1] = 0l;
  timeModeItems[2] = "Frame Range";
  timeModeItems[3] = 1l;
  layout.AddEnumControl(kTimeMode, timeModeItems, "Mode", siControlCombo);
  layout.AddItem(kStartFrame);
  layout.AddItem(kEndFrame);
  layout.EndGroup();
  layout.AddGroup("Export Options");
  layout.AddItem(kExportMeshes, "Meshes");
  layout.AddItem(kExportPoints, "Points");
  layout.AddItem(kExportCurves, "Curves");
  layout.AddItem(kExportCameras, "Cameras");
  layout.AddItem(kExportLights, "Lights");
  layout.EndGroup();

  layout.AddGroup("");
  layout.AddRow();
  item = layout.AddButton(kWrite, "Write");
  item.PutAttribute(siUICX, 300);
  item.PutAttribute(siUICY, 60);
  layout.EndRow();
  layout.EndGroup();
  return CStatus::OK;
}

void UsdWriteUI_UpdateTimeMode(const CustomProperty& prop)
{
  CParameterRefArray params = prop.GetParameters();
  Parameter param;
  size_t timeMode = params.GetValue(kTimeMode);
  bool readOnly = true;
  if (timeMode == 1l) readOnly = false;

  param = params.GetItem(kStartFrame);
  param.PutCapabilityFlag(siCapabilities::siReadOnly, readOnly);
  param = params.GetItem(kEndFrame);
  param.PutCapabilityFlag(siCapabilities::siReadOnly, readOnly);
}

SICALLBACK UsdWriteUI_PPGEvent(const CRef& in_ctxt)
{
  Application app;
  PPGEventContext ctxt(in_ctxt);

  PPGEventContext::PPGEvent eventID = ctxt.GetEventID();
  bool needRefresh = false;
  switch (eventID)
  {
    case PPGEventContext::siParameterChange:
    {
      Parameter changed = ctxt.GetSource();
      CustomProperty prop = changed.GetParent();
      CString paramName = changed.GetScriptName();
      if (paramName == kTimeMode)
      {
        UsdWriteUI_UpdateTimeMode(prop);
        needRefresh = true;
      }
      break;
    }
    case PPGEventContext::siOnInit:
    {
      CRefArray props = ctxt.GetInspectedObjects();

      for (LONG i = 0; i < props.GetCount(); i++)
      {
        CustomProperty prop(props[i]);
        UsdWriteUI_UpdateTimeMode(prop);
      }
      needRefresh = true;
      break;
    }
    case PPGEventContext::siOnClosed:
    {
      // Get all inspected CustomProperty objects
      CRefArray props = ctxt.GetInspectedObjects();

      for (LONG i = 0; i < props.GetCount(); i++)
      {
        CustomProperty prop(props[i]);
        app.LogMessage("OnClosed called for " + prop.GetFullName(), siVerboseMsg);
      }
      break;
    }
    case PPGEventContext::siButtonClicked:
    {
      // If there are multiple buttons 
      // we can use this attribute to figure out which one was clicked.
      CValue buttonPressed = ctxt.GetAttribute("Button");

      // Get all inspected CustomProperty objects
      CRefArray props = ctxt.GetInspectedObjects();

      for (LONG i = 0; i < props.GetCount(); i++)
      {
        CustomProperty prop(props[i]);
        CParameterRefArray params = prop.GetParameters();
        if (buttonPressed.GetAsText().IsEqualNoCase(kWrite))
        {
          CValueArray args;
          args.Add(params.GetValue(kFolder));
          args.Add(params.GetValue(kFilename));
          args.Add(params.GetValue(kStartFrame));
          args.Add(params.GetValue(kEndFrame));

          app.ExecuteCommand("UsdWrite", args, CValue());
        }

      }
      break;
    }
    case PPGEventContext::siTabChange:
    {
      // Retrieve the label of the tab that is now active
      CValue tabLabel = ctxt.GetAttribute("Tab");

      // Get all inspected CustomProperty objects
      CRefArray props = ctxt.GetInspectedObjects();

      for (LONG i = 0; i < props.GetCount(); i++)
      {
        CustomProperty prop(props[i]);
        app.LogMessage("Tab changed to: " + tabLabel.GetAsText() + CString(" while inspecting ") + prop.GetFullName());
      }
      break;
    }
  }

  if(needRefresh)ctxt.PutAttribute("Refresh", true);

  return CStatus::OK;
}

