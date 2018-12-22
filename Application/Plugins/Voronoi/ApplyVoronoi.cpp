// ApplyVoronoi.cpp
//-------------------------------------------
#include  "Voronoi.h"

SICALLBACK ApplyVoronoi_Init( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	Command oCmd;
	oCmd = ctxt.GetSource();
	oCmd.PutDescription(L"Create an instance of Voronoi operator");
	oCmd.SetFlag(siNoLogging,false);

	return CStatus::OK;
}

SICALLBACK ApplyVoronoi_Execute( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	CValueArray args = ctxt.GetAttribute(L"Arguments");
	Application app;
	Model root = app.GetActiveSceneRoot();

	CustomOperator newOp = app.GetFactory().CreateObject(L"Voronoi");

	Selection selection( app.GetSelection() );
	if(selection.GetCount() != 0)
	{
		X3DObject oMesh = selection.GetItem(0);
		X3DObject oCloud = selection.GetItem(1);
		newOp.AddIOPort(oMesh.GetActivePrimitive());
		newOp.AddInputPort(oCloud.GetActivePrimitive());
		newOp.Connect();
		return CStatus::OK;
	}
	else
	{
		return CStatus::False;
	}
}