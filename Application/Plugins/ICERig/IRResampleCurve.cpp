/*
  IRResampleCurve

  this code was extracted from melena

*/
#include "IRRegister.h"

using namespace ICERIG;

// Defines port, group and map identifiers used for registering the ICENode
enum IRResampleCurveIDs
{
   ID_IN_Position = 0,
   ID_IN_Count = 1,
   ID_IN_Min = 2,
   ID_IN_Max = 3,
   ID_G_100 = 100,
   ID_OUT_Result = 200,
   ID_G_300 = 300,
   ID_TYPE_CNS = 400,
   ID_STRUCT_CNS,
   ID_CTXT_CNS,
   ID_UNDEF = ULONG_MAX
};

CStatus RegisterIRResampleCurve( PluginRegistrar& in_reg )
{
   ICENodeDef nodeDef;
   nodeDef = Application().GetFactory().CreateICENodeDef(L"IRResampleCurve");

   CStatus st;

	st = nodeDef.PutColor(IRNodeB,IRNodeB,IRNodeG);
	st.AssertSucceeded( );

   // Add input ports and groups.
   st = nodeDef.AddPortGroup(ID_G_100);
   st.AssertSucceeded( ) ;

   st = nodeDef.AddInputPort(ID_IN_Position,ID_G_100,siICENodeDataVector3,siICENodeStructureArray,siICENodeContextAny,L"Position",L"Position",CVector3f(1.0,1.0,1.0),ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
   st.AssertSucceeded( ) ;

   st = nodeDef.AddInputPort(ID_IN_Count,ID_G_100,siICENodeDataLong,siICENodeStructureSingle,siICENodeContextAny,L"Count",L"Count",5,ID_UNDEF,ID_STRUCT_CNS,ID_CTXT_CNS);
   st.AssertSucceeded( ) ;

   st = nodeDef.AddInputPort(ID_IN_Min,ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextAny,L"Min",L"Min",0.0f,ID_UNDEF,ID_UNDEF,ID_UNDEF);
   st.AssertSucceeded( ) ;

   st = nodeDef.AddInputPort(ID_IN_Max,ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextAny,L"Max",L"Max",1.0f,ID_UNDEF,ID_UNDEF,ID_UNDEF);
   st.AssertSucceeded( ) ;

   // Add output ports and groups.
   st = nodeDef.AddPortGroup(ID_G_300);
   st.AssertSucceeded( ) ;

   st = nodeDef.AddOutputPort(ID_OUT_Result,ID_G_300,siICENodeDataVector3,siICENodeStructureArray,siICENodeContextAny,L"Result",L"Result",ID_UNDEF,ID_UNDEF,ID_CTXT_CNS);
   st.AssertSucceeded( ) ;

   PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
   nodeItem.PutCategories(L"ICERig");

   return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus IRResampleCurve_Evaluate( ICENodeContext& in_ctxt )
{
   // The current output port being evaluated...
   ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );

   switch( out_portID )
   {
      case ID_OUT_Result:
      {
         // Get the output port array ...
         CDataArray2DVector3f outData( in_ctxt );

         // Get the input data buffers for each port
         CDataArray2DVector3f StrandPositionData( in_ctxt, ID_IN_Position );
         CDataArrayLong CountData( in_ctxt, ID_IN_Count );
         CDataArrayFloat MinData( in_ctxt, ID_IN_Min );
         CDataArrayFloat MaxData( in_ctxt, ID_IN_Max );

         // We need a CIndexSet to iterate over the data
         CIndexSet indexSet( in_ctxt );
         for(CIndexSet::Iterator it = indexSet.Begin(); it.HasNext(); it.Next())
         {
            CDataArray2DVector3f::Accessor StrandPos = StrandPositionData[it];

            // make sure to tackle negative counts
            if(CountData[it] <= 0 || StrandPos.GetCount() <= 1)
            {
               outData.Resize(it,0);
               continue;
            }

            // clamp the range
            if(MinData[it] < 0)MinData[it] = 0;
            if(MaxData[it] < 0)MaxData[it] = 0;
            if(MinData[it] > 1)MinData[it] = 1;
            if(MaxData[it] > 1)MaxData[it] = 1;
            if(MinData[it] > MaxData[it])
            {
               float temp = MinData[it];
               MinData[it] = MaxData[it];
               MaxData[it] = temp;
            }


            // measure the length of the strand
            CDoubleArray StrandLength (StrandPos.GetCount()-1);
            CDoubleArray StrandLengthAcc (StrandPos.GetCount()-1);
            double StrandLengthSum = 0;
            for(long i=0;i<StrandLength.GetCount();i++)
            {
               CVector3f Segment;
               Segment.Sub(StrandPos[i+1],StrandPos[i]);
               StrandLength[i] = Segment.GetLength();
               StrandLengthSum += StrandLength[i];
               StrandLengthAcc[i] = StrandLengthSum;
            }

            double SegmentLength = (MaxData[it]-MinData[it]) * StrandLengthSum / float(CountData[it]-1);

            // find the first index to start with...
            double currentLength = 0;
            long firstIndex = 0;
            while(currentLength < MinData[it] * StrandLengthSum)
            {
               currentLength += StrandLength[firstIndex];
               firstIndex++;
            }
            if(currentLength > 0)
               currentLength -= StrandLength[firstIndex];
            if(firstIndex>0)
               firstIndex--;

            // Set the output data buffer, etc...
            outData.Resize(it,CountData[it]);

            // walk the strand and place the new points
            for(long i=0;i<CountData[it];i++)
            {
               double acc = (firstIndex == 0) ? 0.0f : StrandLengthAcc[firstIndex-1];
               double ratio = (currentLength - acc) / StrandLength[firstIndex];
               if(ratio > 1)ratio = 1;
               if(ratio < 0)ratio = 0;
               outData[it][i].LinearlyInterpolate(StrandPos[firstIndex],StrandPos[firstIndex+1],(float)ratio);

               currentLength += SegmentLength;
               acc = StrandLengthAcc[firstIndex];
               while(currentLength > acc && firstIndex < (long)StrandPos.GetCount() - 2)
               {
                  acc = StrandLengthAcc[++firstIndex];
               }
            }
         }
      }
      break;

      // Other output ports...

   };

   return CStatus::OK;
}