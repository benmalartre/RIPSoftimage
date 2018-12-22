#include "IRDeltaMush.h"

enum DMI_IDs
{
	DMI_ID_IN_Geometry = 0,
	DMI_ID_IN_SmoothIteration,
	DMI_ID_IN_PointIndex,
	DMI_ID_G_100 = 100,
	DMI_ID_OUT_OutPort = 200,
	DMI_ID_TYPE_CNS = 400,
	DMI_ID_STRUCT_CNS,
	DMI_ID_CTXT_CNS,
	DMI_ID_UNDEF = ULONG_MAX
};


CStatus RegisterIRDeltaMushInit ( PluginRegistrar& in_reg )
{
	ICENodeDef nodeDef;
	nodeDef = Application().GetFactory().CreateICENodeDef(L"DeltaMushInit",L"DeltaMushInit");

	CStatus st;
	st = nodeDef.PutColor(154,188,102);
	st.AssertSucceeded( ) ;

	// Add input ports and groups.
	st = nodeDef.AddPortGroup(DMI_ID_G_100);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(DMI_ID_IN_Geometry,DMI_ID_G_100,siICENodeDataGeometry,siICENodeStructureSingle,siICENodeContextComponent0D,L"Geometry",L"Geometry",CValue(),CValue(),CValue(),DMI_ID_UNDEF,DMI_ID_UNDEF,DMI_ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(DMI_ID_IN_SmoothIteration,DMI_ID_G_100,siICENodeDataLong,siICENodeStructureSingle,siICENodeContextComponent0D,L"SmoothIteration",L"SmoothIteration",10,0,100,DMI_ID_UNDEF,DMI_ID_UNDEF,DMI_ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(DMI_ID_IN_PointIndex,DMI_ID_G_100,siICENodeDataLong,siICENodeStructureSingle,siICENodeContextComponent0D,L"PointIndex",L"PointIndex",10,0,100,DMI_ID_UNDEF,DMI_ID_UNDEF,DMI_ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	// Add output ports.
	st = nodeDef.AddOutputPort(DMI_ID_OUT_OutPort,siICENodeDataMatrix44,siICENodeStructureSingle,siICENodeContextComponent0D,L"OutPointTransform",L"OutPointTransform");
	st.AssertSucceeded( ) ;


	PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
	nodeItem.PutCategories(L"Delta Mush");

	return CStatus::OK;
};

SICALLBACK IRDeltaMushInit_BeginEvaluate ( ICENodeContext& in_ctxt )
{
	//Get the Geometry Object.
	CICEGeometry	geom ( in_ctxt, DMI_ID_IN_Geometry );
	//Get the inputs.
	CDataArrayLong		smoothIterData ( in_ctxt, DMI_ID_IN_SmoothIteration );

	//Get the geometry point position.
	CDoubleArray	geoPointPosition;
	geom.GetPointPositions ( geoPointPosition );

	//Get the geometry transformation.
	XSI::MATH::CMatrix4f	geoTransform;
	geom.GetTransformation ( geoTransform );

	//Get the geometry polygon number.
	ULONG		nPolyCount = geom.GetPolygonCount();

	//Get the geometry polygon description.
	CLongArray	polyPointCount;
	CLongArray	polyPointIndice;
	geom.GetPolygonIndices ( polyPointCount, polyPointIndice );

	//Create pointer for the array point to compute the init of the delta mush data.
	std::vector<DMPoint_t>	*arrayPoints = new std::vector<DMPoint_t>;
	//Resize the array to the number of point in the input geometry.
	arrayPoints->resize ( geoPointPosition.GetCount() / 3 );

	if ( geoPointPosition.GetCount() != 0 )
	{
		//Get the geometry polygon data to get the point position and the neigbor point index.
		GetGeometryPointData ( geoPointPosition, nPolyCount, polyPointCount, polyPointIndice, arrayPoints );

		//Smooth the point data position.
		SmoothPointPositionData ( smoothIterData[0], arrayPoints );

		//Compute the reference frame for the point.
		ComputePointMushInitData ( arrayPoints );
	}

	//Add the pointer to the Position array in the ice node User data.
	in_ctxt.PutUserData ( (CValue::siPtrType) arrayPoints );

	return CStatus::OK;
};

SICALLBACK IRDeltaMushInit_Evaluate ( ICENodeContext&	in_ctxt )
{
	// The current output port being evaluated...
	ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );

 	// Get the input data buffers for each port
	CDataArrayLong SmoothIterationData( in_ctxt, DMI_ID_IN_SmoothIteration ); 

	//Get the pointer to the Position Array in user Data.
	std::vector<DMPoint_t> *arrayPoints = (std::vector<DMPoint_t>*)(CValue::siPtrType) in_ctxt.GetUserData();

	switch( out_portID )
	{		
		case DMI_ID_OUT_OutPort :
		{
			// Get the output port array ...			
			CDataArrayMatrix4f outData( in_ctxt );
			//CDataArrayVector3f outData( in_ctxt );			
			CDataArrayLong PointIndexe ( in_ctxt, DMI_ID_IN_PointIndex );

 			// We need a CIndexSet to iterate over the data 		
			CIndexSet indexSet( in_ctxt );
			for(CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
			{
				if ( (*arrayPoints).size() != 0 )
				{
					outData[it] = (*arrayPoints)[it.GetAbsoluteIndex()].transform;
				}
			}
		}
		break;
		
		// Other output ports...
	};
	
	return CStatus::OK;
};

SICALLBACK IRDeltaMushInit_EndEvaluate ( ICENodeContext& in_ctxt )
{
	CValue userData = in_ctxt.GetUserData ();

	if ( userData.IsEmpty () )
	{
		return CStatus::OK;
	}

	std::vector<DMPoint_t> *arrayPoints = (std::vector<DMPoint_t>*)(CValue::siPtrType)userData;

	if ( arrayPoints )
		delete arrayPoints;

	in_ctxt.PutUserData ( CValue() );

	return CStatus::OK; 
};