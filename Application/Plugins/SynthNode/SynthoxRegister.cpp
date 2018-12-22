// Synthox Plugin
// an attempt to generate sounds from inside xsi
// using STK
//------------------------------------------------------------

#include "SynthoxRegister.h"

XSI::CStatus RegisterSynthoxInstrument( XSI::PluginRegistrar& in_reg );
XSI::CStatus RegisterSynthoxVoicer( XSI::PluginRegistrar& in_reg );
XSI::CStatus RegisterSynthoxReader( XSI::PluginRegistrar& in_reg );
XSI::CStatus RegisterSynthoxPlayer( XSI::PluginRegistrar& in_reg );
//XSI::CStatus RegisterbSineNode( XSI::PluginRegistrar& in_reg );
//XSI::CStatus RegisterbRagaNode( XSI::PluginRegistrar& in_reg );

using namespace XSI; 

SICALLBACK XSILoadPlugin( PluginRegistrar& in_reg )
{
	in_reg.PutAuthor(L"benmalartre");
	in_reg.PutName(L"SynthoxPlugin");
	in_reg.PutVersion(1,0);

	RegisterSynthoxInstrument( in_reg );
	RegisterSynthoxVoicer( in_reg );
	RegisterSynthoxReader(in_reg);
	RegisterSynthoxPlayer(in_reg);
	//RegisterbSineNode( in_reg);
	//RegisterbRagaNode(in_reg);

	return CStatus::OK;
}

SICALLBACK XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
	CString strPluginName;
	strPluginName = in_reg.GetName();
	Application().LogMessage(strPluginName + L" has been unloaded.",siVerboseMsg);

	return CStatus::OK;
}

