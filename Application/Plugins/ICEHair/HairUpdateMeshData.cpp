/*------------------------------------------
	HairUpdateMeshData Custom ICE Node
	this node take one HairMeshData as input 
	as well one CICEGeometry (guide geometrie) 
	and update it after simulation/deformation

															 _ _
												____________/ / \_.)_
												`.     ,'   @_@_/ ,'|
												  `-.-'        , / /|
													 `-.____,-'  | ||
															  \  \ ||
															  |  | \|
															 /   \     ,,'
															 |    \   //
															/      |  ||
															|    ,-\ //
															|   _\  \||
														   (_`-(_____)/  -Shimrod

------------------------------------------*/
#include "HairRegister.h"
#include "HairGeometry.h"
#include "HairData.h"

using namespace ICEHAIR;

// Defines port, group and map identifiers used for registering the ICENode
enum MeshDataIDs
{
	ID_IN_MeshData = 1,
	ID_IN_Geometry = 2,
	ID_G_100 = 100,
	ID_OUT_MeshData = 200,
	ID_TYPE_CNS = 400,
	ID_STRUCT_CNS,
	ID_CTXT_CNS,
	ID_UNDEF = ULONG_MAX
};

CStatus RegisterHairUpdateMeshData( PluginRegistrar& in_reg )
{
	ICENodeDef nodeDef;
	nodeDef = Application().GetFactory().CreateICENodeDef(L"HairUpdateMeshData",L"HairUpdateMeshData");

	CStatus st;
	st = nodeDef.PutColor(HairNodeR,HairNodeG,HairNodeB);
	st.AssertSucceeded( ) ;

	st = nodeDef.PutThreadingModel(XSI::siICENodeSingleThreading);
	st.AssertSucceeded( ) ;

	// Add custom types definition.
	st = nodeDef.DefineCustomType(L"HairMeshData",L"HairMeshData",L"HairMeshData",HairGuideDataR,HairGuideDataG,HairGuideDataB);
	st.AssertSucceeded( ) ;

	CStringArray HairMeshDataCustomType(1);
	HairMeshDataCustomType[0] = L"HairMeshData";

	// Add input ports and groups.
	st = nodeDef.AddPortGroup(ID_G_100);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(ID_IN_MeshData,ID_G_100,HairMeshDataCustomType,siICENodeStructureSingle,siICENodeContextSingleton,L"InMeshData",L"InMeshData",ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
	st.AssertSucceeded();

	st = nodeDef.AddInputPort(ID_IN_Geometry,ID_G_100,siICENodeDataGeometry,siICENodeStructureSingle,siICENodeContextSingleton,L"Geometry",L"Geometry",CValue(),CValue(),CValue(),ID_UNDEF,ID_UNDEF,ID_UNDEF);
	st.AssertSucceeded( ) ;

	// Add output ports.
	st = nodeDef.AddOutputPort(ID_OUT_MeshData,HairMeshDataCustomType,siICENodeStructureSingle,siICENodeContextSingleton,L"OutMeshData",L"OutMeshData",ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
	nodeItem.PutCategories(L"Hair");

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus HairUpdateMeshData_Evaluate( ICENodeContext& in_ctxt )
{
	// Get Input Mesh Data
	CDataArrayCustomType MeshData( in_ctxt, ID_IN_MeshData );

	CDataArrayCustomType::TData* pBufferMeshData;
	ULONG nSizeMeshData;
	MeshData.GetData( 0,(const CDataArrayCustomType::TData**)&pBufferMeshData, nSizeMeshData );
	HairMeshData*  data = (HairMeshData*)pBufferMeshData;

	// Get the geometry
	CICEGeometry geom( in_ctxt, ID_IN_Geometry );

	if(data->_valid)
	{
		data->_geo->GetPointPosition(geom);
		data->GetVirtualVerticesPosition();

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




