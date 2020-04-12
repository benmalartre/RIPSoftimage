// UsdExportPlugin
// this is a very simple xsi to usd exporter
#include "common.h"
#include "mesh.h"
#include "scene.h"

// Plugin definitions
SICALLBACK XSILoadPlugin( PluginRegistrar& in_reg )
{
  in_reg.PutAuthor(L"benmalartre");
  in_reg.PutName(L"UsdExportPlugin");
  in_reg.PutVersion(1,0);
  in_reg.RegisterProperty(L"UsdExportUI");
  in_reg.RegisterCommand(L"UsdExport",L"UsdExport");
  in_reg.RegisterCommand(L"CreateUsdExportUI", L"CreateUsdExportUI");
  
  in_reg.RegisterMenu(siMenuMainFileExportID,L"UsdExport_Menu",false,false);

  return CStatus::OK;
}

SICALLBACK XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
  CString strPluginName;
  strPluginName = in_reg.GetName();
  Application().LogMessage(strPluginName + L" has been unloaded.",siVerboseMsg);
  return CStatus::OK;
}


SICALLBACK UsdExport_Init( CRef& in_ctxt )
{
  Context ctxt( in_ctxt );
  Command oCmd;
  oCmd = ctxt.GetSource();
  oCmd.PutDescription(L"");
  oCmd.EnableReturnValue(true);

  return CStatus::OK;
}

SICALLBACK UsdExport_Execute( CRef& in_ctxt )
{
  Context ctxt( in_ctxt );
  CValueArray args = ctxt.GetAttribute(L"Arguments");

  Application app;
  Model root = app.GetActiveSceneRoot();

  std::string folder = "E:/Projects/RnD/USD_BUILD/assets";

  std::string filename = "Test.usda";

  X2UExportScene exportScene(folder, filename, root.GetRef());
  exportScene.Init();
  exportScene.Process();
  exportScene.Save();

  // Return a value by setting this attribute:
  ctxt.PutAttribute( L"ReturnValue", true );

  // Return CStatus::Fail if you want to raise a script error
  return CStatus::OK;
}

SICALLBACK CreateUsdExportUI_Init(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  Command oCmd;
  oCmd = ctxt.GetSource();
  oCmd.PutDescription(L"");
  oCmd.EnableReturnValue(false);

  return CStatus::OK;
}

SICALLBACK CreateUsdExportUI_Execute(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  CValueArray args = ctxt.GetAttribute(L"Arguments");

  Application app;
  Model root = app.GetActiveSceneRoot();
  CustomProperty prop;
  root.GetPropertyFromName(L"UsdExportUI", prop);
  if(!prop.IsValid())
    prop = root.AddProperty(L"UsdExportUI", false);
  CValueArray toInspect;
  toInspect.Add(CValue(prop));
  app.ExecuteCommand(L"InspectObj", toInspect, CValue());
  return CStatus::OK;
}

SICALLBACK UsdExport_Menu_Init( CRef& in_ctxt )
{
  Context ctxt( in_ctxt );
  Menu oMenu;
  oMenu = ctxt.GetSource();
  MenuItem oNewItem;
  oMenu.AddCommandItem(L"UsdExport",L"CreateUsdExportUI",oNewItem);
  return CStatus::OK;
}

