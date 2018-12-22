// PointCacheSyflexCache.Cpp
//------------------------------------
#include "PointCache.h"

XSIPLUGINCALLBACK CStatus PointCacheSyflexCache_Init( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	Command oCmd;
	oCmd = ctxt.GetSource();
	oCmd.PutDescription(L"");
	oCmd.EnableReturnValue(true);
    ArgumentArray args = oCmd.GetArguments();

    args.Add( L"inFileName" );
    args.Add( L"outFolder" );
	args.Add( L"outFileName");
	args.Add( L"outFileFormat");

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus bPointCacheSyflexCache_Execute( CRef& in_ctxt )
{
	Application app;
	Context ctxt( in_ctxt );

	// Access the arguments to the command
    CValueArray args = ctxt.GetAttribute( L"Arguments" );
	CString inFileName = args[0];
    CString outFolder = args[1];
	CString outFileName = args[2];
	CString outFileFormat = args[3];

	CString startPath = L"";
	CString slashType = L"\\";
	CString startSlashType = L"\\\\";

	if(CUtils::IsLinuxOS())
	{
		slashType = L"//";
		startSlashType = L"////";
	}

	CStringArray inFileNameArray = inFileName.Split(slashType);
	if(inFileNameArray[0].Length() != 2)startPath = startSlashType;
	CString inFileFolder;
	for(long a=0;a<inFileNameArray.GetCount()-1;a++)
	{
		inFileFolder += inFileNameArray[a]+slashType;
	}

	inFileName = inFileNameArray[inFileNameArray.GetCount()-1];
	inFileNameArray = inFileName.Split(L".");

	if(inFileNameArray.GetCount() == 2)
	{
		CString Padding = inFileNameArray[1];
		CValue cPadding = (CValue)Padding;
		int iPadding = (int)cPadding;
		int incr = 1;
		bool negate = false;

		CString baseName = inFileNameArray[0];

		bool check = false;
		bool init = false;
		int length, nbp;
		std::fstream infile;
		long startFrame = (long)iPadding;
		long endFrame = startFrame;

		CString outFilePath  = CUtils::BuildPath(outFolder, outFileName+L"."+outFileFormat);
		const char* outfilepath = outFilePath.GetAsciiString() ;

		std::fstream outfile(outfilepath,std::ios::out|std::ios::binary|std::ios::trunc); 
		float* inpos;
		double* outpos;

		do
		{
			// check for negative padding
			if(iPadding<0)
			{
				negate = true;
				Padding = (CString)iPadding;
				CStringArray tArray(Padding.Split(L"-"));
				Padding = tArray[0];	
			}
			else 
			{
				negate = false;
				Padding = (CString)iPadding;
			}

			// padding
			if(Padding.Length()==1)Padding = L"000"+Padding;
			else if(Padding.Length()==2)Padding = L"00"+Padding;
			else if(Padding.Length()==3)Padding = L"0"+Padding;

			// negative padding
			if(negate == true)Padding = L"-"+Padding;

			// build file path
			CString inFilePath = startPath+inFileFolder+baseName+L"."+Padding;
			const char* infilepath = inFilePath.GetAsciiString() ;

			//check for next file
			// if exists append positions to cache file
			struct stat results;
			if (stat(infilepath, &results) == 0)
			{
				if(infile.is_open())infile.close();
				infile.open(infilepath,std::ios::binary|std::ios::in);

				CMatrix4 oMatrix;

				if(init == false)
				{
					// get length of file:
					infile.seekg (0, std::ios::end);
					length = infile.tellg();
					nbp = (length /2)/3/sizeof(float);
					infile.seekg (0, std::ios::beg);

					// allocate memory
					inpos = new float[3*nbp];
					outpos = new double[3*nbp];

					if(outFileFormat == L"bpc")
					{
						// write the bmx header 
						outfile.write((char*) &nbp, sizeof(long));
						outfile.write((char*) &startFrame, sizeof(long));
						outfile.write((char*) &endFrame, sizeof(long));
					}
					else if(outFileFormat == L"bkd")
					{
						// write the pk header
						int signature[10];
						signature[0] = 1967;				// Magic number
						signature[1] = 4;					// Version
						signature[2] = 0;					// Is SRT
						signature[3] = startFrame;			// StartFrame
						signature[4] = endFrame;			// EndFrame

						for(int a=5;a<10;a++)signature[a] = 0;
						outfile.write((char*)& signature,sizeof(int)*10);

						// transfer data from syCache to bmx Cache
						outfile.write((char*)& iPadding,sizeof(int));
						outfile.write((char*)& nbp,sizeof(int));
						outfile.write((char*)& oMatrix,sizeof(CMatrix4));

						infile.read((char*)inpos,3*nbp*sizeof(float));
						for(int x=0;x<3*nbp;x++)outpos[x] = (double)inpos[x];
						outfile.write((char*) outpos, 3*nbp*sizeof(double));
					}
					init = true;
				}

				if(outFileFormat == L"bpc")
				{
					// transfer data from syCache to bmx Cache
					infile.read((char*)inpos,3*nbp*sizeof(float));
					for(int x=0;x<3*nbp;x++)outpos[x] = (double)inpos[x];
					outfile.write((char*) outpos, 3*nbp*sizeof(double));
				}
				else if(outFileFormat == L"bkd")
				{
					// transfer data from syCache to pk Cache
					outfile.write((char*)& iPadding,sizeof(int));
					outfile.write((char*)& nbp,sizeof(int));
					outfile.write((char*)& oMatrix,sizeof(CMatrix4));

					infile.read((char*)inpos,3*nbp*sizeof(float));
					for(int x=0;x<3*nbp;x++)outpos[x] = (double)inpos[x];
					outfile.write((char*) outpos, 3*nbp*sizeof(double));
				}
			}
			else 
			{
				if(outFileFormat == L"bpc")
				{
					// update endframe info in the header of the bpc file
					outfile.seekp(8);
					outfile.write((char*) &endFrame, sizeof(long));
				}
				else if(outFileFormat == L"bkd")
				{
					outfile.seekp(16);
					outfile.write((char*) &endFrame, sizeof(int));
				}
				
				// break and close
				check = true;
				outfile.close();
			}
			//Increment
			iPadding += incr;
			endFrame = (long)iPadding-1;
		}
		while (check == false);
		if(inpos)delete[]inpos;
		if(outpos)delete[]outpos;

		return CStatus::OK;
	}

	else
	{
		Application().LogMessage(L"The selected file is not a syCache file :: operation aborted !!");
		return CStatus::Fail;
	}
}
