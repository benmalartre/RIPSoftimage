#include "sRegister.h"

SICALLBACK XSILoadPlugin( PluginRegistrar& in_reg )
{
	in_reg.PutAuthor(L"benmalartre");
	in_reg.PutName(L"BarbatrucPlugin");
	in_reg.PutVersion(1,0);
	in_reg.RegisterCommand(L"bbppGeoCacheRecord",L"bbppGeoCacheRecord");
	in_reg.RegisterCommand(L"bbppGeoCacheImport",L"bbppGeoCacheImport");
	in_reg.RegisterCommand(L"bbppApplyGeoCache",L"bbppApplyGeoCache");
	in_reg.RegisterOperator(L"bbppGeoCache");

	return CStatus::OK;
}

SICALLBACK XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
	CString strPluginName;
	strPluginName = in_reg.GetName();
	Application().LogMessage(strPluginName + L" has been unloaded.",siVerboseMsg);
	return CStatus::OK;
}