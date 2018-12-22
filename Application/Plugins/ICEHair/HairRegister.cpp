/*----------------------------------------------
	
	Hair Register.cpp
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
#include "HairRegister.h"
#include "HairGeometry.h"

XSI::CStatus RegisterHairGeometryChecker( XSI::PluginRegistrar& in_reg );
XSI::CStatus RegisterHairMeshData( XSI::PluginRegistrar& in_reg );
XSI::CStatus RegisterHairUpdateMeshData( XSI::PluginRegistrar& in_reg );
XSI::CStatus RegisterHairSkeletonRoot( XSI::PluginRegistrar& in_reg );
XSI::CStatus RegisterHairSkeletonPoints( XSI::PluginRegistrar& in_reg );
XSI::CStatus RegisterHairGuideTriangle( XSI::PluginRegistrar& in_reg );
XSI::CStatus RegisterHairTriangleAreaSum( XSI::PluginRegistrar& in_reg );
XSI::CStatus RegisterHairStrandFitting( XSI::PluginRegistrar& in_reg );

SICALLBACK XSILoadPlugin( PluginRegistrar& in_reg )
{
	in_reg.PutAuthor(L"benmalartre");
	in_reg.PutName(L"Hair Plugin");
	in_reg.PutVersion(1,0);

	RegisterHairGeometryChecker(in_reg);
	RegisterHairMeshData(in_reg);
	RegisterHairUpdateMeshData(in_reg);
	RegisterHairSkeletonRoot(in_reg);
	RegisterHairSkeletonPoints(in_reg);
	RegisterHairGuideTriangle(in_reg);
	RegisterHairTriangleAreaSum(in_reg);
	RegisterHairStrandFitting(in_reg);

	return CStatus::OK;
}

SICALLBACK XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
	CString strPluginName;
	strPluginName = in_reg.GetName();
	Application().LogMessage(strPluginName + L" has been unloaded.",siVerboseMsg);
	return CStatus::OK;
}

