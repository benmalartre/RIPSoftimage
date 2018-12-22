/*------------------------------------------------------

		HairTriangleAreaSum Custom Node


					.------------------------.
					|       PSYCHIATRIC      |
					|         HELP  5¢       |
					|________________________|
					||     .-"""--.         ||
					||    /        \.-.     ||
					||   |     ._,     \    ||
					||   \_/`-'   '-.,_/    ||
					||   (_   (' _)') \     ||
					||   /|           |\    ||
					||  | \     __   / |    ||
					||   \_).,_____,/}/     ||
				  __||____;_--'___'/ (      ||
				 |\ ||   (__,\\    \_/------||
				 ||\||______________________||
				 ||||                        |
				 ||||       THE DOCTOR       |
				 \|||         IS [IN]   _____|
				  \||                  (______)
			 jgs   `|___________________//||\\
									   //=||=\\


------------------------------------------*/
#include "HairRegister.h"
#include "HairGeometry.h"
using namespace ICEHAIR;

// Defines port, group and map identifiers used for registering the ICENode
enum TriangleAreaSumIDs
{
	ID_IN_TriangleArea = 1,
	ID_G_100 = 100,
	ID_OUT_AreaSum = 200,
	ID_TYPE_CNS = 400,
	ID_STRUCT_CNS,
	ID_CTXT_CNS,
	ID_UNDEF = ULONG_MAX
};

CStatus RegisterHairTriangleAreaSum( PluginRegistrar& in_reg )
{
	ICENodeDef nodeDef;
	nodeDef = Application().GetFactory().CreateICENodeDef(L"HairTriangleAreaSum",L"HairTriangleAreaSum");

	CStatus st;
	st = nodeDef.PutColor(HairNodeR,HairNodeG,HairNodeB);
	st.AssertSucceeded( ) ;

	// Add input ports and groups.
	st = nodeDef.AddPortGroup(ID_G_100);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(ID_IN_TriangleArea,ID_G_100,siICENodeDataFloat,siICENodeStructureArray,siICENodeContextSingleton,L"TriangleArea",L"TriangleArea",0,ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
	st.AssertSucceeded();

	// Add output ports.
	st = nodeDef.AddOutputPort(ID_OUT_AreaSum, siICENodeDataFloat,siICENodeStructureArray,siICENodeContextSingleton,L"AreaSum",L"AreaSum",ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
	nodeItem.PutCategories(L"Hair");

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus HairTriangleAreaSum_Evaluate( ICENodeContext& in_ctxt )
{
	// Get the input CDataArray2DFloat
	CDataArray2DFloat TriangleAreaData( in_ctxt, ID_IN_TriangleArea );
	CDataArray2DFloat::Accessor data = TriangleAreaData[0];
	ULONG nb = data.GetCount();

	CDataArray2DFloat outData( in_ctxt );
	CDataArray2DFloat::Accessor outAccessor = outData.Resize(0,nb);
	float sum = 0;
	for(ULONG i=0;i<nb;i++)
	{
		sum += data[i];
		outAccessor[i] = sum;
	}	

	return CStatus::OK;
}

