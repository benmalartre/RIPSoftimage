// CorrectSyflexCache.Cpp
//------------------------------------
#include "PointCache.h"

XSIPLUGINCALLBACK CStatus CorrectSyflexCache_Init( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	Command oCmd;
	oCmd = ctxt.GetSource();
	oCmd.PutDescription(L"");
	oCmd.EnableReturnValue(true);
    ArgumentArray args = oCmd.GetArguments();

    args.Add( L"inSyflexMesh" );
    args.Add( L"inSyflexCache" );
	args.Add( L"inFrame");

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus CorrectSyflexCache_Execute( CRef& in_ctxt )
{
	Application app;
	Context ctxt( in_ctxt );

	// Access the arguments to the command
    CValueArray args = ctxt.GetAttribute( L"Arguments" );
	X3DObject inSyflexMesh = CRef(args[0]);
    CString inSyflexCache = args[1];
	long inFrame = args[2];

	const char* sylexCachePath = inSyflexCache.GetAsciiString() ;

	// if not exists abort..
	struct stat results;
	if (stat(sylexCachePath, &results) == 0)
	{
		//get the syflex cache file
		std::fstream syflexfile; 
		if(syflexfile.is_open())syflexfile.close();
		syflexfile.open(sylexCachePath,std::ios::binary|std::ios::in|std::ios::out);

		//Get the point position array for this frame
		Primitive oPrim(inSyflexMesh.GetActivePrimitive());
		Geometry oGeom = oPrim.GetGeometry(inFrame, siConstructionModeAnimation);
		CVector3Array oPosArray = oGeom.GetPoints().GetPositionArray();
		long nbp = oPosArray.GetCount();

		//Write the syflex cache file
		float* pos = new float[3*nbp];
		for(long a=0;a<nbp;a++)
		{
			pos[a*3] = (float)oPosArray[a].GetX();
			pos[a*3+1] = (float)oPosArray[a].GetY();
			pos[a*3+2] = (float)oPosArray[a].GetZ();
		}
		syflexfile.write((char*) pos,3*nbp*sizeof(float));
		syflexfile.close();
		delete[]pos;
		return CStatus::OK;
	}

	else
	{
		Application().LogMessage(L"The syflex cache frame doesn't exists ---> CorrectSyflexCache aborted!!");
		return CStatus::Fail;
	}
}
