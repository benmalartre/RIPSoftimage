// bSoundReaderNode Plugin
// an attempt to read wav sounds from inside xsi
// using STK
//------------------------------------------------------------

// STK Includes
//------------------------------------------------------------
#include "FileWvIn.h"
#include "RtWvOut.h"
#include "RtAudio.h"

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
#include <signal.h>
#include <iostream>
#include <cstdlib>

// namespaces
//------------------------------------------------------------
using namespace XSI;

struct SoundReaderData
{
	// Initialize our WvIn and RtAudio pointers.
	FileWvIn* _input;
	FileWvIn* _output;
	RtAudio* _out;
	StkFrames _frames;
	bool _done;
	float _time;
	bool _open;
	CString _filename;
	int _channels;
	double _rate;

	SoundReaderData():  _input(0),_open(false),_channels(1) {}
};

// The tick() function handles sample computation and scheduling of
// control updates.  If doing realtime audio output, it will be called
// automatically when the system needs a new buffer of audio samples.
int TickReader( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
          double streamTime, RtAudioStreamStatus status, void *dataPointer )
{		
	SoundReaderData *data = (SoundReaderData *) dataPointer;
	register StkFloat  sample, *samples = (StkFloat *) outputBuffer;

	data->_input->tick(data->_frames);

	for(unsigned int i=0;i<data->_frames.size();i++)
	{
		*samples++ = data->_frames[i];
	}
	if ( data->_input->isFinished() ) 
	{
		data->_done = true;
		return 1;
	}

	else
		return 0;
}

// Defines port, group and map identifiers used for registering the ICENode
enum IDs
{
	ID_IN_Enable = 0,
	ID_IN_File = 1,
	ID_G_100,
	ID_OUT_Output = 200,
	ID_TYPE_CNS = 400,
	ID_STRUCT_CNS,
	ID_CTXT_CNS,
	ID_UNDEF = ULONG_MAX
};

CStatus RegisterbSoundReaderNode( PluginRegistrar& in_reg )
{
	ICENodeDef nodeDef;
	nodeDef = Application().GetFactory().CreateICENodeDef(L"bSoundReaderNode",L"bSoundReaderNode");

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
	st = nodeDef.AddInputPort(ID_IN_File,ID_G_100,siICENodeDataString,siICENodeStructureSingle,siICENodeContextSingleton,L"File",L"File");
	st.AssertSucceeded( ) ;

	// Add output ports.
	st = nodeDef.AddOutputPort(ID_OUT_Output,siICENodeDataFloat,siICENodeStructureArray,siICENodeContextSingleton,L"Output",L"Output");
	st.AssertSucceeded( ) ;

	PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
	nodeItem.PutCategories(L"Custom ICENode");

	return CStatus::OK;
}

SICALLBACK bSoundReaderNode_Evaluate( ICENodeContext& in_ctxt )
{
	in_ctxt.SetAsTimeVarying();
	// Get the user data that we allocated in BeginEvaluate
	SoundReaderData* data = (SoundReaderData*)(CValue::siPtrType)in_ctxt.GetUserData( );

	//Time
	CTime time(in_ctxt.GetTime());
	float framerate = time.GetFrameRate();
	float t = time.GetTime();

	
	// Node Enable
	CDataArrayBool enableData(in_ctxt, ID_IN_Enable);
	if(!enableData[0]||t==data->_time)
	{
		// if not enable, we shut down the stream
		Application().LogMessage(L"We Stop the stream...");
		//data->_out->abortStream();
		return CStatus::OK;
	}
	/*
	else
	{
		if(!data->_out->isStreamRunning()){}
			data->_out->startStream();
	}
	*/
	
	data->_time = t;
	// The current output port being evaluated...
	ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );

	switch( out_portID )
	{		
		case ID_OUT_Output :
		{
			// FileName
			CDataArrayString fileData(in_ctxt, ID_IN_File);
			CString file = fileData[0];
			
			if(!data->_open || data->_filename != file)
			{
				try {
					//data->_input->openFile(file.GetAsciiString());
					data->_output->openFile(file.GetAsciiString());
					data->_open = true;
					data->_filename = file;
					/*
					// Set input read rate based on the default STK sample rate.
					data->_rate = 1.0;
					Application().LogMessage(L"File Rate : "+(CString)data->_input->getFileRate());
					data->_rate = data->_input->getFileRate() / Stk::sampleRate();
					data->_input->setRate( data->_rate );
					data->_input->ignoreSampleRateChange();
					Application().LogMessage(L"Rate : "+(CString)data->_rate);

					// Set input read rate based on the default STK sample rate.

					// Find out how many channels we have.
					data->_channels = data->_input->channelsOut();
					Application().LogMessage(L"Nb Channels : "+(CString)data->_channels);
					*/
					
				}
				catch (StkError &) 
				{
					Application().LogMessage(L"Can't open "+file+L"...");
					data->_open = false;
					data->_filename = L"";
					return CStatus::Fail;
				}
			}
		
			data->_output->reset();
			data->_output->addTime(t*(Stk::sampleRate()/framerate));

			unsigned int nFrames = data->_output->getFileRate();
			StkFrames frames;
			frames.resize(nFrames,1);

			// Declare the output port array ...
			CDataArray2DFloat outData( in_ctxt );
			
			// Output arrays must always be initialized first
			CDataArray2DFloat::Accessor outAccessor = outData.Resize( 0, nFrames );
			data->_output->tick(frames);

			if ( data->_output->isFinished() ) 
			{
				data->_done = true;
				return 1;
			}

			for(LONG f=0;f<nFrames;f++)
			{
				outAccessor[f] = frames[f];
			}
		
		}
		break;

	};

	return CStatus::OK;
}

SICALLBACK bSoundReaderNode_Init( CRef& in_ctxt )
{
	ICENodeContext ctxt( in_ctxt );

	// Set the global sample rate before creating class instances.
	Stk::setSampleRate( SAMPLE_RATE );

	SoundReaderData* data = new SoundReaderData();
	/*
	data->_out = new RtAudio();

	// Figure out how many bytes in an StkFloat and setup the RtAudio stream.
	RtAudio::StreamParameters parameters;
	parameters.deviceId = data->_out->getDefaultOutputDevice();
	parameters.nChannels = 1;
	RtAudioFormat format = ( sizeof(StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
	*/
	unsigned int bufferFrames = RT_BUFFER_SIZE;
	/*
	data->_out->openStream( &parameters, NULL, format, (unsigned int)Stk::sampleRate(), &bufferFrames, &TickReader, (void *)data );
	data->_input = new FileWvIn();
	*/
	data->_output = new FileWvIn();

	data->_frames.resize(bufferFrames,1);

	ctxt.PutUserData( data );

	return CStatus::OK;
}

SICALLBACK bSoundReaderNode_Term( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );

	// Release memory allocated in BeginEvaluate
	CValue userData = ctxt.GetUserData( );
	if ( userData.IsEmpty() )
	{
		return CStatus::OK;
	}

	SoundReaderData* data = (SoundReaderData*)(CValue::siPtrType)userData;

	//data->_input->closeFile();
	data->_output->closeFile();
	//data->_out->abortStream();

	//delete data->_input;
	delete data->_output;
	//delete data->_out;

	delete data;

	// Clear user data; 
	ctxt.PutUserData( CValue() );
	return CStatus::OK;
}

