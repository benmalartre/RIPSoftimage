// bSineNode Plugin
// an attempt to generate sounds from inside xsi
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

using namespace stk;

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

// namespaces
//------------------------------------------------------------
using namespace XSI;

struct sineData{
	RtAudio _dac;
	SineWave _sine;
	StkFrames _frames;
};

// The tick() function handles sample computation and scheduling of
// control updates.  If doing realtime audio output, it will be called
// automatically when the system needs a new buffer of audio samples.
int TickSineNode( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
          double streamTime, RtAudioStreamStatus status, void *userData )
{
	
	Application().LogMessage(L"Tick called...");
	sineData *data = (sineData *) userData;

	register StkFloat  sample, *samples = (StkFloat *) outputBuffer;

	for(unsigned int i=0;i<nBufferFrames;i++)
	{
		sample = data->_sine.tick();
		*samples++ = sample;
	}
	return 0;
}

// Defines port, group and map identifiers used for registering the ICENode
enum IDs
{
	ID_IN_Enable = 0,
	ID_IN_Frequency = 1,
	ID_G_100,
	ID_OUT_Output = 200,
	ID_TYPE_CNS = 400,
	ID_STRUCT_CNS,
	ID_CTXT_CNS,
	ID_UNDEF = ULONG_MAX
};

CStatus RegisterbSineNode( PluginRegistrar& in_reg )
{
	ICENodeDef nodeDef;
	nodeDef = Application().GetFactory().CreateICENodeDef(L"bSineNode",L"bSineNode");

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

	// Add output ports.
	st = nodeDef.AddOutputPort(ID_OUT_Output,siICENodeDataFloat,siICENodeStructureArray,siICENodeContextSingleton,L"Output",L"Output");
	st.AssertSucceeded( ) ;

	PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
	nodeItem.PutCategories(L"Custom ICENode");

	return CStatus::OK;
}

SICALLBACK bSineNode_Evaluate( ICENodeContext& in_ctxt )
{

	// Get the user data that we allocated in BeginEvaluate
	sineData* data = (sineData*)(CValue::siPtrType)in_ctxt.GetUserData( );
	
	// Node Enable
	CDataArrayBool enableData(in_ctxt, ID_IN_Enable);
	if(!enableData[0])
	{
		// if not enable, we shut down the stream
		Application().LogMessage(L"We stop the stream...");
		//data->_dac.stopStream();
		data->_dac.abortStream();
		return CStatus::OK;
	}
	else
	{
		if(!data->_dac.isStreamRunning())
			data->_dac.startStream();
	}
	
	// The current output port being evaluated...
	ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );

	switch( out_portID )
	{		
		case ID_OUT_Output :
		{
			// Frequency
			CDataArrayFloat freqData(in_ctxt, ID_IN_Frequency);
			StkFloat frequency = freqData[0];
			
			data->_sine.setFrequency(frequency);
			/*
			try
			{
				data->_out.readBuffer(&data->_frames,RT_BUFFER_SIZE);
				// Declare the output port array ...
				CDataArray2DFloat outData( in_ctxt );
				
				// Output arrays must always be initialized first
				CDataArray2DFloat::Accessor outAccessor = outData.Resize( 0, RT_BUFFER_SIZE );

				
				if(!data->_frames.empty())
				{
					for(ULONG f=0;f<RT_BUFFER_SIZE;f++)
					{
						outAccessor[f] = data->_frames[f];
					}
				}
			}
			catch(RtError &error)
			{
				Application().LogMessage(L"Fail reading buffer ---> exit!!");
				return CStatus::OK;
			}
			*/
		}
		break;

	};

	return CStatus::OK;
}

SICALLBACK bSineNode_Init( CRef& in_ctxt )
{
	ICENodeContext ctxt( in_ctxt );

	// Set the global sample rate before creating class instances.
	Stk::setSampleRate( SAMPLE_RATE );

	sineData* data = new sineData();

	// Figure out how many bytes in an StkFloat and setup the RtAudio stream.
	RtAudio::StreamParameters parameters;
	parameters.deviceId = data->_dac.getDefaultOutputDevice();
	parameters.nChannels = 1;
	RtAudioFormat format = ( sizeof(StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
	unsigned int bufferFrames = RT_BUFFER_SIZE;
	data->_frames.resize(RT_BUFFER_SIZE);

	try 
	{
		data->_dac.openStream( &parameters, NULL, format, (unsigned int)Stk::sampleRate(), &bufferFrames, &TickSineNode, (void *)data );
	}
	catch ( RtError &error ) 
	{
		return false;
	}

	data->_sine.setFrequency( 441.0 );
	ctxt.PutUserData( data );
	return CStatus::OK;

	return true;
}

SICALLBACK bSineNode_Term( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	// Release memory allocated in BeginEvaluate
	CValue userData = ctxt.GetUserData( );
	if ( userData.IsEmpty() )
	{
		return CStatus::OK;
	}

	sineData* data = (sineData*)(CValue::siPtrType)userData;
	data->_dac.stopStream();

	delete data;

	// Clear user data; 
	ctxt.PutUserData( CValue() );
	return CStatus::OK;
}

