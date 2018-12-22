// SynthoxPlayerPlugin
// an attempt to read wav sounds from inside xsi
// using STK
//------------------------------------------------------------

// STK Includes
//------------------------------------------------------------
#include "FileWvIn.h"
#include "RtWvOut.h"
#include "RtAudio.h"

#include "SynthoxRegister.h"

struct PlayerData
{
	// Initialize our WvIn and RtAudio pointers.
	FileWvIn* _input;
	RtAudio* _out;
	StkFrames _frames;
	bool _isStereo;
	unsigned int _nFrames;
	ULONG _filesize;
	bool _done;
	bool _initialized;
	float _time;
	float _framerate;
	bool _frameChanged;
	LONG _currentFrame;
	LONG _lastFrame;
	bool _isPlaying;
	bool _open;
	bool _settle;
	CString _filename;
	int _channels;
	double _rate;
	LONG _counter;

	PlayerData():  _input(NULL),_out(NULL),_open(false),_done(false),_initialized(0),_channels(1),_settle(false) {}
};

// This tick() function handles sample computation only.  It will be
// called automatically when the system needs a new buffer of audio
// samples.
int TickSynthoxPlayer( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
	PlayerData* data = (PlayerData *) userData;
	register StkFloat *samples = (StkFloat *) outputBuffer;

	if(data->_settle)
		return 1;

	if(!data->_isPlaying && data->_frameChanged)
	{
		data->_input->reset();
		data->_input->addTime(data->_time*(Stk::sampleRate()));
		data->_frameChanged = false;
	}

	data->_input->tick( data->_frames );
	for ( unsigned int i=0; i<data->_frames.size(); i++ )
	*samples++ = data->_frames[i];

	data->_counter+=data->_frames.size();
	if(data->_counter>=Stk::sampleRate())
	{
		data->_settle = true;
	}
	return 0;
}

void InitFileInput(PlayerData* data, CString file)
{
	if(data->_out!=NULL)
	{
		data->_out->abortStream();
		data->_out->closeStream();
		delete data->_out;
	}

	if(data->_input!=NULL)
	{
		data->_input->closeFile();
		delete data->_input;
	}

	data->_input = new FileWvIn();
	data->_input->openFile(file.GetAsciiString());
	data->_channels = data->_input->channelsOut();

	data->_open = true;
	data->_filename = file;
}

void InitRealtimeOutput(PlayerData* data)
{
	data->_out = new RtAudio();

	// Figure out how many bytes in an StkFloat and setup the RtAudio stream.
	RtAudio::StreamParameters parameters;
	parameters.deviceId = data->_out->getDefaultOutputDevice();
	parameters.nChannels = data->_channels;
	RtAudioFormat format = ( sizeof(StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
	
	data->_out->openStream( &parameters, NULL, format, (unsigned int)Stk::sampleRate(), &data->_nFrames, &TickSynthoxPlayer, (void *)data );
	data->_rate = data->_input->getFileRate()/ Stk::sampleRate();
	data->_input->setRate( data->_rate );
	data->_input->ignoreSampleRateChange();

	// Resize the StkFrames object appropriately.
	data->_frames.resize(data->_nFrames,data->_channels);
	data->_out->startStream();
	
	data->_initialized = true;
}

// Defines port, group and map identifiers used for registering the ICENode
enum SynthoxPlayerIDs
{
	SynthoxPlayer_ID_IN_Enable = 0,
	SynthoxPlayer_ID_IN_File = 1,
	SynthoxPlayer_ID_IN_Rate = 2,
	SynthoxPlayer_ID_IN_Volume = 3,
	SynthoxPlayer_ID_G_100,
	SynthoxPlayer_ID_OUT_Output = 200,
	SynthoxPlayer_ID_TYPE_CNS = 400,
	SynthoxPlayer_ID_STRUCT_CNS,
	SynthoxPlayer_ID_CTXT_CNS,
	SynthoxPlayer_ID_UNDEF = ULONG_MAX
};

CStatus RegisterSynthoxPlayer( PluginRegistrar& in_reg )
{
	ICENodeDef nodeDef;
	nodeDef = Application().GetFactory().CreateICENodeDef(L"SynthoxPlayer",L"SynthoxPlayer");

	CStatus st;
	st = nodeDef.PutColor(200,200,10);
	st.AssertSucceeded( ) ;

	st = nodeDef.PutThreadingModel(XSI::siICENodeSingleThreading);
	st.AssertSucceeded( ) ;

	// Add input ports and groups.
	st = nodeDef.AddPortGroup(SynthoxPlayer_ID_G_100);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(SynthoxPlayer_ID_IN_Enable,SynthoxPlayer_ID_G_100,siICENodeDataBool,siICENodeStructureSingle,siICENodeContextSingleton,L"Enable",L"Enable",false);
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(SynthoxPlayer_ID_IN_File,SynthoxPlayer_ID_G_100,siICENodeDataString,siICENodeStructureSingle,siICENodeContextSingleton,L"File",L"File",L"");
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(SynthoxPlayer_ID_IN_Rate,SynthoxPlayer_ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"Rate",L"Rate",1.0);
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(SynthoxPlayer_ID_IN_Volume,SynthoxPlayer_ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"Volume",L"Volume",33.0);
	st.AssertSucceeded( ) ;

	// Add output ports.
	st = nodeDef.AddOutputPort(SynthoxPlayer_ID_OUT_Output,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"Output",L"Output");
	st.AssertSucceeded( ) ;

	PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
	nodeItem.PutCategories(L"Custom ICENode");

	return CStatus::OK;
}

SICALLBACK SynthoxPlayer_Evaluate( ICENodeContext& in_ctxt )
{
	in_ctxt.SetAsTimeVarying();
	// Get the user data that we allocated in BeginEvaluate
	PlayerData* data = (PlayerData*)(CValue::siPtrType)in_ctxt.GetUserData( );

	//Time
	CTime time(in_ctxt.GetTime());
	data->_framerate = time.GetFrameRate();
	data->_currentFrame = (LONG)time.GetTime();
	double t = time.GetTime()/data->_framerate;
	if(data->_currentFrame == data->_lastFrame+1)
		data->_isPlaying = true;
	else data->_isPlaying = false;

	data->_lastFrame = data->_currentFrame;
	
	// Node Enable
	CDataArrayBool enableData(in_ctxt, SynthoxPlayer_ID_IN_Enable);
	if(!enableData[0])
	{
		// if not enable, we shut down the stream
		Application().LogMessage(L"We Stop the stream...");
		//StopRealtimeOutput();
		return CStatus::OK;
	}
	/*
	else
	{
			InitRealtimeOutput(data);
	}
	*/
	
	data->_time = (float)t;
	data->_frameChanged = true;
	data->_settle = false;
	data->_counter = 0;
	// The current output port being evaluated...
	ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );

	switch( out_portID )
	{		
		case SynthoxPlayer_ID_OUT_Output:
		{
			// FileName
			CDataArrayString fileData(in_ctxt, SynthoxPlayer_ID_IN_File);
			CString file = fileData[0];
			
			if(!data->_open || data->_filename != file|| !data->_initialized)
			{
				InitFileInput(data, file);
				InitRealtimeOutput(data);
			}
			else
			{
				if(!data->_out->isStreamRunning())
				{
					data->_out->startStream();
				}
			}

			// Declare the output port array ...
			CDataArrayFloat outData( in_ctxt );
			
			outData[0] = t*(Stk::sampleRate()/data->_framerate);
		
		}
		break;

	};

	return CStatus::OK;
}

SICALLBACK SynthoxPlayer_Init( CRef& in_ctxt )
{
	ICENodeContext ctxt( in_ctxt );

	// Set the global sample rate before creating class instances.
	Stk::setSampleRate( SAMPLE_RATE );

	PlayerData* data = new PlayerData();
	data->_nFrames = RT_BUFFER_SIZE;

	ctxt.PutUserData( data );

	return CStatus::OK;
}

SICALLBACK SynthoxPlayer_Term( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );

	// Release memory allocated in BeginEvaluate
	CValue userData = ctxt.GetUserData( );
	if ( userData.IsEmpty() )
	{
		return CStatus::OK;
	}

	PlayerData* data = (PlayerData*)(CValue::siPtrType)userData;

	data->_out->abortStream();
	data->_out->closeStream();
	data->_input->closeFile();

	delete data->_out;
	delete data->_input;

	delete data;

	// Clear user data; 
	ctxt.PutUserData( CValue() );
	return CStatus::OK;
}

