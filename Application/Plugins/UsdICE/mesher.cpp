//Toonix Mesher
//------------------------------------------------
#include "ToonixRegister.h"
#include "ToonixGeometry.h"
#include "ToonixLine.h"
#include "ToonixMesh.h"

// Defines port, group and map identifiers used for registering the ICENode
enum IDs
{
	ID_IN_ToonixLine = 0,
	ID_IN_Subdiv = 1,
	ID_IN_ViewPosition = 2,
	ID_G_100 = 100,
	ID_OUT_Vertices= 201,
	ID_OUT_Polygons=202,
	ID_TYPE_CNS = 400,
	ID_STRUCT_CNS,
	ID_CTXT_CNS,
	ID_UNDEF = ULONG_MAX
};

bool GetMesherDirtyState(ICENodeContext& in_ctxt )
{
	CICEPortState LineState( in_ctxt, ID_IN_ToonixLine );
	bool lineDirty = LineState.IsDirty( CICEPortState::siAnyDirtyState );
	LineState.ClearState();

	CICEPortState SubdivState( in_ctxt, ID_IN_Subdiv );
	bool subdivDirty = SubdivState.IsDirty( CICEPortState::siAnyDirtyState );
	SubdivState.ClearState();

	CICEPortState ViewState( in_ctxt, ID_IN_ViewPosition );
	bool viewDirty = ViewState.IsDirty( CICEPortState::siAnyDirtyState );
	ViewState.ClearState();

	return(lineDirty||subdivDirty||viewDirty);
}


CStatus RegisterToonixMesher( PluginRegistrar& in_reg )
{
	ICENodeDef nodeDef;
	nodeDef = Application().GetFactory().CreateICENodeDef(L"ToonixMesher",L"ToonixMesher");

	CStatus st;
	st = nodeDef.PutColor(ToonixNodeR,ToonixNodeG,ToonixNodeB);
	st.AssertSucceeded( ) ;

	st = nodeDef.PutThreadingModel(siICENodeSingleThreading);
	st.AssertSucceeded( ) ;

	// Add custom types definition.
	st = nodeDef.DefineCustomType(L"ToonixLine",L"ToonixLine",L"ToonixLine",ToonixLineR,ToonixLineG,ToonixLineB);
	st.AssertSucceeded( ) ;

	// Add input ports and groups.
	st = nodeDef.AddPortGroup(ID_G_100);
	st.AssertSucceeded( ) ;

	CStringArray ToonixLineCustomType(1);
	ToonixLineCustomType[0] = L"ToonixLine";

	st = nodeDef.AddInputPort(ID_IN_ToonixLine,ID_G_100,ToonixLineCustomType,siICENodeStructureSingle,siICENodeContextSingleton,L"ToonixLine",L"ToonixLine",ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(ID_IN_Subdiv,ID_G_100,siICENodeDataLong,siICENodeStructureSingle,siICENodeContextSingleton,L"Subdivisions",L"Subdivisions",6);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(ID_IN_ViewPosition,ID_G_100,siICENodeDataVector3,siICENodeStructureSingle,siICENodeContextSingleton,L"ViewPosition",L"ViewPosition");
	st.AssertSucceeded( ) ;

	// Add output ports.
	st = nodeDef.AddOutputPort(ID_OUT_Vertices,siICENodeDataVector3,siICENodeStructureArray,siICENodeContextSingleton,L"Vertices",L"Vertices",ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddOutputPort(ID_OUT_Polygons,siICENodeDataLong,siICENodeStructureArray,siICENodeContextSingleton,L"Polygons",L"Polygons",ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
	nodeItem.PutCategories(L"Toonix");

	return CStatus::OK;
}

SICALLBACK ToonixMesher_Evaluate( ICENodeContext& in_ctxt )
{
	TXMesh* mesh = (TXMesh*)(CValue::siPtrType)in_ctxt.GetUserData( );

	if(!mesh->m_valid || GetMesherDirtyState(in_ctxt))
	{
		// Get the input TXLine
		CDataArrayCustomType ToonixLine( in_ctxt, ID_IN_ToonixLine );

		CDataArrayCustomType::TData* pBufferToonixLine;
		ULONG nSizeToonixLine;
		ToonixLine.GetData( 0,(const CDataArrayCustomType::TData**)&pBufferToonixLine, nSizeToonixLine );
		TXLine* line = (TXLine*)pBufferToonixLine;
		if(!line)
		{
			mesh->m_nbv = mesh->m_nbp = 0;
		}
		else
		{
			mesh->m_line = line;

			CDataArrayLong subdivData(in_ctxt, ID_IN_Subdiv);
			mesh->m_subdiv = subdivData[0];
			CDataArrayVector3f viewData(in_ctxt, ID_IN_ViewPosition);
			mesh->m_view = viewData[0];
			mesh->Build();
		}
		mesh->m_valid = true;
	}

	// The current output port being evaluated...
	LONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );

	switch( out_portID )
	{	
		case ID_OUT_Vertices:
		{
			// Get the output port array ...			
			CDataArray2DVector3f outData( in_ctxt );

			CDataArray2DVector3f::Accessor outDataSubArray = outData.Resize(0,mesh->m_nbv);
			for(ULONG n=0;n<mesh->m_nbv;n++)
			{
				outDataSubArray[n].Set(mesh->m_vertices[n].GetX(),mesh->m_vertices[n].GetY(),mesh->m_vertices[n].GetZ());
			}
			
		}
		break;

		case ID_OUT_Polygons:
		{
			// Get the output port array ...			
			CDataArray2DLong outData( in_ctxt );	

			CDataArray2DLong::Accessor outDataSubArray = outData.Resize(0,mesh->m_nbp);
			for(ULONG n=0;n<mesh->m_nbp;n++)
			{
				outDataSubArray[n] = mesh->m_polygons[n];
			}
		}
		break;
	}
	return CStatus::OK;
}

SICALLBACK ToonixMesher_Init( CRef& in_ctxt )
{
	Context ctxt(in_ctxt);
	TXMesh* mesh = new TXMesh();
	mesh->m_valid = false;
	ctxt.PutUserData((CValue::siPtrType)mesh);
	return CStatus::OK;
}

SICALLBACK ToonixMesher_Term( CRef& in_ctxt )
{
	Context ctxt(in_ctxt);
	if(!ctxt.GetUserData().IsEmpty())
	{
		TXMesh* mesh = (TXMesh*)(CValue::siPtrType)ctxt.GetUserData( );
		delete mesh;
		ctxt.PutUserData((CValue::siPtrType)NULL);
	}
	return CStatus::OK;
}
