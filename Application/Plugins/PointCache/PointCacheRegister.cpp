// PointCacheRegister.Cpp
//------------------------------------
#include "PointCache.h"
#include <xsi_pluginregistrar.h>

XSIPLUGINCALLBACK CStatus XSILoadPlugin( PluginRegistrar& in_reg ) 
{
	in_reg.PutAuthor(L"benmalartre");
	in_reg.PutName(L"PointCachePlugin");
	in_reg.PutEmail(L"benmalartre@hotmail.com");
	in_reg.PutURL(L"");
	in_reg.PutVersion(1,0);
	in_reg.RegisterCommand(L"PointCacheWriter",L"PointCacheWriter");
	in_reg.RegisterCommand(L"PointCacheSyflexCache",L"PointCacheSyflexCache");
	in_reg.RegisterCommand(L"CorrectSyflexCache",L"CorrectSyflexCache");

	in_reg.RegisterOperator(L"PointCacheOp");

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
	return( XSI::CStatus::OK );
}