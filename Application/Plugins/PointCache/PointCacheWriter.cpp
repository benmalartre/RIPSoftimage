// PointCacheMaker.Cpp
//------------------------------------
#include "PointCache.h"

bool FileExists(CString);

XSIPLUGINCALLBACK CStatus PointCacheWriter_Init( CRef& in_ctxt )
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
	args.Add( L"inFormat");
	args.Add( L"inRate");

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus PointCacheWriter_Execute( CRef& in_ctxt )
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
	CValue inFormat = args[5];
	CValue inRate = args[6];

	X3DObject oObj = (CRef)inObject ;
	Primitive oPrim = oObj.GetActivePrimitive();
	CTransformation oTra;
	CMatrix4 oMatrix;
	std::vector<CMatrix4> oMatrices;
	Geometry oGeom;
	CVector3Array oPosArray;

	long format = inFormat;
	CString filepathstr;

	if(format == 0)filepathstr = CUtils::BuildPath(inFolder, oObj.GetName()+L".bpc");
	else if(format == 1)filepathstr = CUtils::BuildPath(inFolder, oObj.GetName()+L".pc2");
	else if(format == 2)filepathstr = CUtils::BuildPath(inFolder, oObj.GetName()+L".bkd");

	const char* filepath = filepathstr.GetAsciiString() ;
	std::fstream out_file(filepath,std::ios::out|std::ios::binary|std::ios::trunc); 
	
	int nbPoints = oPrim.GetGeometry().GetPoints().GetCount();
	int rate = inRate;
	if (rate == 0)rate = 1;
	int startFrame = inStartFrame;
	int endFrame = inEndFrame;
	int clampedEndFrame = startFrame+(endFrame-startFrame)/rate;
	int timeLapse = clampedEndFrame - startFrame +1;
	double* position = new double[3*timeLapse*nbPoints];
	bool coord = inCoord;
	int id = 0;

	bool useYup = true;

	UIToolkit ToolKit = Application().GetUIToolkit();
	ProgressBar ProgressBar = ToolKit.GetProgressBar();

	ProgressBar.PutMaximum( timeLapse);
	ProgressBar.PutStep( 1 );
	ProgressBar.PutVisible( true );
	ProgressBar.PutCaption( L"PointCache ---> Write cache for "+ oObj.GetFullName());

	for(int a=startFrame;a<endFrame+1;a+=rate)
	{
		oGeom = oPrim.GetGeometry(a,siConstructionModeAnimation);
		oTra = oObj.GetKinematics().GetGlobal().GetTransform(a);
		oMatrix = oTra.GetMatrix4();
		oMatrices.push_back(oMatrix);

		int offset = 0;

		oPosArray = oGeom.GetPoints().GetPositionArray();
		for(int b=0;b<nbPoints;b++)
		{
			offset = 3*id*nbPoints;
			if(coord == true)oPosArray[b] = MapObjectPositionToWorldSpace(oTra, oPosArray[b]);
			
			if(useYup == true)
			{
				position[ offset+ b*3 + 0] = oPosArray[b].GetX();
				position[ offset + b*3 + 1] = oPosArray[b].GetY();
				position[ offset + b*3 + 2] = oPosArray[b].GetZ();
			}
			else
			{
				position[offset + b*3 + 0] = oPosArray[b].GetX();
				position[offset + b*3 + 1] = oPosArray[b].GetZ() * -1.0;
				position[offset + b*3 + 2] = oPosArray[b].GetY();
			}
		}
		id ++;
		ProgressBar.Increment();
		if(ProgressBar.IsCancelPressed())
		{
			out_file.close();
			return CStatus::Abort;
		}
	}
	ProgressBar.PutVisible(false);

	// bpc file format
	if(format == 0)
	{
		out_file.write((char*) &nbPoints, sizeof(long));
		out_file.write((char*) &startFrame, sizeof(long));
		out_file.write((char*) &clampedEndFrame, sizeof(long));
		out_file.write((char*) position, sizeof(double)* 3 *timeLapse*nbPoints);
	}
	
	// pc2 file format
	else if(format == 1)
	{
		char signature[12] = "POINTCACHE2";
		int version = 1;
		float start = (float)startFrame;
		float sampleRate = (float)rate;
		int numSamples = (int)timeLapse;

		out_file.write((char*) &signature, 12 * sizeof(char));
		out_file.write((char*) &version, sizeof(int));
		out_file.write((char*) &nbPoints, sizeof(int));
		out_file.write((char*) &start, sizeof(float));
		out_file.write((char*) &sampleRate, sizeof(float));
		out_file.write((char*) &numSamples, sizeof(int));
		out_file.write((char*) position, sizeof(double)* 3 *timeLapse*nbPoints);
	}

	// pk file format
	else if(format == 2)
	{
		int signature[10];
		signature[0] = 1967;					// Magic number
		signature[1] = 4;						// Version
		signature[2] = 0;						// Is SRT
		signature[3] = startFrame;				// StartFrame
		signature[4] = clampedEndFrame;			// EndFrame

		for(int a=5;a<10;a++)signature[a] = 0;

		out_file.write((char*)& signature,sizeof(int)*10);

		double* pos = position;

		for(int b=0;b<timeLapse;b++)
		{
			int t = b+startFrame;
			out_file.write((char*)& t,sizeof(int));
			out_file.write((char*)& nbPoints,sizeof(int));
			out_file.write((char*)& oMatrices[b],sizeof(CMatrix4));
			out_file.write((char*) pos,sizeof(double)*3*nbPoints);
			pos+=3*nbPoints;
		}	
	}

	out_file.close();

	delete[]position;

	return CStatus::OK;
}

bool FileExists(CString inFilename) 
{
  struct stat stFileInfo;
  bool blnReturn = false;
  int intStat;

  // Attempt to get the file attributes
  intStat = stat(inFilename.GetAsciiString(),&stFileInfo);
  if(intStat == 0) 
  {
    // the file exist ---> ask the user if he wants to overwrite
	UIToolkit ToolKit = Application().GetUIToolkit();
	CString message = inFilename +L" already exists !! Do you want to overwrite ?";
	CString title = L"WritePointCache";
	long out;
	ToolKit.MsgBox (message, siMsgYesNo, title, out);
	
    if(out == 7)blnReturn = true;
  } 
  return(blnReturn);
}
