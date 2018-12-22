// sPointCache.Cpp
//------------------------------------
#include "sRegister.h"
#include "sPointCache.h"

XSIPLUGINCALLBACK CStatus bbppPointCache_Define( CRef& in_ctxt )
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
	oPDef = oFactory.CreateParamDef(L"TimeWarp",CValue::siDouble,siPersistable|siAnimatable,L"TimeWarp",L"TimeWarp",1l,0.001,5l,0.001 ,5l);
	oCustomOperator.AddParameter(oPDef,oParam);
	oPDef = oFactory.CreateParamDef(L"CustomFrame",CValue::siDouble,siPersistable|siAnimatable,L"CustomFrame",L"CustomFrame",0l,0l,99999999l,01 ,1000l);
	oCustomOperator.AddParameter(oPDef,oParam);
	oPDef = oFactory.CreateParamDef(L"UseCustomPlayback",CValue::siBool,siPersistable|siAnimatable,L"UseCustomPlayback",L"UseCustomPlayback",false,false,true,false ,true);
	oCustomOperator.AddParameter(oPDef,oParam);
	oPDef = oFactory.CreateParamDef(L"Interpolation",CValue::siInt4,siPersistable|siNonKeyableVisible,L"Interpolation",L"Interpolation",2l,0l,2l,0l ,2l);
	oCustomOperator.AddParameter(oPDef,oParam);
	oPDef = oFactory.CreateParamDef(L"HermiteTension",CValue::siFloat,siPersistable|siAnimatable,L"HermiteTension",L"HermiteTension",0l,-1l,1l,-1l,1l);
	oCustomOperator.AddParameter(oPDef,oParam);
	oPDef = oFactory.CreateParamDef(L"HermiteBias",CValue::siFloat,siPersistable|siAnimatable,L"HermiteBias",L"HermiteBias",0l,-1l,1l,-1l ,1l);
	oCustomOperator.AddParameter(oPDef,oParam);

	oCustomOperator.PutAlwaysEvaluate(true);
	oCustomOperator.PutDebug(0);
	return CStatus::OK;
}

CStatus sPointCache::Init(const CString& in_filepath,long in_nbpoints )
{
	if(_reader.Init(in_filepath.GetAsciiString(),in_nbpoints))return CStatus::OK;
	else return CStatus::False;
}

CStatus sPointCache::Update(OperatorContext& in_ctxt)
{
	CustomOperator oOp = in_ctxt.GetSource();

	// Get Inputs
	Primitive inPrim(in_ctxt.GetInputValue(0));
	Geometry inGeom(inPrim.GetGeometry());
	CPointRefArray inPoints(inGeom.GetPoints());
	CVector3Array inPos(inPoints.GetPositionArray());
	long nbP = inPoints.GetCount();
	CString inFilePath( in_ctxt.GetParameterValue(L"CacheFile"));
	inFilePath = CUtils::ResolvePath(inFilePath);

	// Initialize Pointers
	if(_reader._position == NULL)_reader.CreatePointers(nbP);
	
	if(inFilePath.Length()>0 && (!_reader._s.is_open()|| inFilePath != _reader._file))
	{
		_reader.Init(inFilePath.GetAsciiString(),nbP);
		oOp.PutParameterValue(L"CacheStartFrame", _reader._startFrame);
		oOp.PutParameterValue(L"CacheEndFrame", _reader._endFrame);
	}

	if(inFilePath.Length()>0 && _reader._s.is_open()&& inFilePath == _reader._file)
	{
		if(nbP == _reader._nbPoints)
		{
			float oOffset = oOp.GetParameterValue(L"TimeOffset");
			float oWarp = oOp.GetParameterValue(L"TimeWarp");
			bool oUseCustomPlayback = oOp.GetParameterValue(L"UseCustomPlayback");

			if(oWarp == 0)oWarp = 1;

			CTime T = in_ctxt.GetTime();
			float t = float(T);
			float epsilon = 0.01;
			if(t<0)epsilon = -0.01;
			float currentFrame;
			if(oUseCustomPlayback == true)currentFrame = oOp.GetParameterValue(L"CustomFrame");
			else currentFrame = (t-oOffset-_reader._startFrame)*oWarp;

			if(_reader.isSubFrame(currentFrame,(long)(currentFrame+epsilon)))
			{
				long previous = (long)(currentFrame);
				long next = previous + 1;
				long p = (long)(currentFrame);
				float interpolate = (float)fabs(p - currentFrame);
				
				if(previous<0)_reader.ReadFrame(0);

				else if(next>(_reader._endFrame - _reader._startFrame))
				{
					_reader.ReadFrame(_reader._endFrame - _reader._startFrame);
				}

				else
				{
					long oInterpolateMode = oOp.GetParameterValue(L"Interpolation");
					float oTension = oOp.GetParameterValue(L"HermiteTension");
					float oBias = oOp.GetParameterValue(L"HermiteBias");

					if(previous == 0 ||next == _reader._endFrame - _reader._startFrame)
					{
						oInterpolateMode = 0;
					}
						
					_reader.ReadSubFrame(previous,next,interpolate,oInterpolateMode,oTension,oBias);
				}
				for(long a=0;a<_reader._nbPoints;a++)
				{
					inPos[a].Set(_reader._position[a*3],_reader._position[a*3+1],_reader._position[a*3+2]);
				}
			}
			else
			{
				long ct;
				if(t>=0)ct = (long)(currentFrame + epsilon);
				else ct = (long)(currentFrame + 1 + epsilon);

				if(ct<0)
				{
					_reader.ReadFrame(0);
				}

				else if(ct>(_reader._endFrame - _reader._startFrame))
				{
					_reader.ReadFrame(_reader._endFrame - _reader._startFrame);
				}

				else
				{
					_reader.ReadFrame(ct);

				}	
				for(long a=0;a<_reader._nbPoints;a++)
				{	
					inPos[a].Set(_reader._position[a*3],_reader._position[a*3+1],_reader._position[a*3+2]);
				}
			}
		}
	}

	Primitive outPrim(in_ctxt.GetOutputTarget());
	Geometry outGeom(outPrim.GetGeometry());
	outGeom.GetPoints().PutPositionArray(inPos);

	return CStatus::OK ;
}

//////////////////////////////////////////////////////////////////////////////
// Entry Points
//////////////////////////////////////////////////////////////////////////////

XSIPLUGINCALLBACK CStatus bbppPointCache_Init( CRef& in_ctx )
{
	Context ctx(in_ctx) ;	
	sPointCache* pThis = new sPointCache();	
	ctx.PutUserData( (CValue::siPtrType)pThis );
	CustomOperator oOp = ctx.GetSource();
	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus bbppPointCache_Term( CRef& in_ctx)
{
	Context ctx(in_ctx) ;
	CValue::siPtrType pUserData = ctx.GetUserData();
	sPointCache* pThis = (sPointCache*)pUserData;
	pThis->_reader.DeletePointers();

	delete pThis;
	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus bbppPointCache_Update(CRef&	in_ctx)
{
	OperatorContext ctx(in_ctx) ;
	CValue::siPtrType pUserData = ctx.GetUserData();
	sPointCache* pThis = (sPointCache*)pUserData;	
	return pThis->Update(ctx);
}