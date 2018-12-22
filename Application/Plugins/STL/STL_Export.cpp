#include "STL_Register.h"

void ConvertCVector3ArrayToFlatArrayOfFloat(float*& out_pos, CVector3Array& in_pos)
{
	int nb = in_pos.GetCount();
	for(int i=0;i<nb;i++)
	{
		out_pos[i*3] = (float)in_pos[i].GetX();
		out_pos[i*3+1] = (float)in_pos[i].GetY();
		out_pos[i*3+2] = (float)in_pos[i].GetZ();
	}
}

void ConvertPolygonDataArrayToTwoFlatArrayOfInt(int*& fnbv, int*& fv, CLongArray& facedatas, long nbfaces)
{
	int offset = 0;
	int nb;
	int i = 0;
	for(int a=0;a<nbfaces;a++)
	{
		nb = facedatas[offset];
		fnbv[a] = nb;
		for(int b=offset+1;b<offset+nb+1;b++,i++)
		{
			fv[i] = facedatas[b];
		}
		offset += nb+1;
	}
}

SICALLBACK STL_Export_Init( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	Command oCmd;
	oCmd = ctxt.GetSource();
	oCmd.PutDescription(L"");
	oCmd.EnableReturnValue(true);
    ArgumentArray args = oCmd.GetArguments();

    args.Add( L"inObject" );
    args.Add( L"inFolder" );

	return CStatus::OK;
}

SICALLBACK STL_Export_Execute( CRef& in_ctxt )
{
	Application app;
	Context ctxt( in_ctxt );

	// Access the arguments to the command
    CValueArray args = ctxt.GetAttribute( L"Arguments" );
    CValue inObject = args[0] ;
    CString inFolder = args[1] ;

	X3DObject oObj = (CRef)inObject ;
	Primitive oPrim = oObj.GetActivePrimitive();
	CTransformation oTra;
	CMatrix4 oMatrix;
	PolygonMesh oMesh;

	CPointRefArray aPoints;
	CVector3Array aPos;
	CVector3Array aVertices;
	CLongArray aPolygons;

	CString filepathstr = CUtils::BuildPath(inFolder, oObj.GetName()+L".stl");
	const char* filepath = filepathstr.GetAsciiString() ;

	STL_File Writer;

	oMesh = oPrim.GetGeometry(NULL,siConstructionModeAnimation);

	CTriangleRefArray triangles = oMesh.GetTriangles();
	Application().LogMessage(L"Nb Triangles : "+(CString)triangles.GetCount());
	//Writer._header = "stl from softimage";
	Writer._nbt = triangles.GetCount();

	Writer._normals.resize(Writer._nbt);
	Writer._vertices.resize(Writer._nbt*3);
	CVector3 n,a,b,c,ab,ac;
	Triangle t;
	CVector3Array pos;
	for(long i=0;i<Writer._nbt;i++){
		t = triangles[i];
		pos = t.GetPositionArray();
		a = pos[0];
		b = pos[1];
		c = pos[2];

		Writer._vertices[i*3+0].x = (float)a.GetX();
		Writer._vertices[i*3+0].y = (float)a.GetY();
		Writer._vertices[i*3+0].z = (float)a.GetZ();

		Writer._vertices[i*3+1].x = (float)b.GetX();
		Writer._vertices[i*3+1].y = (float)b.GetY();
		Writer._vertices[i*3+1].z = (float)b.GetZ();

		Writer._vertices[i*3+2].x = (float)c.GetX();
		Writer._vertices[i*3+2].y = (float)c.GetY();
		Writer._vertices[i*3+2].z = (float)c.GetZ();

		ab.Sub(b,a);
		ac.Sub(a,c);
		n.Cross(ab,ac);
		n.NormalizeInPlace();
		Writer._normals[i].x = (float)n.GetX();
		Writer._normals[i].y = (float)n.GetY();
		Writer._normals[i].z = (float)n.GetZ();
	}

	Writer.Write(filepath,true);
	/*
	CFacetRefArray facets = oMesh.GetFacets();

	for(long x=0;x<facets.GetCount();x++){
		Facet f = facets.GetItem(x);
		if(f.GetNbPoints() != 3){
			Application().LogMessage(L"[STL_Export] Geometry MUST be triangulated first ---> Aborted!!",XSI::siErrorMsg);
			return CStatus::Abort;
		}
	}

	oTra = oObj.GetKinematics().GetGlobal().GetTransform();
	oMatrix = oTra.GetMatrix4();

	oMesh.Get(aVertices,aPolygons);

	nbf = oMesh.GetFacets().GetCount();

	nbv = aVertices.GetCount();
	nbfv = aPolygons.GetCount()- nbf;

	nbt = oMesh.GetTriangles().GetCount();

	Application().LogMessage(L"Nb Faces : "+(CString)nbf);
	Application().LogMessage(L"Nb Vertices : "+(CString)nbv);
	Application().LogMessage(L"Nb Triangles : "+(CString)nbt);
	ConvertCVector3ArrayToFlatArrayOfFloat(frame->_pos, aVertices);
	ConvertPolygonDataArrayToTwoFlatArrayOfInt(frame->_fnbv, frame->_fv, aPolygons, nbf);

	Writer.AddFrame(frame);
	offset += frame->_size;

	Writer.Write(filepath);
	Writer.Delete();

	*/
	return CStatus::OK;
}

