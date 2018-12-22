/*------------------------------------------------------

		HairGuideTriangle Custom Node

    ___  ___  ___  ___  ___.---------------.
  .'\__\'\__\'\__\'\__\'\__,`   .  ____ ___ \
  |\/ __\/ __\/ __\/ __\/ _:\   |`.  \  \___ \
   \\'\__\'\__\'\__\'\__\'\_`.__|""`. \  \___ \
    \\/ __\/ __\/ __\/ __\/ __:                \
     \\'\__\'\__\'\__\ \__\'\_;-----------------`
      \\/   \/   \/   \/   \/ :               hh|
       \|______________________;________________| 


------------------------------------------*/
#include "HairRegister.h"
#include "HairData.h"

using namespace ICEHAIR;

// Defines port, group and map identifiers used for registering the ICENode
enum GuideTriangleIDs
{
	ID_IN_MeshData = 1,
	ID_G_100 = 100,
	ID_OUT_GuideTriangle = 200,
	ID_OUT_TriangleArea = 201,
	ID_TYPE_CNS = 400,
	ID_STRUCT_CNS,
	ID_CTXT_CNS,
	ID_UNDEF = ULONG_MAX
};

CStatus RegisterHairGuideTriangle( PluginRegistrar& in_reg )
{
	ICENodeDef nodeDef;
	nodeDef = Application().GetFactory().CreateICENodeDef(L"HairGuideTriangle",L"HairGuideTriangle");

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
	st = nodeDef.AddOutputPort(ID_OUT_GuideTriangle, siICENodeDataLong,siICENodeStructureArray,siICENodeContextSingleton,L"GuideTriangle",L"GuideTriangle");
	st.AssertSucceeded( ) ;

	st = nodeDef.AddOutputPort(ID_OUT_TriangleArea, siICENodeDataFloat,siICENodeStructureArray,siICENodeContextSingleton,L"TriangleArea",L"TriangleArea");
	st.AssertSucceeded( ) ;

	PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
	nodeItem.PutCategories(L"Hair"); 

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus HairGuideTriangle_Evaluate( ICENodeContext& in_ctxt )
{
	// Get the input HairMeshData
	CDataArrayCustomType MeshData( in_ctxt, ID_IN_MeshData );

	CDataArrayCustomType::TData* pBufferMeshData;
	ULONG nSizeMeshData;
	MeshData.GetData( 0,(const CDataArrayCustomType::TData**)&pBufferMeshData, nSizeMeshData );
	HairMeshData*  data = (HairMeshData*)pBufferMeshData;

	// The current output port being evaluated...
	LONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );

	switch( out_portID )
	{	
		// Guide Triangle Indices
		case ID_OUT_GuideTriangle :
		{
			CDataArray2DLong outData( in_ctxt );
			if(!data->_valid)
				CDataArray2DLong::Accessor outAccessor = outData.Resize(0,0);
			else
			{
				ULONG nbg = (ULONG)data->_guidetriangles.size();
				CDataArray2DLong::Accessor outAccessor = outData.Resize(0,nbg);

				for(ULONG i=0;i<nbg;i++)
				{
					outAccessor[i] = data->_guidetriangles[i];
				}
			}
		}break;

		// Triangle Area
		case ID_OUT_TriangleArea :
		{
			CDataArray2DFloat outData( in_ctxt );
			if(!data->_valid)
				CDataArray2DFloat::Accessor outAccessor = outData.Resize(0,0);
			else
			{
				ULONG nba = (ULONG)data->_trianglesarea.size();
				CDataArray2DFloat::Accessor outAccessor = outData.Resize(0,nba);

				for(ULONG i=0;i<nba;i++)
				{
					outAccessor[i] = data->_trianglesarea[i];
				}
			}
		}break;
	}

	return CStatus::OK;
}

