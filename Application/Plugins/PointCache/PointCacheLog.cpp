// PointCacheLog.Cpp
//------------------------------------
#include "PointCache.h"


XSIPLUGINCALLBACK CStatus PointCacheLog_Init( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	Command oCmd;
	oCmd = ctxt.GetSource();
	oCmd.PutDescription(L"");
	oCmd.EnableReturnValue(true);
    ArgumentArray args = oCmd.GetArguments();

    args.Add( L"inFileName" );

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus PointCacheLog_Execute( CRef& in_ctxt )
{
	Application app;
	Context ctxt( in_ctxt );

	// Access the arguments to the command
    CValueArray args = ctxt.GetAttribute( L"Arguments" );
	CString inFileName = args[0];

	std::fstream f;
	const char* fileName = inFileName.GetAsciiString() ;
	struct stat results;
	if (stat(fileName, &results) == 0)
	{
		f.open(fileName,std::ios::binary|std::ios::in);

		long nbPoints, startFrame, endFrame, timelapse;
		f.read((char*) &nbPoints, sizeof(long));
		f.read((char*) &startFrame, sizeof(long));
		f.read((char*) &endFrame, sizeof(long));

		timelapse = endFrame - startFrame;
		float* position;
		long logFrame = startFrame;

		for(long cFrame = 0;cFrame<timelapse;cFrame++)
		{
			long id = cFrame * SIZE_POINT * nbPoints + START_CACHE;
			f.seekg(id,std::ios::beg);

			position = new float[3];
			f.read((char*) position, sizeof(float)* 3 );
			Application().LogMessage(L"Frame "+(CString)logFrame +L" ---> "+(CString)position[0]+L","+(CString)position[1]+L","+(CString)position[2]);

			logFrame++;
		}
	}

	return CStatus::OK;
}