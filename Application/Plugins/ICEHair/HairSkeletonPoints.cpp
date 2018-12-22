// HairSkeletonPoints Custom Node
//------------------------------------------
#include "HairRegister.h"
#include "HairGeometry.h"
#include "HairData.h"

using namespace ICEHAIR;

// Defines port, group and map identifiers used for registering the ICENode
enum SkeletonPointsIDs
{
	ID_IN_MeshData = 1,
	ID_IN_PointID = 2,
	ID_G_100 = 100,
	ID_OUT_Points = 200,
	ID_OUT_Root = 201,
	ID_TYPE_CNS = 400,
	ID_STRUCT_CNS,
	ID_CTXT_CNS,
	ID_UNDEF = ULONG_MAX
};

CStatus RegisterHairSkeletonPoints( PluginRegistrar& in_reg )
{
	ICENodeDef nodeDef;
	nodeDef = Application().GetFactory().CreateICENodeDef(L"HairSkeletonPoints",L"HairSkeletonPoints");

	CStatus st;
	st = nodeDef.PutColor(HairNodeR,HairNodeG,HairNodeB);
	st.AssertSucceeded( );

	// Add custom types definition.
	st = nodeDef.DefineCustomType(L"HairMeshData",L"HairMeshData",L"HairMeshData",HairGuideDataR,HairGuideDataG,HairGuideDataB);
	st.AssertSucceeded( ) ;

	// Add input ports and groups.
	st = nodeDef.AddPortGroup(ID_G_100);
	st.AssertSucceeded( ) ;

	CStringArray EmitStrandsDataCustomType(1);
	EmitStrandsDataCustomType[0] = L"HairMeshData";

	st = nodeDef.AddInputPort(ID_IN_MeshData,ID_G_100,EmitStrandsDataCustomType,siICENodeStructureSingle,siICENodeContextSingleton,L"MeshData",L"MeshData",ID_UNDEF,ID_UNDEF,ID_UNDEF);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(ID_IN_PointID,ID_G_100,siICENodeDataLong,siICENodeStructureSingle,siICENodeContextComponent0D,L"PointID",L"PointID",MATH::CVector3f(1.0,1.0,1.0),ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	// Add output ports.
	st = nodeDef.AddOutputPort(ID_OUT_Points,siICENodeDataVector3,siICENodeStructureArray,siICENodeContextComponent0D,L"Points",L"Points",ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddOutputPort(ID_OUT_Root,siICENodeDataVector3,siICENodeStructureSingle,siICENodeContextComponent0D,L"Root",L"Root",ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
	nodeItem.PutCategories(L"Hair");

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus HairSkeletonPoints_Evaluate( ICENodeContext& in_ctxt )
{
	// Get the input HairMeshData
	CDataArrayCustomType MeshData( in_ctxt, ID_IN_MeshData );

	CDataArrayCustomType::TData* pBufferMeshData;
	ULONG nSizeMeshData;
	MeshData.GetData( 0,(const CDataArrayCustomType::TData**)&pBufferMeshData, nSizeMeshData );
	HairMeshData*  data = (HairMeshData*)pBufferMeshData;
	if(!data->_valid || pBufferMeshData == NULL)return CStatus::OK;

	// The current output port being evaluated...
	LONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );
	CDataArrayLong PointIDData(in_ctxt,ID_IN_PointID);

	IHGuide* guide;
	ULONG id, ti;
	bool first;
	CVector3f p;

	switch( out_portID )
	{	
		case ID_OUT_Root :
		{
			CDataArrayVector3f outData( in_ctxt );
			
			CIndexSet indexSet( in_ctxt );
			for(CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
			{
				// Creates an Accessor object to iterate over the 2D array data
				// Output arrays must always be initialized first
				id = PointIDData[it];
				
				guide = data->_guides[id];
				outData[it] = guide->_vertices.front()->_pos;
			}
		}break;
		

		case ID_OUT_Points :
		{
			CDataArray2DVector3f outData( in_ctxt );
			
			// We need a CIndexSet to iterate over the data
			CIndexSet indexSet( in_ctxt);
			for(CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
			{
				// Creates an Accessor object to iterate over the 2D array data
				// Output arrays must always be initialized first
				id = PointIDData[it];
				
				guide = data->_guides[id];
				ti = 0;
				first = true;
			
				ULONG u = (ULONG)guide->_vertices.size()-1;
				
				CDataArray2DVector3f::Accessor outAccessor = outData.Resize( it, u);
				std::vector<IHVertex*>::iterator v;
				for( v = guide->_vertices.begin(); v < guide->_vertices.end(); ++v)
				{
					//Skip first segment first point
					if(first){first=false;continue;}
					else
					{
						p = (*v)->_pos;
						outAccessor[ti].Set(p.GetX(),p.GetY(),p.GetZ());
						ti++;
					}
				}
			}
			
		}break;
	}

	return CStatus::OK;
}



