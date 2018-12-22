// bRagaNode Plugin
// an attempt to generate raga from inside xsi
// using STK
//------------------------------------------------------------

// STK Includes
//------------------------------------------------------------
#include "SineWave.h"
#include "RtWvOut.h"
#include "XSIWvOut.h"
#include "Drone.h"
#include "Instrmnt.h"
#include "JCRev.h"
#include "Drone.h"
#include "Sitar.h"
#include "Tabla.h"
#include "VoicDrum.h"
#include "SKINI.h"
#include "Messager.h"
#include "SKINI.msg"

#define SAMPLE_RATE 44100.0

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
#include <cstdlib>
#include <signal.h>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <cstdlib>

// namespaces
//------------------------------------------------------------
using namespace XSI;
using std::min;
using namespace stk;

struct RagaData{
	RtAudio dac;
	JCRev    reverbs[2];
	Drone    drones[3];
	Sitar    sitar;
	VoicDrum voicDrums;
	Messager messager;
	Skini::Message message;
	Tabla    tabla;
	StkFloat lastSample;
	StkFloat t60;
	int counter;
	bool settling;
	bool haveMessage;
	StkFloat droneChance, noteChance;
	StkFloat drumChance, voiceChance;
	int tempo;
	int chanceCounter;
	int key;
	int ragaStep;
	int ragaPoint;
	int endPhase;
	StkFloat rateScaler;

	// Default constructor.
	RagaData(): 
		t60(4.0), counter(0),
		settling( false ), haveMessage( false ), droneChance(0.01), noteChance(0.01),
		drumChance(0.0), voiceChance(0.0), tempo(3000), chanceCounter(3000), key(0), ragaPoint(6), endPhase(0) {}

};

// Raga key numbers and drone frequencies.
const int ragaUp[2][13] = {{57, 60, 62, 64, 65, 68, 69, 71, 72, 76, 77, 81},
                           {52, 54, 55, 57, 59, 60, 63, 64, 66, 67, 71, 72}};

const int ragaDown[2][13] = {{57, 60, 62, 64, 65, 67, 69, 71, 72, 76, 79, 81},
                             {48, 52, 53, 55, 57, 59, 60, 64, 66, 68, 70, 72}};

StkFloat droneFreqs[3] = { 55.0, 82.5, 220.0 };

#define DELTA_CONTROL_TICKS 64 // default sample frames between control input checks

StkFloat float_random(StkFloat max) // Return random float between 0.0 and max
{	
  StkFloat temp = (StkFloat) (max * rand() / (RAND_MAX + 1.0) );
  return temp;	
}


// The tick() function handles sample computation and scheduling of
// control updates.  It will be called automatically by RtAudio when
// the system needs a new buffer of audio samples.
int TickRagaNode( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *dataPointer )
{
  RagaData *data = (RagaData *) dataPointer;
  register StkFloat temp, outs[2], *samples = (StkFloat *) outputBuffer;
  int i, voiceNote, counter, nTicks = (int) nBufferFrames;

  while ( nTicks > 0) {
   
    data->counter = DELTA_CONTROL_TICKS;

    counter = min( nTicks, data->counter );
    data->counter -= counter;
    for ( i=0; i<counter; i++ ) {
      outs[0] = data->reverbs[0].tick( data->drones[0].tick() + data->drones[2].tick()
                                       + data->sitar.tick() );
      outs[1] = data->reverbs[1].tick( 1.5 * data->drones[1].tick() + 0.5 * data->voicDrums.tick()
                                       + 0.5 * data->tabla.tick() );
      // Mix a little left to right and back.
      *samples++ = outs[0] + 0.3 * outs[1];
      *samples++ = outs[1] + 0.3 * outs[0];
      nTicks--;

      // Do a bunch of random controls unless settling down to end.
      if ( data->settling ) {
        if ( data->counter == 0 ) {
          data->counter = (int) (data->t60 * Stk::sampleRate());
          if ( data->endPhase == 0 ) {
            data->drones[2].noteOn( droneFreqs[2], 0.1 );
            Application().LogMessage(L"What Need Have I for This?" );
          }
          else if ( data->endPhase == 1 ) {
            data->drones[0].noteOn( droneFreqs[0], 0.1 );
            Application().LogMessage(L"RagaMatic finished ... ");
          }
          else if ( data->endPhase == 2 ) {
            Application().LogMessage(L"All is Bliss ... ");;
          }
          else if ( data->endPhase == 3 ) {
            Application().LogMessage(L"All is Bliss ...");
          }
          data->endPhase++;
        }
      }
      else {
        data->chanceCounter--;
        if (data->chanceCounter == 0)	{
          data->chanceCounter = (int) ( data->tempo / data->rateScaler );
          if ( float_random(1.0) < data->droneChance )
            data->drones[0].noteOn( droneFreqs[0], 0.1 );
          if ( float_random(1.0) < data->droneChance )
            data->drones[1].noteOn( droneFreqs[1], 0.1 );
          if ( float_random(1.0) < data->droneChance )
            data->drones[2].noteOn( droneFreqs[2], 0.1 );
          if ( float_random(1.0) < data->noteChance ) {
            temp = float_random(1.0);
            if ( temp < 0.1) data->ragaStep = 0;
            else if (temp < 0.5) data->ragaStep = 1;
            else data->ragaStep = -1;
            data->ragaPoint += data->ragaStep;
            if ( data->ragaPoint < 0 ) 
              data->ragaPoint -= ( 2 * data->ragaStep );
            if ( data->ragaPoint > 11 ) data->ragaPoint = 11;
            if ( data->ragaStep > 0 )
              data->sitar.noteOn( Midi2Pitch[ragaUp[data->key][data->ragaPoint]],
                                  0.05 + float_random(0.3) );
            else
              data->sitar.noteOn( Midi2Pitch[ragaDown[data->key][data->ragaPoint]],
                                  0.05 + float_random(0.3) );
          }
          if ( float_random(1.0) < data->voiceChance ) {
            voiceNote = (int) float_random(11);
            data->voicDrums.noteOn( voiceNote, 0.3 + (0.4 * data->drumChance) +
                                    float_random(0.3 * data->voiceChance));
          }
          if ( float_random(1.0) < data->drumChance ) {
            voiceNote = (int) float_random(TABLA_NUMWAVES);
            data->tabla.noteOn( voiceNote, 0.2 + (0.2 * data->drumChance) + 
                                float_random(0.6 * data->drumChance));
          }
        }
      }
    }
    if ( nTicks == 0 ) break;
  }

  return 0;
}

// Defines port, group and map identifiers used for registering the ICENode
enum IDs
{
	ID_IN_Enable = 0,
	ID_IN_Frequency = 1,
	ID_IN_Tempo = 2,
	ID_G_100,
	ID_OUT_Output = 200,
	ID_TYPE_CNS = 400,
	ID_STRUCT_CNS,
	ID_CTXT_CNS,
	ID_UNDEF = ULONG_MAX
};

CStatus RegisterbRagaNode( PluginRegistrar& in_reg )
{
	ICENodeDef nodeDef;
	nodeDef = Application().GetFactory().CreateICENodeDef(L"bRagaNode",L"bRagaNode");

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
	st = nodeDef.AddInputPort(ID_IN_Frequency,ID_G_100,siICENodeDataFloat,siICENodeStructureAny,siICENodeContextSingleton,L"Frequency",L"Frequency",120.0);
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(ID_IN_Tempo,ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"Tempo",L"Tempo",3000.0);
	st.AssertSucceeded( ) ;

	// Add output ports.
	st = nodeDef.AddOutputPort(ID_OUT_Output,siICENodeDataFloat,siICENodeStructureArray,siICENodeContextSingleton,L"Output",L"Output");
	st.AssertSucceeded( ) ;

	PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
	nodeItem.PutCategories(L"Custom ICENode");

	return CStatus::OK;
}

SICALLBACK bRagaNode_Evaluate( ICENodeContext& in_ctxt )
{

	// Get the user data that we allocated in BeginEvaluate
	RagaData* data = (RagaData*)(CValue::siPtrType)in_ctxt.GetUserData( );

	// Node Enable
	CDataArrayBool enableData(in_ctxt, ID_IN_Enable);
	/*
	if(!enableData[0])
	{
		// if not enable, we shut down the stream
		Application().LogMessage(L"We stop the stream...");
		//data->dac.stopStream();
		data->dac.abortStream();
		return CStatus::OK;
	}
	else
	{
		if(!data->dac.isStreamRunning())
			data->dac.startStream();
	}*/

	if(!data->dac.isStreamRunning())
			data->dac.startStream();
	
	// The current output port being evaluated...
	ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );

	switch( out_portID )
	{		
		case ID_OUT_Output :
		{
			// Frequency
			CDataArrayFloat freqData(in_ctxt, ID_IN_Frequency);
			StkFloat frequency = freqData[0];

			// Tempo
			CDataArrayFloat tempoData(in_ctxt, ID_IN_Tempo);
			data->tempo = (int) (11025 - tempoData[0] * 70.0 );
		}
		break;

	};

	return CStatus::OK;
}

SICALLBACK bRagaNode_Init( CRef& in_ctxt )
{
	ICENodeContext ctxt( in_ctxt );

	// Set the global sample rate before creating class instances.
	Stk::setSampleRate( SAMPLE_RATE );

	RagaData* data = new RagaData();

	// Figure out how many bytes in an StkFloat and setup the RtAudio stream.
	RtAudio::StreamParameters parameters;
	parameters.deviceId = data->dac.getDefaultOutputDevice();
	parameters.nChannels = 2;
	RtAudioFormat format = ( sizeof(StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
	unsigned int bufferFrames = RT_BUFFER_SIZE;

	try 
	{
		data->dac.openStream( &parameters, NULL, format, (unsigned int)Stk::sampleRate(), &bufferFrames, &TickRagaNode, (void *)data );
	}
	catch ( RtError &error ) 
	{
		return false;
	}

	data->reverbs[0].setT60( data->t60 );
	data->reverbs[0].setEffectMix( 0.5 );
	data->reverbs[1].setT60( 2.0 );
	data->reverbs[1].setEffectMix( 0.2 );

	data->drones[0].noteOn( droneFreqs[0], 0.1 );
	data->drones[1].noteOn( droneFreqs[1], 0.1 );
	data->drones[2].noteOn( droneFreqs[2], 0.1 );

	data->rateScaler = 22050.0 / Stk::sampleRate();

	ctxt.PutUserData( data );
	return CStatus::OK;

	return true;
}

SICALLBACK bRagaNode_Term( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	// Release memory allocated in BeginEvaluate
	CValue userData = ctxt.GetUserData( );
	if ( userData.IsEmpty() )
	{
		return CStatus::OK;
	}

	RagaData* data = (RagaData*)(CValue::siPtrType)userData;
	delete data;

	// Clear user data; 
	ctxt.PutUserData( CValue() );
	return CStatus::OK;
}

