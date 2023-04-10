// UsdExportPlugin
// this is a very simple xsi to usd exporter
#include "common.h"
#include "utils.h"
#include "mesh.h"
#include "scene.h"

// Plugin definitions
SICALLBACK XSILoadPlugin( PluginRegistrar& in_reg )
{
  in_reg.PutAuthor(L"benmalartre");
  in_reg.PutName(L"UsdWrite");
  in_reg.PutVersion(1,0);
  in_reg.RegisterProperty(L"UsdWriteUI");
  in_reg.RegisterCommand(L"UsdWrite",L"UsdWrite");
  in_reg.RegisterCommand(L"CreateUsdWriteUI", L"CreateUsdWriteUI");
  
  in_reg.RegisterMenu(siMenuMainFileExportID,L"UsdWrite_Menu",false,false);

  return CStatus::OK;
}

SICALLBACK XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
  CString strPluginName;
  strPluginName = in_reg.GetName();
  Application().LogMessage(strPluginName + L" has been unloaded.",siVerboseMsg);
  return CStatus::OK;
}


SICALLBACK UsdWrite_Init( CRef& in_ctxt )
{
  Context ctxt( in_ctxt );
  Command cmd;
  cmd = ctxt.GetSource();
  cmd.PutDescription(L"Export to Pixar Universal Scene Description file format");

  X2UTimeInfos infos;
 
  ArgumentArray args;
  args = cmd.GetArguments();
  args.Add(kFolder, "");
  args.Add(kFilename, "");
  args.Add(kTimeMode, 0L);
  args.Add(kStartFrame, 1.0);
  args.Add(kEndFrame, 100.0);
  args.Add(kSampleRate, 1.f);
  args.Add(kExportSelection, false);
  args.Add(kExportMeshes, true);
  args.Add(kExportUVs, true);
  args.Add(kExportColors, true);
  args.Add(kExportCurves, true);
  args.Add(kExportPoints, true);
  args.Add(kExportCameras, true);
  args.Add(kExportAttributes, false);
  args.Add(kExportAttributesList, CValue());

  return CStatus::OK;
}

SICALLBACK UsdWrite_Execute( CRef& in_ctxt )
{
  Context ctxt( in_ctxt );
  Command cmd(ctxt.GetSource());

  ArgumentArray args = cmd.GetArguments();
  CString folder = args.GetItem(kFolder).GetValue();
  CString filename = args.GetItem(kFilename).GetValue();
  LONG timeMode = args.GetItem(kTimeMode).GetValue();
  double startFrame = args.GetItem(kStartFrame).GetValue();
  double endFrame = args.GetItem(kEndFrame).GetValue();
  double sampleRate = args.GetItem(kSampleRate).GetValue();

  size_t options = 0;
 
  bool exportSelection = args.GetItem(kExportSelection).GetValue();
  bool exportMeshes = args.GetItem(kExportMeshes).GetValue();
  bool exportUVs = args.GetItem(kExportUVs).GetValue();
  bool exportColors = args.GetItem(kExportColors).GetValue();
  bool exportPoints = args.GetItem(kExportPoints).GetValue();
  bool exportCurves = args.GetItem(kExportCurves).GetValue();
  bool exportCameras = args.GetItem(kExportCameras).GetValue();
  bool exportAttributes = args.GetItem(kExportAttributes).GetValue();

  CString exportAttributesListStr = args.GetItem(kExportAttributesList).GetValue();
  CStringArray exportAttributesList = exportAttributesListStr.Split(";");
  
  LOG("------------------------------------------------------------------------");
  LOG("Export Options :");
  LOG("   Meshes    : " + CString(exportMeshes));
  LOG("   Cameras   : " + CString(exportCameras));
  LOG("   Points    : " + CString(exportPoints));
  LOG("   Curves    : " + CString(exportCurves));
  LOG("   Attribute : " + CString(exportAttributes));
  if (exportAttributes) {
    for (size_t i = 0; i < exportAttributesList.GetCount(); ++i) {
      LOG("     |___" + exportAttributesList[i]);
    }
  }
  LOG("------------------------------------------------------------------------");

  if (exportSelection) options += X2U_EXPORT_SELECTION;
  if (exportMeshes) options += X2U_EXPORT_MESHES;
  if (exportUVs) options += X2U_EXPORT_UVS;
  if (exportColors) options += X2U_EXPORT_COLORS;
  if (exportCameras) options += X2U_EXPORT_CAMERAS;
  if (exportAttributes) options += X2U_EXPORT_ATTRIBUTES;

  Application app;
  Model root = app.GetActiveSceneRoot();

  // ensure folder exists
  if (X2UCheckFolder(folder) && X2UCheckFilename(filename))
  {
    X2UScene exportScene(
      (std::string)folder.GetAsciiString(),
      (std::string) filename.GetAsciiString(),
      root.GetRef()
    );
    SetCurrentScene(&exportScene);
    
    exportScene.Init();
    if (timeMode == TimeMode::SCENE) {
      exportScene.SetTimeInfosFromScene(sampleRate);
    }
    else {
      exportScene.SetTimeInfos(startFrame, endFrame, sampleRate);
    }
    exportScene.SetOptions(options);
    exportScene.SetAttributes(exportAttributesList);
    exportScene.Process();
    exportScene.Save();
  }

  // Return a value by setting this attribute:
  ctxt.PutAttribute( L"ReturnValue", true );

  // Return CStatus::Fail if you want to raise a script error
  return CStatus::OK;
}

SICALLBACK CreateUsdWriteUI_Init(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  Command oCmd;
  oCmd = ctxt.GetSource();
  oCmd.PutDescription(L"");
  oCmd.EnableReturnValue(false);

  return CStatus::OK;
}

SICALLBACK CreateUsdWriteUI_Execute(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  CValueArray args = ctxt.GetAttribute(L"Arguments");

  Application app;
  Model root = app.GetActiveSceneRoot();
  CustomProperty prop;
  root.GetPropertyFromName(L"UsdWriteUI", prop);
  if(!prop.IsValid())
    prop = root.AddProperty(L"UsdWriteUI", false);
  CValueArray toInspect;
  toInspect.Add(CValue(prop));
  app.ExecuteCommand(L"InspectObj", toInspect, CValue());
  return CStatus::OK;
}

SICALLBACK UsdWrite_Menu_Init( CRef& in_ctxt )
{
  Context ctxt( in_ctxt );
  Menu oMenu;
  oMenu = ctxt.GetSource();
  MenuItem oNewItem;
  oMenu.AddCommandItem(L"UsdWrite",L"CreateUsdWriteUI",oNewItem);
  return CStatus::OK;
}

