// bSynthNode Plugin
// an attempt to generate sounds from inside xsi
// using STK
//------------------------------------------------------------

//Softimage SDK Includes
//------------------------------------------------------------
#include <xsi_application.h>
#include <xsi_context.h>
#include <xsi_pluginregistrar.h>
#include <xsi_status.h>

#include <xsi_icenodecontext.h>
#include <xsi_icenodedef.h>
#include <xsi_command.h>
#include <xsi_factory.h>
#include <xsi_longarray.h>
#include <xsi_doublearray.h>
#include <xsi_math.h>
#include <xsi_vector2f.h>
#include <xsi_vector3f.h>
#include <xsi_vector4f.h>
#include <xsi_matrix3f.h>
#include <xsi_matrix4f.h>
#include <xsi_rotationf.h>
#include <xsi_quaternionf.h>
#include <xsi_color4f.h>
#include <xsi_shape.h>
#include <xsi_icegeometry.h>
#include <xsi_iceportstate.h>
#include <xsi_indexset.h>
#include <xsi_dataarray.h>
#include <xsi_dataarray2D.h>
#include <xsi_time.h>

// Specific Includes
//------------------------------------------------------------
#include "bSynthInstruments.h"
#include "bSynthData.h"

// namespaces
//------------------------------------------------------------
using namespace XSI;

// Defines port, group and map identifiers used for registering the ICENode
enum IDs
{
	ID_IN_Enable = 0,
	ID_IN_Time = 1,
	ID_IN_Tempo = 2,
	ID_IN_Frequency = 3,
	ID_IN_Volume = 4,
	ID_IN_NbVoices = 5,
	ID_IN_Note = 6,
	ID_IN_Velocity = 7,
	ID_IN_Instrument = 8,
	ID_G_100,
	ID_OUT_Output = 200,
	ID_TYPE_CNS = 400,
	ID_STRUCT_CNS,
	ID_CTXT_CNS,
	ID_UNDEF = ULONG_MAX
};

XSI::CStatus RegisterbSynthNode( XSI::PluginRegistrar& in_reg );
XSI::CStatus RegisterbSineNode( XSI::PluginRegistrar& in_reg );
XSI::CStatus RegisterbSoundReaderNode( XSI::PluginRegistrar& in_reg );
//XSI::CStatus RegisterbRagaNode( XSI::PluginRegistrar& in_reg );

using namespace XSI; 

SICALLBACK XSILoadPlugin( PluginRegistrar& in_reg )
{
	in_reg.PutAuthor(L"benmalartre");
	in_reg.PutName(L"bSynthNodePlugin");
	in_reg.PutVersion(1,0);

	RegisterbSynthNode( in_reg );
	RegisterbSineNode( in_reg);
	RegisterbSoundReaderNode(in_reg);
	//RegisterbRagaNode(in_reg);

	//RegistrationInsertionPoint - do not remove this line

	return CStatus::OK;
}

SICALLBACK XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
	CString strPluginName;
	strPluginName = in_reg.GetName();
	Application().LogMessage(strPluginName + L" has been unloaded.",siVerboseMsg);

	return CStatus::OK;
}

CStatus RegisterbSynthNode( PluginRegistrar& in_reg )
{
	ICENodeDef nodeDef;
	nodeDef = Application().GetFactory().CreateICENodeDef(L"bSynthNode",L"bSynthNode");

	CStatus st;
	st = nodeDef.PutColor(200,200,10);
	st.AssertSucceeded( ) ;

	st = nodeDef.PutThreadingModel(XSI::siICENodeSingleThreading);
	st.AssertSucceeded( ) ;

	// Add input ports and groups.
	st = nodeDef.AddPortGroup(ID_G_100);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(ID_IN_Enable,ID_G_100,siICENodeDataBool,siICENodeStructureSingle,siICENodeContextSingleton,L"Enable",L"Enable",false);
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(ID_IN_Time,ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"Time",L"Time",0.0);
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(ID_IN_Tempo,ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"Tempo",L"Tempo",6.0);
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(ID_IN_Frequency,ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"Frequency",L"Frequency",33.0);
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(ID_IN_Volume,ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"Volume",L"Volume",1.0);
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(ID_IN_Note,ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"Note",L"Note",50.0);
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(ID_IN_Velocity,ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"Velocity",L"Velocity",64.0);
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(ID_IN_NbVoices,ID_G_100,siICENodeDataLong,siICENodeStructureSingle,siICENodeContextSingleton,L"NbVoices",L"NbVoices",12);
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(ID_IN_Instrument,ID_G_100,siICENodeDataLong,siICENodeStructureSingle,siICENodeContextSingleton,L"Instrument",L"Instrument",15);
	st.AssertSucceeded( ) ;

	// Add output ports.
	st = nodeDef.AddOutputPort(ID_OUT_Output,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"Output",L"Output");
	st.AssertSucceeded( ) ;

	PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
	nodeItem.PutCategories(L"Custom ICENode");

	return CStatus::OK;
}

SICALLBACK bSynthNode_Evaluate( ICENodeContext& in_ctxt )
{

	// Get the user data that we allocated in BeginEvaluate
	synthNodeData* data = (synthNodeData*)(CValue::siPtrType)in_ctxt.GetUserData( );

	// Node Enable
	CDataArrayBool enableData(in_ctxt, ID_IN_Enable);
	if(!enableData[0])
	{
		// if not enable, we shut down the stream
		data->shutUp();
		return CStatus::OK;
	}
	else
	{
		data->standUp();
	}
	Application().LogMessage(L"Enable passed...");

	// The current output port being evaluated...
	ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );
  
	switch( out_portID )
	{	
		Application().LogMessage(L"Output called...");
		case ID_OUT_Output :
		{
			// Time
			CDataArrayFloat timeData(in_ctxt, ID_IN_Time);
			data->timeCurrent = timeData[0];
			Application().LogMessage(L"Get Time Data...");

			// Tempo
			CDataArrayFloat tempoData(in_ctxt, ID_IN_Tempo);
			data->tempo = tempoData[0];
			Application().LogMessage(L"Get Tempo Data...");

			// Nb Voices
			CDataArrayLong voiceData(in_ctxt, ID_IN_NbVoices);
			LONG nbVoices = voiceData[0];
			Application().LogMessage(L"Get NbVoices Data...");

			// Frequency
			CDataArrayFloat freqData(in_ctxt, ID_IN_Frequency);
			data->frequency = freqData[0];
			Application().LogMessage(L"Get Frequency Data...");
			//data->voicer->setFrequency(data->frequency);

			//Volume
			CDataArrayFloat volumeData(in_ctxt, ID_IN_Volume);
			data->volume = volumeData[0];
			Application().LogMessage(L"Get Volume Data...");

			//Note
			CDataArrayFloat noteData(in_ctxt, ID_IN_Note);
			data->note = noteData[0];
			Application().LogMessage(L"Get Note Data...");

			//Velocity
			CDataArrayFloat velData(in_ctxt, ID_IN_Velocity);
			data->velocity = velData[0];
			Application().LogMessage(L"Get Velocity Data...");

			//Instrument
			CDataArrayLong instruData(in_ctxt, ID_IN_Instrument);
			data->instrumentCurrent = instruData[0];
			Application().LogMessage(L"Get Instrument Data...");
			
			//change instrument
			if(data->instrumentCurrent!=data->instrumentLast||data->nbVoices!=nbVoices)
			{
				for(int i=0;i<data->nbVoices;i++)
				{
					data->voicer->removeInstrument(data->instruments[i]);
					delete(data->instruments[i]);
				}
				data->instruments.clear();

				data->nbVoices = voiceData[0];
				for(int i=0;i<data->nbVoices;i++)
				{
					Instrmnt* instru = voiceByNumber3(data->instrumentCurrent);
					if(instru == NULL)
						instru = voiceByNumber3(7);
					data->voicer->addInstrument(instru);
					data->instruments.push_back(instru);
				}
				
				data->instrumentLast = data->instrumentCurrent;
				
			}
			
			data->settling = false;
			data->counter = 0;

		}
		break;

	};
	
	return CStatus::OK;
}

SICALLBACK bSynthNode_Init( CRef& in_ctxt )
{
	ICENodeContext ctxt( in_ctxt );
	CTime nodeTime(ctxt.GetTime());
	double frameRate = nodeTime.GetFrameRate();

	double samplePerFrame = SAMPLE_RATE/(frameRate);
	Application().LogMessage(L"Sample per Frame : "+(CString)(samplePerFrame+1));

	synthNodeData* data = new synthNodeData();
	if(data->init())
	{
		data->maxTick = (int)samplePerFrame;
		ctxt.PutUserData( data );
		return CStatus::OK;
	}
	else
	{
		Application().LogMessage(L"Error opening Audio Stream ---> Aborted!!");
		return CStatus::Fail;
	}
}

SICALLBACK bSynthNode_Term( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	// Release memory allocated in BeginEvaluate
	CValue userData = ctxt.GetUserData( );
	if ( userData.IsEmpty() )
	{
		return CStatus::OK;
	}

	synthNodeData* data = (synthNodeData*)(CValue::siPtrType)userData;
	if(data->dac.isStreamOpen())
		data->dac.closeStream();

	delete data;

	// Clear user data; 
	ctxt.PutUserData( CValue() );
	return CStatus::OK;
}

