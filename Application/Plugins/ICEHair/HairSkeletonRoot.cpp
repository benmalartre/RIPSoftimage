// HairSkeletonRoot Custom Node
//------------------------------------------
#include "HairRegister.h"
#include "HairGeometry.h"
#include "HairData.h"

using namespace ICEHAIR;

// Defines port, group and map identifiers used for registering the ICENode
enum SkeletonRootIDs
{
	ID_IN_MeshData = 1,
	ID_G_100 = 100,
	ID_OUT_Position = 200,
	ID_TYPE_CNS = 400,
	ID_STRUCT_CNS,
	ID_CTXT_CNS,
	ID_UNDEF = ULONG_MAX
};

struct SkeletonRootData
{
	std::vector<CVector3f> _pos;
	ULONG _nbp;
};

CStatus RegisterHairSkeletonRoot( PluginRegistrar& in_reg )
{
	ICENodeDef nodeDef;
	nodeDef = Application().GetFactory().CreateICENodeDef(L"HairSkeletonRoot",L"HairSkeletonRoot");

	CStatus st;
	st = nodeDef.PutColor(HairNodeR,HairNodeG,HairNodeB);
	st.AssertSucceeded( ) ;

	// Add custom types definition.
	st = nodeDef.DefineCustomType(L"HairMeshData",L"HairMeshData",L"HairMeshData",HairGuideDataR,HairGuideDataG,HairGuideDataB);
	st.AssertSucceeded( ) ;

	// Add input ports and groups.
	st = nodeDef.AddPortGroup(ID_G_100);
	st.AssertSucceeded( ) ;

	CStringArray EmitRootDataCustomType(1);
	EmitRootDataCustomType[0] = L"HairMeshData";

	st = nodeDef.AddInputPort(ID_IN_MeshData,ID_G_100,EmitRootDataCustomType,siICENodeStructureSingle,siICENodeContextSingleton,L"MeshData",L"MeshData",ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
	st.AssertSucceeded();
	
	// Add output ports.
	st = nodeDef.AddOutputPort(ID_OUT_Position, siICENodeDataVector3,siICENodeStructureSingle,siICENodeContextElementGenerator,L"RootPosition",L"RootPosition");
	st.AssertSucceeded( ) ;


	PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
	nodeItem.PutCategories(L"Hair");

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus HairSkeletonRoot_Evaluate( ICENodeContext& in_ctxt )
{
	// Get User Data
	SkeletonRootData* data = (SkeletonRootData*)(CValue::siPtrType)in_ctxt.GetUserData( );
	IHGuide* guide = NULL;

	// The current output port being evaluated...
	LONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );

	switch( out_portID )
	{	
		case ID_OUT_Position :
		{
			CDataArrayVector3f outData( in_ctxt );
			
			CIndexSet indexSet( in_ctxt );
			for(CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
			{
				outData[it] = data->_pos[it.GetAbsoluteIndex()]; 
			}
		
		}break;
	}

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus HairSkeletonRoot_BeginEvaluate( ICENodeContext& in_ctxt )
{
	SkeletonRootData* root = (SkeletonRootData*)(CValue::siPtrType)in_ctxt.GetUserData( );

	// Get the input HairMeshData
	CDataArrayCustomType MeshData( in_ctxt, ID_IN_MeshData );

	CDataArrayCustomType::TData* pBufferMeshData;
	ULONG nSizeMeshData;
	CStatus status = MeshData.GetData( 0,(const CDataArrayCustomType::TData**)&pBufferMeshData, nSizeMeshData );
	status.AssertSucceeded();

	HairMeshData*  data = (HairMeshData*)pBufferMeshData;
	
	if(!data->_valid)
		in_ctxt.PutNumberOfElementsToProcess(0);

	else
	{
		root->_nbp = (ULONG)data->_guides.size();
		root->_pos.resize(root->_nbp);

		for(ULONG i=0;i<root->_nbp;i++)
		{
			root->_pos[i] = data->_guides[i]->_vertices.front()->_initpos;
		}

		in_ctxt.PutNumberOfElementsToProcess(root->_nbp);
	}

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus HairSkeletonRoot_Init( CRef& in_ctxt )
{
	Context ctxt(in_ctxt);
	SkeletonRootData* data = new SkeletonRootData();
	ctxt.PutUserData((CValue::siPtrType)data );
	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus HairSkeletonRoot_Term( CRef& in_ctxt )
{
	Context ctxt(in_ctxt);
	SkeletonRootData* data = (SkeletonRootData*)(CValue::siPtrType)ctxt.GetUserData( );
	delete data;
	ctxt.PutUserData((CValue)NULL);
	return CStatus::OK;
}



