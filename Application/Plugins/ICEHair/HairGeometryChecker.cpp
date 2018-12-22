/*------------------------------------------
	HairGeometryChecker Custom ICE Node
	this node take one CICEGeometry as input (guide geometrie) 
	as well as a CDataArrayLong describing Tip Polygons Cluster
	This node checks if geometry is valid for hair generation

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
enum CheckerDataIDs
{
	ID_IN_Geometry = 1,
	ID_IN_TipPolygon = 2,
	ID_G_100 = 100,
	ID_OUT_Valid = 200,
	ID_TYPE_CNS = 400,
	ID_STRUCT_CNS,
	ID_CTXT_CNS,
	ID_UNDEF = ULONG_MAX
};

CStatus RegisterHairGeometryChecker( PluginRegistrar& in_reg )
{
	ICENodeDef nodeDef;
	nodeDef = Application().GetFactory().CreateICENodeDef(L"HairGeometryChecker",L"HairGeometryChecker");

	CStatus st;
	st = nodeDef.PutColor(HairNodeR,HairNodeG,HairNodeB);
	st.AssertSucceeded( ) ;

	st = nodeDef.PutThreadingModel(XSI::siICENodeSingleThreading);
	st.AssertSucceeded( ) ;


	// Add input ports and groups.
	st = nodeDef.AddPortGroup(ID_G_100);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(ID_IN_Geometry,ID_G_100,siICENodeDataGeometry,siICENodeStructureSingle,siICENodeContextSingleton,L"Geometry",L"Geometry",CValue(),CValue(),CValue(),ID_UNDEF,ID_UNDEF,ID_UNDEF);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(ID_IN_TipPolygon,ID_G_100,siICENodeDataBool,siICENodeStructureSingle,siICENodeContextComponent2D,L"TipPolygon",L"TipPolygon",CValue(),CValue(),CValue(),ID_UNDEF,ID_UNDEF,ID_UNDEF);
	st.AssertSucceeded( ) ;

	// Add output ports.
	st = nodeDef.AddOutputPort(ID_OUT_Valid,siICENodeDataBool,siICENodeStructureSingle,siICENodeContextSingleton,L"Valid",L"Valid",ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
	nodeItem.PutCategories(L"Hair");

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus HairGeometryChecker_Evaluate( ICENodeContext& in_ctxt )
{
	HairMeshData* data = new HairMeshData();
	data->_init = false;

	CICEGeometry geom( in_ctxt, ID_IN_Geometry );

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
		data->_valid = true;
		data->GetTipDatas();
		data->GetTipIslands();
		data->InitGuides();
		data->GetTipBranches();
	}

	CDataArrayBool outData( in_ctxt);
	outData.Set(0,data->_valid);

	delete data;
	return CStatus::OK;
}


