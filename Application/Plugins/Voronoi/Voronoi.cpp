// Voronoi.cpp
//-------------------------------------------
#include  "Voronoi.h"

SICALLBACK XSILoadPlugin( PluginRegistrar& in_reg )
{
	in_reg.PutAuthor(L"benmalartre");
	in_reg.PutName(L"VoronoiPlugin");
	in_reg.PutEmail(L"");
	in_reg.PutURL(L"");
	in_reg.PutVersion(1,0);
	in_reg.RegisterOperator(L"Voronoi");
	in_reg.RegisterCommand(L"ApplyVoronoi",L"ApplyVoronoi");

	return CStatus::OK;
}

SICALLBACK XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
	CString strPluginName;
	strPluginName = in_reg.GetName();
	Application().LogMessage(strPluginName + L" has been unloaded.",siVerboseMsg);
	return CStatus::OK;
}

SICALLBACK Voronoi_Define( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	CustomOperator oCustomOperator;
	Parameter oParam;
	CRef oPDef;
	Factory oFactory = Application().GetFactory();
	oCustomOperator = ctxt.GetSource();
	oPDef = oFactory.CreateParamDef(L"Offset",CValue::siDouble,siPersistable|siAnimatable,L"Offset",L"Offset",.01,0,10000,0,1);
	oCustomOperator.AddParameter(oPDef,oParam);

	oCustomOperator.PutAlwaysEvaluate(false);
	oCustomOperator.PutDebug(0);
	
	return CStatus::OK;
}

//////////////////////////////////////////////////////////////////////////////
// Entry Points
//////////////////////////////////////////////////////////////////////////////
SICALLBACK Voronoi_Init( CRef& in_ctxt )
{
	Context ctx(in_ctxt) ;	
	CVoronoi* pThis = new CVoronoi();	
	ctx.PutUserData( (CValue::siPtrType)pThis );
	return CStatus::OK;
}

SICALLBACK Voronoi_Term( CRef& in_ctxt )
{
	Context ctx(in_ctxt) ;
	CValue::siPtrType pUserData = ctx.GetUserData();
	CVoronoi* pThis = (CVoronoi*)pUserData;	
	delete pThis;
	return CStatus::OK;
}

SICALLBACK Voronoi_Update( CRef& in_ctxt )
{
	OperatorContext ctx(in_ctxt) ;
	CValue::siPtrType pUserData = ctx.GetUserData();
	CVoronoi* pThis = (CVoronoi*)pUserData;	
	return pThis->Update(ctx);
}