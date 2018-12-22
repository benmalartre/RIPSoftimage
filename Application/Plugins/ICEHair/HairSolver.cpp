/*
  HairSolver

  this code was extracted from melena

													oooo$$$$$$$$$$$$oooo
											  oo$$$$$$$$$$$$$$$$$$$$$$$$o
										   oo$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$o         o$   $$ o$
						   o $ oo        o$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$o       $$ $$ $$o$
						oo $ $ "$      o$$$$$$$$$    $$$$$$$$$$$$$    $$$$$$$$$o       $$$o$$o$
						"$$$$$$o$     o$$$$$$$$$      $$$$$$$$$$$      $$$$$$$$$$o    $$$$$$$$
						  $$$$$$$    $$$$$$$$$$$      $$$$$$$$$$$      $$$$$$$$$$$$$$$$$$$$$$$
						  $$$$$$$$$$$$$$$$$$$$$$$    $$$$$$$$$$$$$    $$$$$$$$$$$$$$  """$$$
						   "$$$""""$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$     "$$$
							$$$   o$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$     "$$$o
						   o$$"   $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$       $$$o
						   $$$    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" "$$$$$$ooooo$$$$o
						  o$$$oooo$$$$$  $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$   o$$$$$$$$$$$$$$$$$
						  $$$$$$$$"$$$$   $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$     $$$$""""""""
						 """"       $$$$    "$$$$$$$$$$$$$$$$$$$$$$$$$$$$"      o$$$
									"$$$o     """$$$$$$$$$$$$$$$$$$"$$"         $$$
									  $$$o          "$$""$$$$$$""""           o$$$
									   $$$$o                                o$$$"
										"$$$$o      o$$$$$$o"$$$$o        o$$$$
										  "$$$$$oo     ""$$$$o$$$$$o   o$$$$""
											 ""$$$$$oooo  "$$$o$$$$$$$$$"""
												""$$$$$$$oo $$$$$$$$$$
														""""$$$$$$$$$$$
															$$$$$$$$$$$$
															 $$$$$$$$$$"
															  "$$$""  

*/
#include "HairRegister.h"
#include "HairGeometry.h"

using namespace ICEHAIR;

// Defines port, group and map identifiers used for registering the ICENode
enum SolverIDs
{
   ID_IN_RootPosition = 1,
   ID_IN_StrandPosition = 2,
   ID_IN_StrandVelocity = 3,
   ID_IN_StrandLength = 4,
   ID_G_100 = 100,
   ID_OUT_StrandPosition = 200,
   ID_G_300 = 300,
   ID_TYPE_CNS = 400,
   ID_STRUCT_CNS,
   ID_CTXT_CNS,
   ID_UNDEF = ULONG_MAX
};

CStatus RegisterHairSolver( PluginRegistrar& in_reg )
{
   ICENodeDef nodeDef;
   nodeDef = Application().GetFactory().CreateICENodeDef(L"HairSolver");

   CStatus st;
   st = nodeDef.PutColor(HairNodeR,HairNodeG,HairNodeB);
   st.AssertSucceeded( );

   // Add input ports and groups.
   st = nodeDef.AddPortGroup(ID_G_100);
   st.AssertSucceeded( ) ;

   st = nodeDef.AddInputPort(ID_IN_RootPosition,ID_G_100,siICENodeDataVector3,siICENodeStructureSingle,siICENodeContextAny,L"In_RootPosition",L"In_RootPosition",CVector3f(1.0,1.0,1.0),ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
   st.AssertSucceeded( ) ;

   st = nodeDef.AddInputPort(ID_IN_StrandPosition,ID_G_100,siICENodeDataVector3,siICENodeStructureArray,siICENodeContextAny,L"In_StrandPosition",L"In_StrandPosition",CVector3f(1.0,1.0,1.0),ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
   st.AssertSucceeded( ) ;

   st = nodeDef.AddInputPort(ID_IN_StrandVelocity,ID_G_100,siICENodeDataVector3,siICENodeStructureArray,siICENodeContextAny,L"In_StrandVelocity",L"In_StrandVelocity",CVector3f(1.0,1.0,1.0),ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
   st.AssertSucceeded( ) ;

   st = nodeDef.AddInputPort(ID_IN_StrandLength,ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextAny,L"In_StrandLength",L"In_StrandLength",1,ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
   st.AssertSucceeded( ) ;

   // Add output ports and groups.
   st = nodeDef.AddPortGroup(ID_G_300);
   st.AssertSucceeded( ) ;

   st = nodeDef.AddOutputPort(ID_OUT_StrandPosition,ID_G_300,siICENodeDataVector3,siICENodeStructureArray,siICENodeContextAny,L"Out_StrandPosition",L"Out_StrandPosition",ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
   st.AssertSucceeded( ) ;

   PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
   nodeItem.PutCategories(L"Hair");

   return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus HairSolver_Evaluate( ICENodeContext& in_ctxt )
{
   // The current output port being evaluated...
   ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );

   switch( out_portID )
   {
      case ID_OUT_StrandPosition:
      {
         // Get the output port array ...
         CDataArray2DVector3f outData( in_ctxt );

         // Get the input data buffers for each port
         CDataArrayVector3f In_RootPositionData( in_ctxt, ID_IN_RootPosition );
         CDataArray2DVector3f In_StrandPositionData( in_ctxt, ID_IN_StrandPosition );
         CDataArray2DVector3f In_StrandVelocityData( in_ctxt, ID_IN_StrandVelocity );
         CDataArrayFloat In_StrandLengthData( in_ctxt, ID_IN_StrandLength );

         // We need a CIndexSet to iterate over the data
         CIndexSet indexSet( in_ctxt );

         for(CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
         {
            // Set the output data buffer, etc...
            CDataArray2DVector3f::Accessor In_StrandPositionDataSubArray = In_StrandPositionData[it];
            CDataArray2DVector3f::Accessor In_StrandVelocityDataSubArray = In_StrandVelocityData[it];
            ULONG l_ulCount = In_StrandPositionDataSubArray.GetCount();

            CDataArray2DVector3f::Accessor outDataSubArray = outData.Resize(it,l_ulCount);

            CVector3f l_vPreviousPos = In_RootPositionData[it];
            float l_fLength = In_StrandLengthData[it];
            l_fLength /= (float)(l_ulCount-1);
            outDataSubArray[0] = l_vPreviousPos;

            for(ULONG i=1;i<l_ulCount;i++)
            {
               CVector3f l_vPos;
               l_vPos.Add(In_StrandPositionDataSubArray[i],In_StrandVelocityDataSubArray[i]);

               l_vPos.Sub(l_vPos,l_vPreviousPos);
               l_vPos.NormalizeInPlace();
               l_vPos.ScaleInPlace(l_fLength);
               l_vPos.AddInPlace(l_vPreviousPos);

               outDataSubArray[i] = l_vPos;
               l_vPreviousPos = l_vPos;
            }
         }
      }
      break;

   };

   return CStatus::OK;
}