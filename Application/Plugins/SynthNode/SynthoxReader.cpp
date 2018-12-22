// SynthoxReaderPlugin
// an attempt to read wav sounds from inside xsi
// using STK
//------------------------------------------------------------

// STK Includes
//------------------------------------------------------------
#include "FileWvIn.h"
#include "RtWvOut.h"
#include "RtAudio.h"

#include "SynthoxRegister.h"

struct ReaderData
{
	// Initialize our WvIn and RtAudio pointers.
	FileWvIn* _input;
	FileWvIn* _output;
	RtAudio* _out;
	bool _cached;
	float _volume;
	StkFrames _frames;
	StkFloat* _cacheLeft;
	StkFloat* _cacheRight;
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

	ReaderData():  _input(NULL),_out(NULL),_open(false),_done(false),
					_initialized(0),_channels(1),_cacheLeft(NULL),
					_cacheRight(NULL),_cached(false),_settle(false) {}
};

// This tick() function handles sample computation only.  It will be
// called automatically when the system needs a new buffer of audio
// samples.
int TickSynthoxReader( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
	ReaderData* data = (ReaderData *) userData;
	register StkFloat *samples = (StkFloat *) outputBuffer;

	if(data->_settle)
		return 1;

	if(!data->_isPlaying && data->_frameChanged)
	{
		data->_input->reset();
		data->_input->addTime(data->_time*(Stk::sampleRate()));
		data->_frameChanged = false;
	}

	Application().LogMessage(L"Counter :"+(CString)data->_counter);

	data->_input->tick( data->_frames );
	for ( unsigned int i=0; i<data->_frames.size(); i++ )
	*samples++ = data->_frames[i]*data->_volume;

	data->_counter+=data->_frames.size();
	if(data->_counter>=Stk::sampleRate())
	{
		data->_settle = true;
	}
	return 0;
}

void InitFileInput(ReaderData* data, CString file)
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

	StkFloat filerate = data->_input->getFileRate();
	Application().LogMessage(L"File Rate : "+(CString)filerate);

	data->_open = true;
	data->_filename = file;
}

void InitRealtimeOutput(ReaderData* data)
{
	data->_out = new RtAudio();

	// Figure out how many bytes in an StkFloat and setup the RtAudio stream.
	RtAudio::StreamParameters parameters;
	parameters.deviceId = data->_out->getDefaultOutputDevice();
	parameters.nChannels = data->_channels;
	RtAudioFormat format = ( sizeof(StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
	
	data->_out->openStream( &parameters, NULL, format, (unsigned int)Stk::sampleRate(), &data->_nFrames, &TickSynthoxReader, (void *)data );
	//data->_rate = data->_input->getFileRate()/ Stk::sampleRate();
	data->_input->setRate( data->_rate );
	data->_input->ignoreSampleRateChange();

	// Resize the StkFrames object appropriately.
	data->_frames.resize(data->_nFrames,data->_channels);
	data->_out->startStream();
	
	data->_initialized = true;
}

// Defines port, group and map identifiers used for registering the ICENode
enum SynthoxReaderIDs
{
	SynthoxReader_ID_IN_Enable = 0,
	SynthoxReader_ID_IN_File = 1,
	SynthoxReader_ID_IN_Rate = 2,
	SynthoxReader_ID_IN_Volume = 3,
	SynthoxReader_ID_G_100,
	SynthoxReader_ID_OUT_OutputLeft = 200,
	SynthoxReader_ID_OUT_OutputRight = 201,
	SynthoxReader_ID_TYPE_CNS = 400,
	SynthoxReader_ID_STRUCT_CNS,
	SynthoxReader_ID_CTXT_CNS,
	SynthoxReader_ID_UNDEF = ULONG_MAX
};

CStatus RegisterSynthoxReader( PluginRegistrar& in_reg )
{
	ICENodeDef nodeDef;
	nodeDef = Application().GetFactory().CreateICENodeDef(L"SynthoxReader",L"SynthoxReader");

	CStatus st;
	st = nodeDef.PutColor(200,200,10);
	st.AssertSucceeded( ) ;

	st = nodeDef.PutThreadingModel(XSI::siICENodeSingleThreading);
	st.AssertSucceeded( ) ;

	// Add input ports and groups.
	st = nodeDef.AddPortGroup(SynthoxReader_ID_G_100);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(SynthoxReader_ID_IN_Enable,SynthoxReader_ID_G_100,siICENodeDataBool,siICENodeStructureSingle,siICENodeContextSingleton,L"Enable",L"Enable",false);
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(SynthoxReader_ID_IN_File,SynthoxReader_ID_G_100,siICENodeDataString,siICENodeStructureSingle,siICENodeContextSingleton,L"File",L"File",L"");
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(SynthoxReader_ID_IN_Rate,SynthoxReader_ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"Rate",L"Rate",1.0);
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(SynthoxReader_ID_IN_Volume,SynthoxReader_ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"Volume",L"Volume",10.0);
	st.AssertSucceeded( ) ;

	// Add output ports.
	st = nodeDef.AddOutputPort(SynthoxReader_ID_OUT_OutputLeft,siICENodeDataFloat,siICENodeStructureArray,siICENodeContextSingleton,L"Left",L"Left");
	st.AssertSucceeded( ) ;
	st = nodeDef.AddOutputPort(SynthoxReader_ID_OUT_OutputRight,siICENodeDataFloat,siICENodeStructureArray,siICENodeContextSingleton,L"Right",L"Right");
	st.AssertSucceeded( ) ;

	PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
	nodeItem.PutCategories(L"Custom ICENode");

	return CStatus::OK;
}

SICALLBACK SynthoxReader_Evaluate( ICENodeContext& in_ctxt )
{
	in_ctxt.SetAsTimeVarying();
	// Get the user data that we allocated in BeginEvaluate
	ReaderData* data = (ReaderData*)(CValue::siPtrType)in_ctxt.GetUserData( );

	//Time
	CTime time(in_ctxt.GetTime());
	data->_framerate = time.GetFrameRate();
	data->_currentFrame = (LONG)time.GetTime();
	data->_time = (float)(time.GetTime()-1.0)/data->_framerate;

	if(data->_currentFrame == data->_lastFrame+1)
		data->_isPlaying = true;
	else data->_isPlaying = false;

	data->_frameChanged = true;
	data->_settle = false;
	data->_counter = 0;

	data->_lastFrame = data->_currentFrame;

	//rate
	CDataArrayFloat rateData(in_ctxt, SynthoxReader_ID_IN_Rate);
	data->_rate = rateData[0];
	
	// volume
	CDataArrayFloat volumeData(in_ctxt,SynthoxReader_ID_IN_Volume);
	data->_volume = volumeData[0];

	// Node Enable
	CDataArrayBool enableData(in_ctxt, SynthoxReader_ID_IN_Enable);
	if(!enableData[0])
	{
		// if not enable, we shut down the stream
		Application().LogMessage(L"We Stop the stream...");
		if(data->_out!=NULL)
		{
			if(data->_out->isStreamRunning())
				data->_out->abortStream();
		}
		
		return CStatus::OK;
	}

	// FileName
	CDataArrayString fileData(in_ctxt, SynthoxReader_ID_IN_File);
	CString file = fileData[0];
	
	if(!data->_open || data->_filename != file|| !data->_initialized)
	{
		if(!data->_cached|| data->_filename != file)
		{
			data->_output = new FileWvIn();
			data->_output->openFile(file.GetAsciiString());
			data->_open = true;
			data->_filename = file;

			//File Size
			data->_filesize = data->_output->getSize();
			data->_cacheLeft = new StkFloat[data->_filesize];

			//Is Stereo File
			data->_isStereo = data->_output->channelsOut()>1;
			Application().LogMessage(L"Is Stereo : "+(CString)data->_isStereo);
			if(data->_isStereo)
			{
				data->_cacheRight = new StkFloat[data->_filesize]; 

				for(ULONG n=0;n<data->_filesize;n++)
				{
					data->_cacheLeft[n] = data->_output->tick(0);
					data->_cacheRight[n] = data->_output->tick(1);
				}
			}
			else
			{
				for(LONG n=0;n<data->_filesize;n++)
				{
					data->_cacheLeft[n] = data->_output->tick(0);
				}
			}

			data->_output->closeFile();
			data->_cached = true;
			delete data->_output;

			InitFileInput(data, file);
			InitRealtimeOutput(data);
		}
	}

	else
	{
		if(!data->_out->isStreamRunning())
		{
			data->_out->startStream();
		}
		else
		{
			data->_input->setRate(data->_rate);
		}
	}
	// The current output port being evaluated...
	ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );
	ULONG outLength = (ULONG)(data->_nFrames*abs(data->_rate));

	switch( out_portID )
	{		
		case SynthoxReader_ID_OUT_OutputLeft:
		{
			if(!data->_initialized && data->_cached)return CStatus::Fail;

			// Declare the output port array ...
			CDataArray2DFloat outData( in_ctxt );
			
			// Output arrays must always be initialized first
			CDataArray2DFloat::Accessor outAccessor = outData.Resize( 0, outLength);
			ULONG base = (ULONG)(data->_time*(Stk::sampleRate()));
		
			for(ULONG f=0;f<outLength;f++)
			{
				if(base+f<0||base+f>=data->_filesize)outAccessor[f] = 0;
				else outAccessor[f] = data->_cacheLeft[base+f];
			}
		}
		break;

		case SynthoxReader_ID_OUT_OutputRight:
		{
			if(!data->_initialized && data->_cached)return CStatus::Fail;

			// Declare the output port array ...
			CDataArray2DFloat outData( in_ctxt );
			
			// Output arrays must always be initialized first
			CDataArray2DFloat::Accessor outAccessor = outData.Resize( 0, outLength );
			ULONG base = (ULONG)(data->_time*(Stk::sampleRate()));
			
			for(ULONG f=0;f<outLength;f++)
			{
				if(base+f<0||base+f>=data->_filesize)outAccessor[f] = 0;
				else 
				{
					if(data->_isStereo)outAccessor[f] = data->_cacheRight[base+f];
					else outAccessor[f] = data->_cacheLeft[base+f];
				}
			}
		}
		break;

	};

	return CStatus::OK;
}

SICALLBACK SynthoxReader_Init( CRef& in_ctxt )
{
	ICENodeContext ctxt( in_ctxt );

	// Set the global sample rate before creating class instances.
	Stk::setSampleRate( SAMPLE_RATE );

	ReaderData* data = new ReaderData();
	data->_nFrames = RT_BUFFER_SIZE;

	ctxt.PutUserData( data );

	return CStatus::OK;
}

SICALLBACK SynthoxReader_Term( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );

	// Release memory allocated in BeginEvaluate
	CValue userData = ctxt.GetUserData( );
	if ( userData.IsEmpty() )
	{
		return CStatus::OK;
	}

	ReaderData* data = (ReaderData*)(CValue::siPtrType)userData;
	
	data->_out->abortStream();
	data->_out->closeStream();
	data->_input->closeFile();

	delete data->_out;
	delete data->_input;
	if(data->_cacheLeft!=NULL)
		delete [] data->_cacheLeft;
	if(data->_cacheRight!=NULL)
		delete [] data->_cacheRight;

	delete data;

	// Clear user data; 
	ctxt.PutUserData( CValue() );
	return CStatus::OK;
}

