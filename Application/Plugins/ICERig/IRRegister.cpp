/*----------------------------------------------
	
	ICERig Register.cpp
                  _   _
                 ( \ / )
                __\ Y /,-')
               (__     .-'
                  |   (
                  [___]
                  |oo |
                ,' \  |
               <___/  |
                  |   |
                  |   |
                  |   |
                  |   |
              _,-/_._  \,_
        _.-"^`  //   \    `^"-.,__
        \     ,//     \          /
         `\,-":;       ;  \-.,_/'
              ||       |   ;
              ||       ;   |
              :\      /    ;
               \`----'    /
                `._____.-'
                  | | |
                __| | |__
          jgs  /    |    \
               `""""`""""`

--------------------------------------------*/
// ---------------------------------------------------------------------------
// Delta Mush Operator from Guillaume Barate
// https://vimeo.com/122981239
// ---------------------------------------------------------------------------

#include "IRRegister.h"
#include "IRDeltaMush.h"
#include "IRCubicBezier.h"


XSI::CStatus RegisterIRSmoothWeights( XSI::PluginRegistrar& in_reg );
XSI::CStatus RegisterIRCubicBezier( XSI::PluginRegistrar& in_reg );
XSI::CStatus RegisterIRCubicBezierGenerator(XSI::PluginRegistrar& in_reg);
XSI::CStatus RegisterIRBuildElementID( XSI::PluginRegistrar& in_reg );
XSI::CStatus RegisterIRDeltaMushInit(XSI::PluginRegistrar& in_reg);
XSI::CStatus RegisterIRDeltaMush(XSI::PluginRegistrar& in_reg);
XSI::CStatus RegisterIRResampleCurve(XSI::PluginRegistrar& in_reg);

SICALLBACK XSILoadPlugin( PluginRegistrar& in_reg )
{
	in_reg.PutAuthor(L"benmalartre");
	in_reg.PutName(L"ICERigNodes");
	in_reg.PutVersion(1,0);

	RegisterIRSmoothWeights( in_reg );
	RegisterIRCubicBezier(in_reg);
	RegisterIRCubicBezierGenerator(in_reg);
	RegisterIRBuildElementID(in_reg);
	RegisterIRDeltaMushInit(in_reg);
	RegisterIRDeltaMush(in_reg);
	RegisterIRResampleCurve(in_reg);
	return CStatus::OK;
}

SICALLBACK XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
	CString strPluginName;
	strPluginName = in_reg.GetName();
	Application().LogMessage(strPluginName + L" has been unloaded.",siVerboseMsg);
	return CStatus::OK;
}