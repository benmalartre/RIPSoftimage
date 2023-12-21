// Triangulate Plugin
//--------------------------------------------------------------------
#include "plugin.h"
#include "triangulate.h"

typedef struct {
  double A,B,C,D;
}plane;


void ConstructPlane(plane& P,CVector3f& p1,CVector3f& p2,CVector3f& p3){
  /*A = y1 (z2 - z3) + y2 (z3 - z1) + y3 (z1> - z2)
  B = z1 (x2 - x3) + z2 (x3 - x1) + z3 (x1 - x2)
  C = x1 (y2 - y3) + x2 (y3 - y1) + x3 (y1 - y2)
  - D = x1 (y2 z3 - y3 z2) + x2 (y3 z1 - y1 z3) + x3 (y1 z2 - y2 z1)*/
	
  P.A=p1.GetY()*( p2.GetZ()-p3.GetZ() ) + p2.GetY()*( p3.GetZ()-p1.GetZ() ) + p3.GetY()*( p1.GetZ()-p2.GetZ() ) ;
  P.B=p1.GetZ()*( p2.GetX()-p3.GetX() ) + p2.GetZ()*( p3.GetX()-p1.GetX() ) + p3.GetZ()*( p1.GetX()-p2.GetX() ) ;	
  P.C=p1.GetX()*( p2.GetY()-p3.GetY() ) + p2.GetX()*( p3.GetY()-p1.GetY() ) + p3.GetX()*( p1.GetY()-p2.GetY() ) ;
  P.D=-( p1.GetX()*( p2.GetY()*p3.GetZ()-p3.GetY()*p2.GetZ() ) + p2.GetX()*( p3.GetY()*p1.GetZ()-p1.GetY()*p3.GetZ() ) +p3.GetX()*( p1.GetY()*p2.GetZ()-p2.GetY()*p1.GetZ() )  );
}



bool IsInHalfSpace(plane& P,CVector3f& point){

  Application app;
  double signed_distance =P.A*point.GetX()+ P.B*point.GetY()+P.C*point.GetZ()+P.D;

  app.LogMessage(CValue((double)signed_distance).GetAsText());

  if(signed_distance +0.00001  >=0 )
    return true;
  else
    return false;
}

bool IsInFrustum(plane& left,plane& right, plane& up, plane& down, plane& nearp,plane& farp,CVector3f& point){
  if(IsInHalfSpace(left,point) &&
    IsInHalfSpace(right,point) &&
    IsInHalfSpace(up,point) &&
    IsInHalfSpace(down,point) && 
    IsInHalfSpace(nearp,point) && 
    IsInHalfSpace(farp,point))
    return true;
  return false;

}

// Defines port, group and map identifiers used for registering the ICENode
enum IDs
{
  ID_IN_PointPosition = 0,
  ID_IN_CameraMatrix = 1,
  ID_IN_CameraAspect = 2,
  ID_IN_CameraFov = 3,
  ID_IN_CameraFovType = 4,
  ID_IN_CameraFar = 5,
  ID_IN_CameraNear = 6,
  ID_G_100 = 100,
  ID_OUT_TrianglePointPosition = 200,
  ID_OUT_TrianglePointID = 201,
  ID_TYPE_CNS = 400,
  ID_STRUCT_CNS,
  ID_CTXT_CNS,
  ID_UNDEF = ULONG_MAX
};

XSI::CStatus RegisterTriangulate( XSI::PluginRegistrar& in_reg );

using namespace XSI; 

struct TriangulateData{
  std::vector<CVector3f> _pos;
  std::vector<LONG> _id;
};

SICALLBACK XSILoadPlugin( PluginRegistrar& in_reg )
{
  in_reg.PutAuthor(L"benmalartre");
  in_reg.PutName(L"Triangulate");
  in_reg.PutVersion(1,0);

  RegisterTriangulate( in_reg );

  return CStatus::OK;
}

SICALLBACK XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
  CString strPluginName;
  strPluginName = in_reg.GetName();
  Application().LogMessage(strPluginName + L" has been unloaded.",siVerboseMsg);
  return CStatus::OK;
}

CStatus RegisterTriangulate( PluginRegistrar& in_reg )
{
  ICENodeDef nodeDef;
  nodeDef = Application().GetFactory().CreateICENodeDef(L"Triangulate",L"Triangulate");

  CStatus st;
  st = nodeDef.PutColor(200,200,200);
  st.AssertSucceeded( ) ;

  st = nodeDef.PutThreadingModel(XSI::siICENodeSingleThreading);
  st.AssertSucceeded( ) ;


  // Add input ports and groups.
  st = nodeDef.AddPortGroup(ID_G_100);
  st.AssertSucceeded( ) ;

  st = nodeDef.AddInputPort(ID_IN_PointPosition,ID_G_100,siICENodeDataVector3,siICENodeStructureAny,siICENodeContextAny,L"PointPosition",L"PointPosition",MATH::CVector3f(0.0,0.0,0.0));
  st.AssertSucceeded( ) ;

  st = nodeDef.AddInputPort(ID_IN_CameraMatrix,ID_G_100,siICENodeDataMatrix44,siICENodeStructureSingle,siICENodeContextSingleton,L"CameraMatrix",L"CameraMatrix",MATH::CMatrix4f());
  st.AssertSucceeded( ) ;

  st = nodeDef.AddInputPort(ID_IN_CameraAspect,ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"CameraAspect",L"CameraAspect",1.0);
  st.AssertSucceeded( ) ;

  st = nodeDef.AddInputPort(ID_IN_CameraFov,ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"CameraFov",L"CameraFov",0.0);
  st.AssertSucceeded( ) ;

  st = nodeDef.AddInputPort(ID_IN_CameraFovType,ID_G_100,siICENodeDataLong,siICENodeStructureSingle,siICENodeContextSingleton,L"CameraFovType",L"CameraFovType",0);
  st.AssertSucceeded( ) ;

  st = nodeDef.AddInputPort(ID_IN_CameraFar,ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"CameraFar",L"CameraFar",0.0);
  st.AssertSucceeded( ) ;

  st = nodeDef.AddInputPort(ID_IN_CameraNear,ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"CameraNear",L"CameraNear",0.0);
  st.AssertSucceeded( ) ;

  // Add output ports.
  st = nodeDef.AddOutputPort(ID_OUT_TrianglePointPosition,siICENodeDataVector3,siICENodeStructureArray,siICENodeContextSingleton,L"TrianglePointPosition",L"TrianglePointPosition", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
  st.AssertSucceeded( ) ;

  st = nodeDef.AddOutputPort(ID_OUT_TrianglePointID,siICENodeDataLong,siICENodeStructureArray,siICENodeContextSingleton,L"TrianglePointID",L"TrianglePointID", ID_UNDEF, ID_UNDEF, ID_CTXT_CNS);
  st.AssertSucceeded( ) ;

  PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
  nodeItem.PutCategories(L"Custom ICENode");

  return CStatus::OK;
}

SICALLBACK Triangulate_Init( CRef& in_ctxt )
{
  Context ctxt( in_ctxt );
  TriangulateData* data = new TriangulateData();
  ctxt.PutUserData( data );
  return CStatus::OK;
}

SICALLBACK Triangulate_Term( CRef& in_ctxt )
{
  Context ctxt( in_ctxt );
  // Release memory allocated in BeginEvaluate
  CValue userData = ctxt.GetUserData( );
  if ( userData.IsEmpty() )
  {
    return CStatus::OK;
  }

  TriangulateData* data = (TriangulateData*)(CValue::siPtrType)userData;
  delete data;

  // Clear user data; 
  ctxt.PutUserData( CValue());
  return CStatus::OK;
}


SICALLBACK Triangulate_BeginEvaluate( ICENodeContext& in_ctxt )
{
  // Get User Data
  CValue userData = in_ctxt.GetUserData( );
  if ( userData.IsEmpty() )
  {
    return CStatus::OK;
  }
  LOG("Triangulate BEGIN EVALUATE");
  TriangulateData* data = (TriangulateData*)(CValue::siPtrType)userData;

  // Get the input data buffers for each port
  CDataArrayVector3f PointPositionData( in_ctxt, ID_IN_PointPosition );
  CDataArrayMatrix4f CameraMatrixData( in_ctxt, ID_IN_CameraMatrix );	
  CDataArrayFloat CameraAspectData( in_ctxt, ID_IN_CameraAspect );	
  CDataArrayFloat CameraFovData( in_ctxt, ID_IN_CameraFov );
  CDataArrayLong CameraFovTypeData( in_ctxt, ID_IN_CameraFovType);
  CDataArrayFloat CameraFarData( in_ctxt, ID_IN_CameraFar );
  CDataArrayFloat CameraNearData( in_ctxt, ID_IN_CameraNear);

  int ntri = 0;

  CMatrix4f camMatrix = CameraMatrixData[0];

  CVector3f NearLD,NearLU,NearRD,NearRU,FarLD,FarLU,FarRD,FarRU;

  float aspect = CameraAspectData[0];
  float fov = CameraFovData[0];
  long fovType = CameraFovTypeData[0];
  float znear =CameraFarData[0];
  float zfar = CameraNearData[0];

  float fov_radian= PI*(fov+0.0001)/180.0;
  float nHalfW, nHalfH, fHalfW, fHalfH ;

  if (fovType == 1){ /////Horizontal FoV
    nHalfW = tan(fov_radian/2.0) * znear;
    nHalfH = nHalfW / aspect;
    fHalfW = tan(fov_radian/2.0) * zfar;
    fHalfH = fHalfW / aspect;
  }

  else if(fovType == 0){ /////Vertical FoV
    nHalfH = tan(fov_radian/2.0) * znear;
    nHalfW= nHalfH * aspect;
    fHalfH = tan(fov_radian/2.0) * zfar;
    fHalfW = fHalfH * aspect;
  }

  //------------------------------------Points of the near Clipping Plane
  NearLD.PutX(-nHalfW);
  NearLD.PutY(-nHalfH);
  NearLD.PutZ(-znear);

  NearLD.MulByMatrix4InPlace(camMatrix);

  NearLU.PutX(-nHalfW);
  NearLU.PutY(nHalfH);
  NearLU.PutZ(-znear);

  NearLU.MulByMatrix4InPlace(camMatrix);

  NearRD.PutX(nHalfW);
  NearRD.PutY(-nHalfH);
  NearRD.PutZ(-znear);

  NearRD.MulByMatrix4InPlace(camMatrix);

  NearRU.PutX(nHalfW);
  NearRU.PutY(nHalfH);
  NearRU.PutZ(-znear);

  NearRU.MulByMatrix4InPlace(camMatrix);
  //------------------------------------------------------------------------------------

  //------------------------------------Points of the far Clipping Plane
  FarLD.PutX(-fHalfW);
  FarLD.PutY(-fHalfH);
  FarLD.PutZ(-zfar);

  FarLD.MulByMatrix4InPlace(camMatrix);

  FarLU.PutX(-fHalfW);
  FarLU.PutY(fHalfH);
  FarLU.PutZ(-zfar);

  FarLU.MulByMatrix4InPlace(camMatrix);

  FarRD.PutX(fHalfW);
  FarRD.PutY(-fHalfH);
  FarRD.PutZ(-zfar);

  FarRD.MulByMatrix4InPlace(camMatrix);

  FarRU.PutX(fHalfW);
  FarRU.PutY(fHalfH);
  FarRU.PutZ(-zfar);

  FarRU.MulByMatrix4InPlace(camMatrix);
  //------------------------------------------------------------------------------------


  //-----------------------------------------CLIPPING PLANES
  plane left_plane,right_plane,up_plane,down_plane,near_plane,far_plane;
  ConstructPlane(left_plane,NearLD,FarLD ,FarLU);
  ConstructPlane(right_plane,FarRD,NearRD,FarRU);
  ConstructPlane(up_plane,NearRU,NearLU,FarRU);
  ConstructPlane(down_plane,NearLD,NearRD,FarRD);
  ConstructPlane(near_plane,NearLD,NearLU,NearRD);
  ConstructPlane(far_plane,FarLU,FarRU,FarRD);
  //----------------------------------------------------------------------------------------------------------

  long nbPoints = PointPositionData.GetCount();
  LOG("NUM POINTS " + nbPoints);

  ITriangulate<float>* triangles = ITriangulate<float>::Create();

  CVector3f* positions = &PointPositionData[0];
  int verts = triangles->Triangulate(nbPoints, &positions[0][0], &positions[0][1], sizeof(CVector3f));

  /*
  int nb_visible_points=0;


	
  if (nbpoints>3) 
  {
    Vertex_t* p = (Vertex_t *)malloc((nbpoints+3)*sizeof(Vertex_t)); 
    Triangle_t* v = (Triangle_t *)malloc(3*nbpoints*sizeof(Triangle_t));

    // Note: Specific CIndexSet for PointPosition is required in single-threading mode			
    CIndexSet PointPositionIndexSet(in_ctxt, ID_IN_PointPosition);
    CVector3f pos;
    float totalFactor = 0.;
    double  h = 1.0 / tan(PI*fov / 180.0);

    CMatrix4f projmat(
    1.0 / (2.0*nHalfW), 0, 0, 0,
    0, 1.0 / (2.0*nHalfH), 0, 0,
    0, 0, (zfar) / (zfar - znear), -1,
    0, 0, -(znear * zfar) / (zfar - znear), 0);

    for (CIndexSet::Iterator itPointPosition = PointPositionIndexSet.Begin(); itPointPosition.HasNext(); itPointPosition.Next()) {
      pos = PointPositionData[itPointPosition];
      p[nb_visible_points].x = pos.GetX();
      p[nb_visible_points].y0 = pos.GetY();
      p[nb_visible_points].z0 = pos.GetZ();

      CMatrix4f invMatrix;
      invMatrix.Invert(camMatrix);
      CVector3f nPosLoc;
      nPosLoc.MulByMatrix4(pos, invMatrix);

      CVector3f nPosLoc2;
      nPosLoc2.MulByMatrix4(nPosLoc, projmat);

      //if(	is_in_frustum(left_plane,right_plane,up_plane,down_plane,near_plane,up_plane, nPosLoc2) )
      //{
      p[nb_visible_points].x = (double)(nPosLoc2.GetX());
      p[nb_visible_points].y = (double)(nPosLoc2.GetY());
      p[nb_visible_points++].z = (double)(nPosLoc2.GetZ());

    //}
    }


    std::sort(p,nb_visible_points,sizeof(Vertex_t),XYZCompare);
    Triangulate(nb_visible_points,p,v,&ntri);
    data->_pos.resize(nb_visible_points);
    for(int i=0;i<nb_visible_points;i++)
    {
    data->_pos[i].Set((float)p[i].x0, (float)p[i].y0, (float)p[i].z0);
    }

    long n_index=4;
    data->_id.resize(ntri*4);
    for (int i=0;i<ntri;i++) 
    {
      data->_id[i*4] = v[i].p1;
      data->_id[i*4+1] =v[i].p2;
      data->_id[i*4+2] =v[i].p3;
      data->_id[i*4+3] =-2;
    }

    free(p);
    free(v);
  }

  */
  return CStatus::OK;
}

SICALLBACK Triangulate_Evaluate( ICENodeContext& in_ctxt )
{
	// Get User Data
	CValue userData = in_ctxt.GetUserData();
	if (userData.IsEmpty())return CStatus::OK;
	TriangulateData* data = (TriangulateData*)(CValue::siPtrType)userData;

	// The current output port being evaluated...
	ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );
  
	switch( out_portID )
	{		
		case ID_OUT_TrianglePointPosition :
		{
			// Get the output port array ...			
			CDataArray2DVector3f outData(in_ctxt);
			size_t nbp = data->_pos.size();
			CDataArray2DVector3f::Accessor outDataSubArray = outData.Resize(0, nbp);
			for (ULONG n = 0; n<nbp; n++)
			{
				outDataSubArray[n] = data->_pos[n];
			}
		}
		break;

		case ID_OUT_TrianglePointID :
		{
			// Get the output port array ...			
			CDataArray2DLong outData(in_ctxt);
			size_t nbi = data->_id.size();
			CDataArray2DLong::Accessor outDataSubArray = outData.Resize(0, nbi);
			for (ULONG n = 0; n<nbi; n++)
			{
				outDataSubArray[n] = data->_id[n];
			}
		}
		break;
		
	};
	
	return CStatus::OK;
}