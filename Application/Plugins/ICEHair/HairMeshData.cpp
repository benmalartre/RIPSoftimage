/*------------------------------------------
	HairFromMesh Custom ICE Node
	this node take one CICEGeometry as input (guide geometrie) 
	as well as a CDataArrayLong describing Tip Polygons Cluster
	It outputs HairMeshData Custom Data Type used by other nodes...

               ,---. 
            ,.'-.   \ 
           ( ( ,'"""""-. 
           `,X          `. 
           /` `           `._ 
          (            ,   ,_\ 
          |          ,---.,'o `. 
          |         / o   \     ) 
           \ ,.    (      .____, 
            \| \    \____,'     \ 
          '`'\  \        _,____,' 
          \  ,--      ,-'     \ 
            ( C     ,'         \ 
             `--'  .'           | 
               |   |         .O | 
             __|    \        ,-'_ 
            / `L     `._  _,'  ' `. 
           /    `--.._  `',.   _\  ` 
           `-.       /\  | `. ( ,\  \ 
          _/  `-._  /  \ |--'  (     \ 
         '  `-.   `'    \/\`.   `.    ) 
               \  -hrr-    \ `.  |    | 


------------------------------------------*/
#include "HairRegister.h"
#include "HairGeometry.h"
#include "HairData.h"
#include "HairBranch.h"

using namespace ICEHAIR;

// Defines port, group and map identifiers used for registering the ICENode
enum MeshDataIDs
{
	ID_IN_Geometry = 1,
	ID_IN_TipPolygon = 2,
	ID_IN_ForceUpdate = 3,
	ID_G_100 = 100,
	ID_OUT_MeshData = 200,
	ID_TYPE_CNS = 400,
	ID_STRUCT_CNS,
	ID_CTXT_CNS,
	ID_UNDEF = ULONG_MAX
};

CStatus RegisterHairMeshData( PluginRegistrar& in_reg )
{
	ICENodeDef nodeDef;
	nodeDef = Application().GetFactory().CreateICENodeDef(L"HairMeshData",L"HairMeshData");

	CStatus st;
	st = nodeDef.PutColor(HairNodeR,HairNodeG,HairNodeB);
	st.AssertSucceeded( ) ;

	st = nodeDef.PutThreadingModel(XSI::siICENodeSingleThreading);
	st.AssertSucceeded( ) ;

	// Add custom types definition.
	st = nodeDef.DefineCustomType(L"HairMeshData",L"HairMeshData",L"HairMeshData",HairGuideDataR,HairGuideDataG,HairGuideDataB);
	st.AssertSucceeded( ) ;

	// Add input ports and groups.
	st = nodeDef.AddPortGroup(ID_G_100);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(ID_IN_Geometry,ID_G_100,siICENodeDataGeometry,siICENodeStructureSingle,siICENodeContextSingleton,L"Geometry",L"Geometry",CValue(),CValue(),CValue(),ID_UNDEF,ID_UNDEF,ID_UNDEF);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(ID_IN_TipPolygon,ID_G_100,siICENodeDataBool,siICENodeStructureSingle,siICENodeContextComponent2D,L"TipPolygon",L"TipPolygon",CValue(),CValue(),CValue(),ID_UNDEF,ID_UNDEF,ID_UNDEF);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(ID_IN_ForceUpdate,ID_G_100,siICENodeDataBool,siICENodeStructureSingle,siICENodeContextSingleton,L"ForceUpdate",L"ForceUpdate",CValue(),CValue(),CValue(),ID_UNDEF,ID_UNDEF,ID_UNDEF);
	st.AssertSucceeded( ) ;

	// Add output ports.
	CStringArray HairMeshDataCustomType(1);
	HairMeshDataCustomType[0] = L"HairMeshData";

	st = nodeDef.AddOutputPort(ID_OUT_MeshData,HairMeshDataCustomType,siICENodeStructureSingle,siICENodeContextSingleton,L"MeshData",L"MeshData",ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
	nodeItem.PutCategories(L"Hair");

	return CStatus::OK;
}

bool GetHairMeshDataDirtyState(ICENodeContext& in_ctxt)
{
	CICEPortState GeoState( in_ctxt, ID_IN_Geometry );
	bool geoDirty = GeoState.IsDirty( CICEPortState::siAnyDirtyState );
	GeoState.ClearState();

	CICEPortState TipState( in_ctxt, ID_IN_TipPolygon );
	bool tipDirty = TipState.IsDirty( CICEPortState::siAnyDirtyState );
	TipState.ClearState();

	return (geoDirty||tipDirty);
}

XSIPLUGINCALLBACK CStatus HairMeshData_Evaluate( ICENodeContext& in_ctxt )
{
	// Get User Data
	HairMeshData* data = (HairMeshData*)(CValue::siPtrType)in_ctxt.GetUserData( );

	if(data->_valid)
	{
		// The current output port being evaluated...
		LONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );

		switch( out_portID )
		{	
			case ID_OUT_MeshData :
			{
				// Get the output port array ...			
				CDataArrayCustomType outData( in_ctxt );
				
				XSI::CDataArrayCustomType::TData* pOutData = outData.Resize(0,sizeof(HairMeshData));
				::memcpy( pOutData, data, sizeof(HairMeshData) );   

			}break;
		}
	}

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus HairMeshData_BeginEvaluate( ICENodeContext& in_ctxt )
{
	// Get User Data
	HairMeshData* data = (HairMeshData*)(CValue::siPtrType)in_ctxt.GetUserData( );

	// Get the geometry
	CICEGeometry geom( in_ctxt, ID_IN_Geometry );
	bool bTopologyDirtyState = geom.IsDirty( CICEGeometry::siTopologyDirtyState );
	geom.ClearState();

	// Force Update??
	CDataArrayBool forceUpdateData(in_ctxt,ID_IN_ForceUpdate);
	bool forceUpdate = forceUpdateData[0];

	if(!data->_init || bTopologyDirtyState || forceUpdate )
	{
		data->_geo->GetCachedMeshData(geom);
		data->_geo->InitGuideData();

		// get tip polygons
		CIndexSet indexSet(in_ctxt,ID_IN_TipPolygon);
		CDataArrayBool tipPolygonData(in_ctxt,ID_IN_TipPolygon);
		data->_tippolygons.clear();
		for(CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
		{
			if(tipPolygonData[it])
			{
				data->_geo->_polygons[it]->_tip =true;
				data->_tippolygons.push_back(data->_geo->_polygons[it]);
			}
			else data->_geo->_polygons[it]->_tip =false;
		}
	
		// clear old datas
		data->ClearDatas();

		// Check geometry
		if(!data->CheckGeometry())
		{
			Application().LogMessage(L"Input Geometry contains triangles or NGons ---> Invalid State...");
			data->_valid = false;
		}
		else
		{
			data->GetTipDatas();
			data->GetTipIslands();
			data->InitGuides();
			data->GetTipBranches();
			data->GetGuideTriangles();
			data->TermGuides();
			
			data->_init = true;
			data->_valid = true;
		}
	}

	else
	{
		data->GetPointPosition(geom);
		data->GetVirtualVerticesPosition();
	}

	in_ctxt.PutUserData( (CValue::siPtrType)data );

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus HairMeshData_Init( CRef& in_ctxt )
{
	Context ctxt(in_ctxt);
	HairMeshData* data = new HairMeshData();
	data->_init = false;
	data->_security = 0;
	ctxt.PutUserData((CValue::siPtrType)data );
	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus HairMeshData_Term( CRef& in_ctxt )
{
	Context ctxt(in_ctxt);
	HairMeshData* data = (HairMeshData*)(CValue::siPtrType)ctxt.GetUserData( );
	data->_valid = false;
	delete data;
	ctxt.PutUserData((CValue)NULL);
	return CStatus::OK;
}



