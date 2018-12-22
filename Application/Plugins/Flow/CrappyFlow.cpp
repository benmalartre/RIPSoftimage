//===============================================================================================
// CrappyFlow ICENode
//===============================================================================================
#include "CrappyFlow.h"

// Defines port, group and map identifiers used for registering the ICENode
//------------------------------------------------
enum IDs
{
	ID_IN_Curves = 0,
	ID_IN_Width = 1,
	ID_G_100 = 100,
	ID_OUT_Vertices = 200,
	ID_OUT_Polygons = 201,
	ID_TYPE_CNS = 400,
	ID_STRUCT_CNS,
	ID_CTXT_CNS,
	ID_UNDEF = ULONG_MAX
};


// Load Plugin
//------------------------------------------------
SICALLBACK XSILoadPlugin( PluginRegistrar& in_reg )
{
	in_reg.PutAuthor(L"benmalartre");
	in_reg.PutName(L"CrappyFlow");
	in_reg.PutVersion(1,0);

	RegisterCrappyFlow( in_reg );

	//RegistrationInsertionPoint - do not remove this line

	return CStatus::OK;
}

// Unload Plugin
//------------------------------------------------
SICALLBACK XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
	CString strPluginName;
	strPluginName = in_reg.GetName();
	Application().LogMessage(strPluginName + L" has been unloaded.",siVerboseMsg);
	return CStatus::OK;
}

// Register
//------------------------------------------------
CStatus RegisterCrappyFlow( PluginRegistrar& in_reg )
{
	ICENodeDef nodeDef;
	nodeDef = Application().GetFactory().CreateICENodeDef(L"CrappyFlow",L"CrappyFlow");

	CStatus st;
	st = nodeDef.PutColor(154,203,206);
	st.AssertSucceeded( ) ;

	st = nodeDef.PutThreadingModel(XSI::siICENodeSingleThreading);
	st.AssertSucceeded( ) ;

	// Add input ports and groups.
	st = nodeDef.AddPortGroup(ID_G_100);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(ID_IN_Curves,ID_G_100,siICENodeDataGeometry,siICENodeStructureSingle,siICENodeContextSingleton,L"Curves",L"Curves",CValue(),CValue(),CValue(),ID_UNDEF,ID_STRUCT_CNS,ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(ID_IN_Width,ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"Width",L"Width",1);
	st.AssertSucceeded( );

	// Add output ports.
	st = nodeDef.AddOutputPort(ID_OUT_Vertices,siICENodeDataVector3,siICENodeStructureArray,siICENodeContextSingleton,L"Vertices",L"Vertices",ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddOutputPort(ID_OUT_Polygons,siICENodeDataLong,siICENodeStructureArray,siICENodeContextSingleton,L"Polygons",L"Polygons",ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
	nodeItem.PutCategories(L"Custom ICENode");

	return CStatus::OK;
}

// Get Dirty State
//------------------------------------------------
CFDirty CrappyFlow_GetDirtyState(ICENodeContext& in_ctxt)
{
	CFDebug(L"Crappy Flow set to always dirty...");
	return CFDirty::JustDirty;

	CICEPortState GeomState( in_ctxt, ID_IN_Curves );
	bool geomDirty = GeomState.IsDirty( CICEPortState::siAnyDirtyState );
	GeomState.ClearState();

	CICEPortState WidthState( in_ctxt, ID_IN_Width );
	bool widthDirty = WidthState.IsDirty( CICEPortState::siAnyDirtyState );
	WidthState.ClearState();
	
	if(geomDirty||widthDirty)return CFDirty::JustDirty;
	else return CFDirty::NotDirty;
}

// Evaluate
//------------------------------------------------
SICALLBACK CrappyFlow_Evaluate( ICENodeContext& in_ctxt )
{
	// Get User Data
	CFData* data = (CFData*)(CValue::siPtrType)in_ctxt.GetUserData( );
	if(data == NULL || data->_nbc == 0)
		return CStatus::OK;

	// The current output port being evaluated...
	ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );
  
	switch( out_portID )
	{		
		case ID_OUT_Vertices :
		{			
			// Get the output port array ...			
			CDataArray2DVector3f outData( in_ctxt );
			CVector3f pos;
			
			CDataArray2DVector3f::Accessor outDataSubArray = outData.Resize(0,data->_nbvertices);
			ULONG offset = 0;

			for(ULONG i=0;i<data->_nbvertices;i++)
			{
				pos = data->_vertices[i];
				outDataSubArray[i].Set(pos.GetX(),pos.GetY(),pos.GetZ());
			}
			
		}
		break;
		
		case ID_OUT_Polygons :
		{	
			// Get the output port array ...			
			CDataArray2DLong outData( in_ctxt );
			
			CDataArray2DLong::Accessor outDataSubArray = outData.Resize(0,data->_nbpolygons);

			for(ULONG i=0;i<data->_nbpolygons;i++)
			{
				outDataSubArray[i] = data->_polygons[i];
			}
		}
		break;
	};
	
	return CStatus::OK;
}

// Begin Evaluate
//------------------------------------------------
SICALLBACK CrappyFlow_BeginEvaluate( ICENodeContext& in_ctxt )
{
	// Get User Data
	CFData* data = (CFData*)(CValue::siPtrType)in_ctxt.GetUserData( );

	// Get Dirty State
	CFDirty dirty = CrappyFlow_GetDirtyState(in_ctxt);
	if(dirty == CFDirty::NotDirty)
		return CStatus::OK;

	// Get geometry object from the input port
	CICEGeometry geom( in_ctxt, ID_IN_Curves );
	ULONG nbc = geom.GetSubGeometryCount();
	if(nbc<=1)
	{
		Application().LogMessage(L"Need a curve group as input");
		return CStatus::OK;
	}

	CDataArrayFloat width( in_ctxt, ID_IN_Width );
	data->_width = width[0];
	
	data->Clear();
	data->GetData(geom);
	data->CreateCorners();
	data->CreateBranches();
	data->Build();

	return CStatus::OK;
}

// Init
//------------------------------------------------
XSIPLUGINCALLBACK CStatus CrappyFlow_Init( CRef& in_ctxt )
{
	Context ctxt(in_ctxt);
	CFData* data = new CFData();
	data->_nbc = 0;
	ctxt.PutUserData( (CValue::siPtrType)data );
	return CStatus::OK;
}

// Term
//------------------------------------------------
XSIPLUGINCALLBACK CStatus CrappyFlow_Term( CRef& in_ctxt )
{
	Context ctxt(in_ctxt);
	if(!ctxt.GetUserData().IsEmpty())
	{
		CFData* data = (CFData*)(CValue::siPtrType)ctxt.GetUserData( );
		data->Clear();
		delete data;
		ctxt.PutUserData((CValue)NULL);
	}
	return CStatus::OK;
}


