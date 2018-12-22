#include "sRegister.h"

SICALLBACK bbppGeoCacheImport_Init( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	Command oCmd;
	oCmd = ctxt.GetSource();
	oCmd.PutDescription(L"");
	oCmd.EnableReturnValue(true);
    ArgumentArray args = oCmd.GetArguments();

    args.Add( L"inFile" );
	args.Add( L"inFrame" );

	return CStatus::OK;
}

SICALLBACK bbppGeoCacheImport_Execute( CRef& in_ctxt )
{
	Application app;
	Context ctxt( in_ctxt );

	// Access the arguments to the command
    CValueArray args = ctxt.GetAttribute( L"Arguments" );

    CString inFile = args[0] ;
	CValue inFrame = args[1];

	Model root = app.GetActiveSceneRoot();

	const char* filepath = inFile.GetAsciiString() ;
	/*
	CStringArray filenamearray = inFile.Split("//");
	CString filename = filenamearray[filenamearray.GetCount()-1];
	app.LogMessage(L"FileName ---> "+filename);
	const char* filenamechar = filename.GetAsciiString();
	*/

	bbppGeoCache Reader;

	if(Reader.OpenFile(filepath))
	{
		if(!Reader.GetHeader())
		{
			Application().LogMessage(L"File Signature Not Valid ---> Import aborted!!"); 
			return CStatus::InvalidArgument;
		}
		Reader.ReadFrame((int)inFrame);
	}

	CVector3Array vertices;
	CLongArray polygons;

	for(int i=0;i<Reader._nbv;i++)
	{
		vertices.Add(CVector3(Reader._vertices[i*3],Reader._vertices[i*3+1],Reader._vertices[i*3+2]));
	}

	int offset = 0;
	for(int i=0;i<Reader._nbf;i++)
	{
		polygons.Add(Reader._faceverticescount[i]);
		for(int j=0;j<Reader._faceverticescount[i];j++)
		{
			polygons.Add(Reader._facevertices[offset+j]);
		}
		offset+=Reader._faceverticescount[i];
	}

	X3DObject oObj;
	CStatus check = root.AddPolygonMesh(vertices,polygons,L"Transformation",oObj);
	assert( check == CStatus::OK );

	ctxt.PutAttribute( L"ReturnValue", oObj.GetRef());

	return CStatus::OK;
}

SICALLBACK bbppApplyGeoCache_Init( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	Command oCmd;
	oCmd = ctxt.GetSource();
	oCmd.PutDescription(L"");
	oCmd.EnableReturnValue(true);
    ArgumentArray args = oCmd.GetArguments();

    args.Add( L"inObject" );

	return CStatus::OK;
}

SICALLBACK bbppApplyGeoCache_Execute( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	Application().LogMessage(L"bbppApplyGeoCache called",siVerboseMsg);

	Selection l_pSelection = Application().GetSelection();
	X3DObject inputMesh(l_pSelection.GetItem(0));

	CRef ioMesh = inputMesh.GetActivePrimitive().GetRef();

	// create the operator
	CustomOperator newOp = Application().GetFactory().CreateObject(L"bbppGeoCache");
	newOp.AddIOPort(ioMesh);
	newOp.Connect();
	ctxt.PutAttribute( L"ReturnValue", newOp.GetRef() );

	return CStatus::OK;
}