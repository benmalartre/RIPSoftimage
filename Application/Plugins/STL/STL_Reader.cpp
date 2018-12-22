// STL_Reader.Cpp
//------------------------------------

#include "STL_Register.h"

XSIPLUGINCALLBACK CStatus STL_Reader_Define( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	CustomOperator oCustomOperator;
	Parameter oParam;
	CRef oPDef;
	Factory oFactory = Application().GetFactory();
	oCustomOperator = ctxt.GetSource();

	oPDef = oFactory.CreateParamDef(L"CacheFile",CValue::siString ,L"");
	oCustomOperator.AddParameter(oPDef,oParam);
	oPDef = oFactory.CreateParamDef(L"CacheStartFrame",CValue::siInt4,siPersistable|siReadOnly,L"CacheStartFrame",L"CacheStartFrame",1l,-10000l,10000l,-1000l,1000l);
	oCustomOperator.AddParameter(oPDef,oParam);
	oPDef = oFactory.CreateParamDef(L"CacheEndFrame",CValue::siInt4,siPersistable|siReadOnly,L"CacheEndFrame",L"CacheEndFrame",100l,-10000l,10000l,-1000l,1000l);
	oCustomOperator.AddParameter(oPDef,oParam);
	oPDef = oFactory.CreateParamDef(L"TimeOffset",CValue::siDouble,siPersistable|siAnimatable,L"TimeOffset",L"TimeOffset",0l,-10000l,10000l,-100l ,100l);
	oCustomOperator.AddParameter(oPDef,oParam);

	oCustomOperator.PutAlwaysEvaluate(true);
	oCustomOperator.PutDebug(0);
	return CStatus::OK;
}

CStatus C_STL_Reader::Update(OperatorContext& in_ctxt)
{
	CustomOperator oOp = in_ctxt.GetSource();

	// Get Inputs
	CString inFilePath( in_ctxt.GetParameterValue(L"CacheFile"));
	inFilePath = CUtils::ResolvePath(inFilePath);
	bool fExist = FileExists(inFilePath.GetAsciiString());

	CTime T = in_ctxt.GetTime();
	int currentFrame = int(T);

	if(fExist)
	{
		int timeOffset(in_ctxt.GetParameterValue(L"TimeOffset"));
		if(_reader._filename != inFilePath.GetAsciiString() || !_reader._file.is_open())
		{
			bool isopen = _reader.OpenFile(inFilePath.GetAsciiString());
			if(!isopen)return CStatus::InvalidArgument;

			bool isvalid = _reader.GetHeader();
			if(!isvalid)return CStatus::InvalidArgument;
		}

		currentFrame +=timeOffset;
		currentFrame -= _reader._startframe;
		_reader.ReadFrame(currentFrame);

		_vertices.Clear();
		_polygons.Clear();

		for(int i=0;i<_reader._nbv;i++)
		{
			_vertices.Add(CVector3(_reader._vertices[i*3],_reader._vertices[i*3+1],_reader._vertices[i*3+2]));
		}

		int offset = 0;
		for(int i=0;i<_reader._nbf;i++)
		{
			_polygons.Add(_reader._faceverticescount[i]);
			for(int j=0;j<_reader._faceverticescount[i];j++)
			{
				_polygons.Add(_reader._facevertices[offset+j]);
			}
			offset+=_reader._faceverticescount[i];
		}

		Primitive output = in_ctxt.GetOutputTarget();
		PolygonMesh(output.GetGeometry()).Set(_vertices,_polygons);
	}
	else
	{
		Application().LogMessage(L"File "+ inFilePath +L" DOESN'T exist !!");
	}


	return CStatus::OK ;
}

/*
MStatus stat;
	if(plug != outmesh)return MS::kUnknownParameter;

	MDataHandle hInput;
	hInput = data.inputValue(currenttime);
	int currentframe = (int)hInput.asFloat();

	hInput = data.inputValue(timeoffset);
	int timeoffset = hInput.asInt();

	hInput = data.inputValue(file);
	MString filename = hInput.asString();
	bool fExist = FileExists(filename);

	if(fExist)
	{
		if(reader->_filename != filename.asChar() || !reader->_file.is_open())
		{
			bool isopen = reader->OpenFile(filename.asChar());
			if(!isopen)return MS::kInvalidParameter;

			bool isvalid = reader->GetHeader();
			if(!isvalid)return MS::kInvalidParameter;
		}

		currentframe +=timeoffset;
		currentframe -= reader->_startframe;
		reader->ReadFrame(currentframe);

		MPointArray vertices;
		MVectorArray normals;
		MIntArray faceverticescount;
		MIntArray facevertices;
		
		for(int a=0;a<reader->_nbv;a++)
		{
			vertices.append(MPoint(reader->_vertices[a*3],reader->_vertices[a*3+1],reader->_vertices[a*3+2]));
			normals.append(MVector(reader->_normals[a*3],reader->_normals[a*3+1],reader->_normals[a*3+2]));
		}

		int fverticessum = 0;
		for(int a=0;a<reader->_nbf;a++)
		{
			fverticessum+=reader->_faceverticescount[a];
			faceverticescount.append(reader->_faceverticescount[a]);
		}

		for(int a=0;a<fverticessum;a++)
		{
			facevertices.append(reader->_facevertices[a]);
		}

		// get outMesh
		MDataHandle outputValueDataHandle = data.outputValue( outmesh );
		
		MFnMeshData outputDataCreator;
		MObject newOutputMeshData = outputDataCreator.create(&stat);

		MFnMesh mesh = newOutputMeshData;
		mesh.create(reader->_nbv, reader->_nbf, vertices, faceverticescount, facevertices, newOutputMeshData,&stat);

		// set outMesh
		outputValueDataHandle.set ( newOutputMeshData );
		data.setClean(plug);

		return MS::kSuccess;
	}
	else
	{
		return MS::kInvalidParameter;
	}
	*/

//////////////////////////////////////////////////////////////////////////////
// Entry Points
//////////////////////////////////////////////////////////////////////////////

XSIPLUGINCALLBACK CStatus STL_Reader_Init( CRef& in_ctx )
{
	Context ctx(in_ctx) ;	
	STL_GeoCache* pThis = new STL_GeoCache();	
	ctx.PutUserData( (CValue::siPtrType)pThis );
	CustomOperator oOp = ctx.GetSource();
	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus STL_Reader_Term( CRef& in_ctx)
{
	Context ctx(in_ctx) ;
	CValue::siPtrType pUserData = ctx.GetUserData();
	C_STL_Reader* pThis = (C_STL_Reader*)pUserData;
	pThis->_reader.Delete();

	delete pThis;
	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus STL_Reader_Update(CRef&	in_ctx)
{
	OperatorContext ctx(in_ctx) ;
	CValue::siPtrType pUserData = ctx.GetUserData();
	C_STL_Reader* pThis = (C_STL_Reader*)pUserData;	
	return pThis->Update(ctx);
}
