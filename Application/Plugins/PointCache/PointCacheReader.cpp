// PointCacheReader.Cpp
//------------------------------------
#include "PointCache.h"
#include <xsi_factory.h>

XSIPLUGINCALLBACK CStatus PointCacheOp_Define( CRef& in_ctxt )
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

CStatus CPointCacheOp::Init(const CString& in_filepath,long in_nbpoints )
{
	if(_s.is_open())_s.close();
	_file = (CString)in_filepath;

	const char* filepath = in_filepath.GetAsciiString() ;

	struct stat results;
	if (stat(filepath, &results) == 0)
	{
		_s.open(filepath,std::ios::binary|std::ios::in);

		_s.read((char*) &_nbPoints, sizeof(long));
		_s.read((char*) &_startFrame, sizeof(long));
		_s.read((char*) &_endFrame, sizeof(long));

		if(in_nbpoints != _nbPoints)Application().LogMessage(L"Number Vertices doesn't match ---> Select another cache file...");

		return CStatus::OK;
	}
	else
	{
		return CStatus::False;
	}
}

CStatus CPointCacheOp::Update(OperatorContext& in_ctxt)
{
	CustomOperator oOp = in_ctxt.GetSource();

	// Get Inputs
	Primitive inPrim(in_ctxt.GetInputValue(0));
	Geometry inGeom(inPrim.GetGeometry());
	CPointRefArray inPoints(inGeom.GetPoints());
	CVector3Array inPos(inPoints.GetPositionArray());
	long nbP = inPoints.GetCount();
	CString inFilePath( in_ctxt.GetParameterValue(L"CacheFile"));
  if (inFilePath == L"")return CStatus::OK;
	inFilePath = CUtils::ResolvePath(inFilePath);


	// Initialize Pointers
	if(_position == NULL)CreatePointers(nbP);
	
	if(inFilePath.Length()>0 && (!_s.is_open()|| inFilePath != _file))
	{
		Init(inFilePath,nbP);
		oOp.PutParameterValue(L"CacheStartFrame",_startFrame);
		oOp.PutParameterValue(L"CacheEndFrame",_endFrame);
	}

	if(inFilePath.Length()>0 && _s.is_open()&& inFilePath == _file)
	{
		if(nbP == _nbPoints)
		{
			double oOffset = (double)oOp.GetParameterValue(L"TimeOffset");
			double oWarp = (double)oOp.GetParameterValue(L"TimeWarp");
			bool oUseCustomPlayback = oOp.GetParameterValue(L"UseCustomPlayback");

			if(oWarp == 0)oWarp = 1;

			CTime T = in_ctxt.GetTime();
			double t = double(T);
			double epsilon = 0.01;
			if(t<0)epsilon = -0.01;
			double currentFrame;
			if(oUseCustomPlayback == true)currentFrame = oOp.GetParameterValue(L"CustomFrame");
			else currentFrame = (t-oOffset-_startFrame)*oWarp;

			if(isSubFrame(currentFrame,(long)(currentFrame+epsilon)))
			{
				long previous = (long)(currentFrame);
				long next = previous + 1;
				long p = (long)(currentFrame);
				double interpolate = fabs((double)p - currentFrame);
				
				if(previous<0)ReadFrame(0);

				else if(next>(_endFrame - _startFrame))
				{
					ReadFrame(_endFrame - _startFrame);
				}

				else
				{
					long oInterpolateMode = oOp.GetParameterValue(L"Interpolation");
					double oTension = oOp.GetParameterValue(L"HermiteTension");
					double oBias = oOp.GetParameterValue(L"HermiteBias");

					if(previous == 0 ||next == _endFrame - _startFrame)
					{
						oInterpolateMode = 0;
					}
						
					ReadSubFrame(previous,next,interpolate,oInterpolateMode,oTension,oBias);
				}
				for(long a=0;a<_nbPoints;a++)
				{
					inPos[a].Set(_position[a*3],_position[a*3+1],_position[a*3+2]);
				}
			}
			else
			{
				long ct;
				if(t>=0)ct = (long)(currentFrame + epsilon);
				else ct = (long)(currentFrame + 1 + epsilon);

				if(ct<0)
				{
					ReadFrame(0);
				}

				else if(ct>(_endFrame - _startFrame))
				{
					ReadFrame(_endFrame - _startFrame);
				}

				else
				{
					ReadFrame(ct);

				}	
				for(long a=0;a<_nbPoints;a++)
				{	
					inPos[a].Set(_position[a*3],_position[a*3+1],_position[a*3+2]);
				}
			}
		}
	}

	Primitive outPrim(in_ctxt.GetOutputTarget());
	Geometry outGeom(outPrim.GetGeometry());
	outGeom.GetPoints().PutPositionArray(inPos);

	return CStatus::OK ;
}

inline CStatus CPointCacheOp::ReadFrame(long in_frame)
{
	long id = in_frame * SIZE_POINT * _nbPoints + START_CACHE;
	_s.seekg(id,std::ios::beg);
	_s.read((char*) _position, sizeof(double)* 3 *_nbPoints);

	return CStatus::OK ;
}

inline CStatus CPointCacheOp::ReadSubFrame(long in_previous,long in_next,double in_interpolate,long in_mode,double in_tension,double in_bias)
{
	long id = in_previous * SIZE_POINT * _nbPoints + START_CACHE;
	_s.seekg(id,std::ios::beg);

	_s.read((char*) _previous, sizeof(double)* 3 *_nbPoints);

	id = in_next * SIZE_POINT * _nbPoints + START_CACHE;
	_s.seekg(id, std::ios::beg);

	_s.read((char*) _next, sizeof(double)* 3 *_nbPoints);

	if(in_mode==0)
	{
		for(long a=0;a<_nbPoints;a++)
		{
			_position[a*3] = LinearInterpolate(_previous[a*3],_next[a*3],in_interpolate);
			_position[a*3+1] = LinearInterpolate(_previous[a*3+1],_next[a*3+1],in_interpolate);
			_position[a*3+2] = LinearInterpolate(_previous[a*3+2],_next[a*3+2],in_interpolate);
		}
	}
	
	else 
	{
		id = (in_previous - 1) * SIZE_POINT * _nbPoints + START_CACHE;
		_s.seekg(id,std::ios::beg);

		_s.read((char*) _previous2, sizeof(double)* 3 *_nbPoints);

		id = (in_next + 1) * SIZE_POINT * _nbPoints + START_CACHE;
		_s.seekg(id,std::ios::beg);

		_s.read((char*) _next2, sizeof(double)* 3 *_nbPoints);

		if(in_mode == 1)
		{
			for(long a=0;a<_nbPoints;a++)
			{
				_position[a*3] = CubicInterpolate(_previous2[a*3],_previous[a*3],_next[a*3],_next2[a*3],in_interpolate);
				_position[a*3+1] = CubicInterpolate(_previous2[a*3+1],_previous[a*3+1],_next[a*3+1],_next2[a*3+1],in_interpolate);
				_position[a*3+2] = CubicInterpolate(_previous2[a*3+2],_previous[a*3+2],_next[a*3+2],_next2[a*3+2],in_interpolate);
			}
		}
		else
		{
			for(long a=0;a<_nbPoints;a++)
			{
				_position[a*3] = HermiteInterpolate(_previous2[a*3],_previous[a*3],_next[a*3],_next2[a*3],in_interpolate,in_tension,in_bias);
				_position[a*3+1] = HermiteInterpolate(_previous2[a*3+1],_previous[a*3+1],_next[a*3+1],_next2[a*3+1],in_interpolate,in_tension,in_bias);
				_position[a*3+2] = HermiteInterpolate(_previous2[a*3+2],_previous[a*3+2],_next[a*3+2],_next2[a*3+2],in_interpolate,in_tension,in_bias);
			}
		}
	}

	return CStatus::OK ;
}

bool CPointCacheOp::isSubFrame(double in_frame,long in_roundedframe)
{
	if(fabs(in_frame-in_roundedframe)>0.001)return 1;
	else return 0;
}

void CPointCacheOp::CreatePointers(long in_nb)
{
	_position = new double[in_nb*3];
	_previous = new double[in_nb*3];
	_next = new double[in_nb*3];
	_previous2 = new double[in_nb*3];
	_next2 = new double[in_nb*3];
}

void CPointCacheOp::DeletePointers()
{
	if(_position)delete[]_position;
	if(_previous)delete[]_previous;
	if(_next)delete[]_next;
	if(_previous2)delete[]_previous2;
	if(_next2)delete[]_next2;
	if(_blobdatas)delete[]_blobdatas;
}

//////////////////////////////////////////////////////////////////////////////
// Entry Points
//////////////////////////////////////////////////////////////////////////////
XSIPLUGINCALLBACK CStatus PointCacheOp_Init( CRef& in_ctx )
{
	Context ctx(in_ctx) ;	
	CPointCacheOp* pThis = new CPointCacheOp();	
	ctx.PutUserData( (CValue::siPtrType)pThis );
	CustomOperator oOp = ctx.GetSource();
	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus PointCacheOp_Term( CRef& in_ctx)
{
	Context ctx(in_ctx) ;
	CValue::siPtrType pUserData = ctx.GetUserData();
	CPointCacheOp* pThis = (CPointCacheOp*)pUserData;	
	pThis->DeletePointers();
	delete pThis;
	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus PointCacheOp_Update(CRef&	in_ctx)
{
  Application().LogMessage("POINT CACHE OP UPDATE");
	OperatorContext ctx(in_ctx) ;
	CValue::siPtrType pUserData = ctx.GetUserData();
	CPointCacheOp* pThis = (CPointCacheOp*)pUserData;	
	return pThis->Update(ctx);
}