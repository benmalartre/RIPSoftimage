#include "STL_Register.h"

SICALLBACK STL_Import_Init( CRef& in_ctxt )
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

SICALLBACK STL_Import_Execute( CRef& in_ctxt )
{
	Application app;
	Context ctxt( in_ctxt );

	// Access the arguments to the command
    CValueArray args = ctxt.GetAttribute( L"Arguments" );

    CString inFile = args[0] ;
	CValue inFrame = args[1];

	if(inFile == ""){
		Application().LogMessage(L"[STL Import] File Name Empty ---> Aborted!!!",XSI::siErrorMsg);
		return CStatus::Abort;
	}
	Model root = app.GetActiveSceneRoot();

	const char* filepath = inFile.GetAsciiString() ;


	STL_File Reader;

	if(Reader.Open(filepath))
	{

		if(!Reader.GetHeader())
		{
			Application().LogMessage(L"File Signature Not Valid ---> Import aborted!!"); 
			return CStatus::InvalidArgument;
		}
		else{
			//Application().LogMessage((CString)Reader._header);
			//Application().LogMessage(L"Nb Triangles : "+(CString)Reader._nbt);
			Reader.Read();
			
			
			CString n;
			CVector3 norm,a,b,c;

			CVector3Array vertices;
			CLongArray polygons;

			for(long i=0;i<Reader._nbt;i++){
				norm.Set(Reader._normals[i].x,Reader._normals[i].y,Reader._normals[i].z);
				a.Set(Reader._vertices[i*3].x,Reader._vertices[i*3].y,Reader._vertices[i*3].z);
				b.Set(Reader._vertices[i*3+1].x,Reader._vertices[i*3+1].y,Reader._vertices[i*3+1].z);
				c.Set(Reader._vertices[i*3+2].x,Reader._vertices[i*3+2].y,Reader._vertices[i*3+2].z);

				vertices.Add(a);
				vertices.Add(b);
				vertices.Add(c);

				polygons.Add(3);
				polygons.Add(i*3);
				polygons.Add(i*3+1);
				polygons.Add(i*3+2);
			}

			X3DObject oObj;
			CStatus check = root.AddPolygonMesh(vertices,polygons,L"STL",oObj);
			assert( check == CStatus::OK );

			ctxt.PutAttribute( L"ReturnValue", oObj.GetRef());
			
			
		}
	}
	
	return CStatus::OK;
}

SICALLBACK STL_Apply_Reader_Init( CRef& in_ctxt )
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

SICALLBACK STL_Apply_Reader_Execute( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	Application().LogMessage(L"STL_Apply_Reader Called",siVerboseMsg);

	Selection l_pSelection = Application().GetSelection();
	X3DObject inputMesh(l_pSelection.GetItem(0));

	CRef ioMesh = inputMesh.GetActivePrimitive().GetRef();

	// create the operator
	CustomOperator newOp = Application().GetFactory().CreateObject(L"STL_Reader");
	newOp.AddIOPort(ioMesh);
	newOp.Connect();
	ctxt.PutAttribute( L"ReturnValue", newOp.GetRef() );

	return CStatus::OK;
}