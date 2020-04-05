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
  in_reg.RegisterCommand(L"UsdExport",L"UsdExport");
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

  LOG(L"UsdExport_Execute called",siVerboseMsg);

  Application app;
 
  X2UExportScene exportScene;

  std::string folder = "E:/Projects/Softimage/XSIWorkgroup/Application/Plugins/UsdExport";
  std::string filename = "Test.usda";

  exportScene.Init(folder, filename, app.GetActiveSceneRoot().GetRef());
  exportScene.Process();
  exportScene.Save();

  // Return a value by setting this attribute:
  ctxt.PutAttribute( L"ReturnValue", true );

  // Return CStatus::Fail if you want to raise a script error
  return CStatus::OK;
}

SICALLBACK UsdExport_Menu_Init( CRef& in_ctxt )
{
  Context ctxt( in_ctxt );
  Menu oMenu;
  oMenu = ctxt.GetSource();
  MenuItem oNewItem;
  oMenu.AddCommandItem(L"UsdExport",L"UsdExport",oNewItem);
  return CStatus::OK;
}

