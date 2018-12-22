#include "sRegister.h"

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

SICALLBACK bbppGeoCacheRecord_Init( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	Command oCmd;
	oCmd = ctxt.GetSource();
	oCmd.PutDescription(L"");
	oCmd.EnableReturnValue(true);
    ArgumentArray args = oCmd.GetArguments();

    args.Add( L"inObject" );
    args.Add( L"inFolder" );
	args.Add( L"inStartFrame" );
	args.Add( L"inEndFrame" );
	args.Add( L"inCoord");

	return CStatus::OK;
}

SICALLBACK bbppGeoCacheRecord_Execute( CRef& in_ctxt )
{
	Application app;
	Context ctxt( in_ctxt );

	// Access the arguments to the command
    CValueArray args = ctxt.GetAttribute( L"Arguments" );
    CValue inObject = args[0] ;
    CString inFolder = args[1] ;
	CValue inStartFrame = args[2];
	CValue inEndFrame = args[3];
	CValue inCoord = args[4];

	X3DObject oObj = (CRef)inObject ;
	Primitive oPrim = oObj.GetActivePrimitive();
	CTransformation oTra;
	CMatrix4 oMatrix;
	PolygonMesh oMesh;

	CPointRefArray aPoints;
	CVector3Array aPos;
	CVector3Array aVertices;
	CLongArray aPolygons;
	int nbv, nbf, nbfv;

	CString filepathstr = CUtils::BuildPath(inFolder, oObj.GetName()+L".bgc");
	const char* filepath = filepathstr.GetAsciiString() ;

	bbppGeoCache Writer;
	
	int startframe = (int)inStartFrame;
	int endframe = (int)inEndFrame;
	int timelapse = endframe - startframe +1;

	Writer.SetStartFrame(startframe);
	Writer.SetEndFrame(endframe);
	Writer.SetTimelapse(timelapse);

	Writer.CreateOffset();

	UIToolkit ToolKit = Application().GetUIToolkit();
	ProgressBar ProgressBar = ToolKit.GetProgressBar();

	ProgressBar.PutMaximum( timelapse);
	ProgressBar.PutStep( 1 );
	ProgressBar.PutVisible( true );
	ProgressBar.PutCaption( L"bbppGeoCacheRecord ---> Write Cache for "+ oObj.GetFullName());

	int frameid = 0;	
	int offset = SIZE_SIGNATURE + SIZE_INT*(2+timelapse);
	CValue outArg;
   CValueArray inArgs; 

   //app.ExecuteCommand( L"NewScene", inArgs, outArg );
	Application().ExecuteCommand(L"FirstFrame",inArgs,outArg);
	for(int a=startframe;a<endframe+1;a++,frameid++)
	{
		Writer.SetOffset(frameid,offset);

		oMesh = oPrim.GetGeometry(a,siConstructionModeAnimation);
		oTra = oObj.GetKinematics().GetGlobal().GetTransform(a);
		oMatrix = oTra.GetMatrix4();

		oMesh.Get(aVertices,aPolygons);
		Application().LogMessage(L"Nb Vertices ---> "+(CString)aVertices.GetCount());
		nbf = oMesh.GetFacets().GetCount();
		nbv = aVertices.GetCount();
		nbfv = aPolygons.GetCount()- nbf;

		bbppGeoFrame* frame = new bbppGeoFrame(nbv,nbf,nbfv);

		ConvertCVector3ArrayToFlatArrayOfFloat(frame->_pos, aVertices);
		ConvertPolygonDataArrayToTwoFlatArrayOfInt(frame->_fnbv, frame->_fv, aPolygons, nbf);

		Writer.AddFrame(frame);
		offset += frame->_size;

		ProgressBar.Increment();
		if(ProgressBar.IsCancelPressed())
		{
			return CStatus::Abort;
		}
		Application().ExecuteCommand(L"NextFrame",inArgs,outArg);
		Application().ExecuteCommand(L"Refresh",inArgs,outArg);
	}

	Writer.Write(filepath);
	Writer.Delete();

	ProgressBar.PutVisible(false);

	return CStatus::OK;
}

