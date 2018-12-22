// Synthox Instrument.cpp
//--------------------------------------------------------------
#include "SynthoxRegister.h"
#include "SynthoxInstrument.h"

CStatus RegisterSynthoxInstrument( PluginRegistrar& in_reg )
{
	ICENodeDef nodeDef;
	nodeDef = Application().GetFactory().CreateICENodeDef(L"SynthoxInstrument",L"SynthoxInstrument");

	CStatus st;
	st = nodeDef.PutColor(200,200,10);
	st.AssertSucceeded();

	// Define the Instrument custom type.
	st = nodeDef.DefineCustomType(	L"SynthoxInstrument",L"Synthox Instrument", L"Handle Instrument Data",216,255,87);

	st = nodeDef.PutThreadingModel(XSI::siICENodeSingleThreading);
	st.AssertSucceeded() ;

	// Add input ports and groups.
	st = nodeDef.AddPortGroup(Instrument_ID_G_100);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(Instrument_ID_IN_Enable,Instrument_ID_G_100,siICENodeDataBool,siICENodeStructureSingle,siICENodeContextSingleton,L"Enable",L"Enable",true);
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(Instrument_ID_IN_Volume,Instrument_ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"Volume",L"Volume",1.0);
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(Instrument_ID_IN_Note,Instrument_ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"Note",L"Note",50.0);
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(Instrument_ID_IN_NoteOn,Instrument_ID_G_100,siICENodeDataBool,siICENodeStructureSingle,siICENodeContextSingleton,L"NoteOn",L"NoteOn",false);
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(Instrument_ID_IN_Instrument,Instrument_ID_G_100,siICENodeDataLong,siICENodeStructureSingle,siICENodeContextSingleton,L"Instrument",L"Instrument",9);
	st.AssertSucceeded( ) ;

	// Add output ports.
	CStringArray OutputCustomType(1);
	OutputCustomType[0] = L"SynthoxInstrument";

	st = nodeDef.AddOutputPort(Instrument_ID_OUT_Output,OutputCustomType,siICENodeStructureSingle,siICENodeContextSingleton,L"Output",L"Output",Instrument_ID_UNDEF,Instrument_ID_UNDEF,Instrument_ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
	nodeItem.PutCategories(L"Custom ICENode");

	return CStatus::OK;
}

SICALLBACK SynthoxInstrument_Evaluate( ICENodeContext& in_ctxt )
{
	// Get the user data that we allocated in BeginEvaluate
	InstrumentData* data = (InstrumentData*)(CValue::siPtrType)in_ctxt.GetUserData( );

	// The current output port being evaluated...
	ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );
  
	switch( out_portID )
	{	
		Application().LogMessage(L"Output called...");
		case Instrument_ID_OUT_Output :
		{
			//Enable
			CDataArrayBool enableData(in_ctxt, Instrument_ID_IN_Enable);
			data->enable = enableData[0];

			if(data->enable)
			{
				//Volume
				CDataArrayFloat volumeData(in_ctxt, Instrument_ID_IN_Volume);
				data->volume = volumeData[0];

				//Note
				CDataArrayFloat noteData(in_ctxt, Instrument_ID_IN_Note);
				data->note = noteData[0];

				CDataArrayBool noteOnData(in_ctxt, Instrument_ID_IN_NoteOn);
				data->noteOn = noteOnData[0];

				//Instrument
				CDataArrayLong instruData(in_ctxt, Instrument_ID_IN_Instrument);
				ULONG instrumentCurrent = instruData[0];
				if(data->instrumentCurrent != instrumentCurrent)
					data->changeInstrument = true;
				else
					data->changeInstrument = false;
				data->instrumentCurrent = instrumentCurrent;
			}

			//Output to custom data
			CDataArrayCustomType outData( in_ctxt );
			CDataArrayCustomType::TData* pOutData = outData.Resize( 0, sizeof(InstrumentData) );
			::memcpy( pOutData, data, sizeof(InstrumentData) );
		}
		break;

	};
	
	return CStatus::OK;
}

SICALLBACK SynthoxInstrument_Init( CRef& in_ctxt )
{
	ICENodeContext ctxt( in_ctxt );
	InstrumentData* data = new InstrumentData();
	
	ctxt.PutUserData( data );
	return CStatus::OK;
}

SICALLBACK SynthoxInstrument_Term( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	// Release memory allocated in BeginEvaluate
	CValue userData = ctxt.GetUserData( );
	if ( userData.IsEmpty() )
	{
		return CStatus::OK;
	}

	InstrumentData* data = (InstrumentData*)(CValue::siPtrType)userData;

	delete data;

	// Clear user data; 
	ctxt.PutUserData( CValue() );
	return CStatus::OK;
}