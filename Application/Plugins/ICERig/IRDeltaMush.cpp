#include "IRDeltaMush.h"

enum DM_IDs
{
	DM_ID_IN_Geometry = 0,
	DM_ID_IN_PointIndex, 
	DM_ID_IN_Mush_Iterations,
	DM_ID_IN_Smooth_Iterations,
	DM_ID_IN_DM_Init_Data,
	DM_ID_IN_Blend,
	DM_ID_G_100 = 100,
	DM_ID_OUT_OutPort = 200,
	DM_ID_TYPE_CNS = 400,
	DM_ID_STRUCT_CNS,
	DM_ID_CTXT_CNS,
	DM_ID_UNDEF = ULONG_MAX
};

CStatus	RegisterIRDeltaMush ( PluginRegistrar& in_reg )
{
	ICENodeDef nodeDef;
	nodeDef = Application().GetFactory().CreateICENodeDef(L"DeltaMush",L"DeltaMush");

	CStatus st;
	st = nodeDef.PutColor(154,188,102);
	st.AssertSucceeded( ) ;


	// Add input ports and groups.
	st = nodeDef.AddPortGroup(DM_ID_G_100);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(DM_ID_IN_Geometry,DM_ID_G_100,siICENodeDataGeometry,siICENodeStructureSingle,siICENodeContextComponent0D,L"Geometry",L"Geometry",CValue(),CValue(),CValue(),DM_ID_UNDEF,DM_ID_UNDEF,DM_ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(DM_ID_IN_PointIndex,DM_ID_G_100,siICENodeDataLong,siICENodeStructureSingle,siICENodeContextComponent0D,L"PointIndex",L"PointIndex",10,0,100,DM_ID_UNDEF,DM_ID_UNDEF,DM_ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(DM_ID_IN_Mush_Iterations,DM_ID_G_100,siICENodeDataLong,siICENodeStructureSingle,siICENodeContextComponent0D,L"Mush_Iterations",L"Mush_Iterations",10,0,100,DM_ID_UNDEF,DM_ID_UNDEF,DM_ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(DM_ID_IN_Smooth_Iterations,DM_ID_G_100,siICENodeDataLong,siICENodeStructureSingle,siICENodeContextComponent0D,L"Smooth_Iterations",L"Smooth_Iterations",4,0,100,DM_ID_UNDEF,DM_ID_UNDEF,DM_ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(DM_ID_IN_DM_Init_Data,DM_ID_G_100,siICENodeDataMatrix44,siICENodeStructureArray,siICENodeContextSingleton,L"DM_Offset_Position",L"DM_Offset_Position",8);
	st.AssertSucceeded( ) ;
	
	st = nodeDef.AddInputPort(DM_ID_IN_Blend,DM_ID_G_100,siICENodeDataFloat,siICENodeStructureArray,siICENodeContextSingleton,L"Blend",L"Blend",16);
	st.AssertSucceeded( ) ;
	
	// Add output ports.
	st = nodeDef.AddOutputPort(DM_ID_OUT_OutPort,siICENodeDataVector3,siICENodeStructureSingle,siICENodeContextComponent0D,L"OutPort",L"OutPort");
	st.AssertSucceeded( ) ;

	PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
	nodeItem.PutCategories(L"Delta Mush");

	return CStatus::OK;
};

bool IRDeltaMush_NeedCacheUpdate(ICENodeContext& in_ctxt)
{
	return false;
}

SICALLBACK IRDeltaMush_BeginEvaluate ( ICENodeContext& in_ctxt )
{
	//Get the geometry object.
	CICEGeometry	geom ( in_ctxt, DM_ID_IN_Geometry );

	//Get the input data.
	CDataArrayLong		mushIterData ( in_ctxt, DM_ID_IN_Mush_Iterations );
	CDataArrayLong		smoothIterData ( in_ctxt, DM_ID_IN_Smooth_Iterations );

	CDataArray2DMatrix4f	dmInitData ( in_ctxt, DM_ID_IN_DM_Init_Data );
	CDataArray2DMatrix4f::Accessor arrayDm = dmInitData[0];
	
	CDataArray2DFloat		blendData ( in_ctxt, DM_ID_IN_Blend );
	CDataArray2DFloat::Accessor	arrayBlendData = blendData[0];
	
	//Get the geometry point position.
	CDoubleArray	geoPointPosition;
	geom.GetPointPositions ( geoPointPosition );

	//Get the geometry transformation.
	XSI::MATH::CMatrix4f	geoTransform;
	geom.GetTransformation ( geoTransform );

	//Get Delta Mush User Data
	CValue userData = in_ctxt.GetUserData();
	DMData_t* datas;
	if (userData.IsEmpty() || IRDeltaMush_NeedCacheUpdate(in_ctxt))
	{
		if (!userData.IsEmpty())
		{
			delete (DMData_t*)(CValue::siPtrType)userData;
		}
		datas = new DMData_t();
		datas->initialized = false;
		datas->arrayPoints.resize(geoPointPosition.GetCount() / 3);
	}
	else datas = (DMData_t*)(CValue::siPtrType)userData;

	if ( geoPointPosition.GetCount() != 0 )
	{
		// if not initialized get the geometry polygon data to get the point position and the neigbor point index.
		if (!datas->initialized)
		{
			//Get the geometry polygon number.
			ULONG		nPolyCount = geom.GetPolygonCount();

			//Get the geometry polygon description.
			CLongArray	polyPointCount;
			CLongArray	polyPointIndice;
			geom.GetPolygonIndices(polyPointCount, polyPointIndice);

			GetGeometryPointData(geoPointPosition, nPolyCount, polyPointCount, polyPointIndice, &datas->arrayPoints);
			datas->initialized = true;
		}
		// else only update point position
		else UpdateGeometryPointData(geoPointPosition, datas->arrayPoints);
			
		//Loop for Mush.
		if ( arrayDm.GetCount() == (ULONG)datas->arrayPoints.size() )
		{
			LONG	whileMushCounter = 0;
			while ( whileMushCounter < mushIterData[0] )
			{
				//Smooth the point data position.
				SmoothPointPositionData ( smoothIterData[0], &datas->arrayPoints );

				//Compute the reference frame for the point.
				ComputePointMushData ( &datas->arrayPoints,  dmInitData );

				whileMushCounter++;
			}
		}

		//Loop over point to blending the old geom with the mushed geom.
		if ( arrayBlendData.GetCount() == datas->arrayPoints.size() )
		{
			for ( ULONG iPnt=0 ; iPnt<datas->arrayPoints.size() ; iPnt++ )
			{
				datas->arrayPoints.at(iPnt).position.LinearlyInterpolate ( datas->arrayPoints.at(iPnt).position, datas->arrayPoints.at(iPnt).smoothPosition, arrayBlendData[iPnt] );
				datas->arrayPoints.at(iPnt).switchResult = true;
			}
		}
	}
	
	//Add the pointer to the Position array in the ice node User data.
	in_ctxt.PutUserData ( (CValue::siPtrType) datas );

	return CStatus::OK;
};
SICALLBACK DeltaMush_Evaluate ( ICENodeContext& in_ctxt )
{
	// The current output port being evaluated...
	ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );
  
	switch( out_portID )
	{		
		case DM_ID_OUT_OutPort :
		{
			// Get the output port array ...			
			CDataArrayVector3f outData( in_ctxt );

			DMData_t* datas = (DMData_t*)(CValue::siPtrType) in_ctxt.GetUserData();

			CDataArrayLong PointIndexData ( in_ctxt, DM_ID_IN_PointIndex );

 			// We need a CIndexSet to iterate over the data 		
			CIndexSet indexSet( in_ctxt );
			for(CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
			{	
				if (datas->arrayPoints.size() != 0 )
				{
					if (datas->arrayPoints[it.GetAbsoluteIndex()].switchResult )
					{
						outData[it] = datas->arrayPoints[it.GetAbsoluteIndex()].position ;
					}
					else
					{
						outData[it] = datas->arrayPoints[it.GetAbsoluteIndex()].smoothPosition ;
					}
				}
			}
		}
		break;
	};
	
	return CStatus::OK;
};

SICALLBACK IRDeltaMush_EndEvaluate ( ICENodeContext& in_ctxt )
{
	return CStatus::OK; 
};

SICALLBACK IRDeltaMush_Init(ICENodeContext& in_ctxt)
{
	return CStatus::OK;
}

SICALLBACK IRDeltaMush_Terminate(ICENodeContext& in_ctxt)
{
	CValue userData = in_ctxt.GetUserData();

	if (userData.IsEmpty())
	{
		return CStatus::OK;
	}

	DMData_t *datas = (DMData_t*)(CValue::siPtrType)userData;

	if (datas) delete datas;

	in_ctxt.PutUserData(CValue());

	return CStatus::OK;
}