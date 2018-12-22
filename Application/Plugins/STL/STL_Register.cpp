#include "STL_Register.h"
//XSI::CStatus RegisterSTLReader( XSI::PluginRegistrar& in_reg );

SICALLBACK XSILoadPlugin( PluginRegistrar& in_reg )
{
	in_reg.PutAuthor(L"benmalartre");
	in_reg.PutName(L"STL_IO_Plugin");
	in_reg.PutVersion(1,0);
	in_reg.RegisterCommand(L"STL_Export",L"STL_Export");
	in_reg.RegisterCommand(L"STL_Import",L"STL_Import");

	//RegisterSTLReader(in_reg);
	return CStatus::OK;
}

SICALLBACK XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
	CString strPluginName;
	strPluginName = in_reg.GetName();
	Application().LogMessage(strPluginName + L" has been unloaded.",siVerboseMsg);
	return CStatus::OK;
}