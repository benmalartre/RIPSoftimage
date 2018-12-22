// SynthoxVoicer.cpp
//----------------------------------------------
#include "SynthoxVoicer.h"

CStatus RegisterSynthoxVoicer( PluginRegistrar& in_reg )
{
	ICENodeDef nodeDef;
	nodeDef = Application().GetFactory().CreateICENodeDef(L"SynthoxVoicer",L"SynthoxVoicer");

	CStatus st;
	st = nodeDef.PutColor(200,200,10);
	st.AssertSucceeded();

	// Define the Instrument custom type.
	st = nodeDef.DefineCustomType(	L"SynthoxInstrument",L"Synthox Instrument", L"Handle Instrument Data",216,255,87);

	st = nodeDef.PutThreadingModel(XSI::siICENodeSingleThreading);
	st.AssertSucceeded() ;

	// Add input ports and groups.
	st = nodeDef.AddPortGroup(Voicer_ID_G_100);
	st.AssertSucceeded( ) ;

	st = nodeDef.AddInputPort(Voicer_ID_IN_Enable,Voicer_ID_G_100,siICENodeDataBool,siICENodeStructureSingle,siICENodeContextSingleton,L"Enable",L"Enable",false);
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(Voicer_ID_IN_Volume,Voicer_ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"Volume",L"Volume",1.0);
	st.AssertSucceeded( ) ;
	st = nodeDef.AddInputPort(Voicer_ID_IN_Frequency,Voicer_ID_G_100,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"Frequency",L"Frequency",33.0);
	st.AssertSucceeded( ) ;

	CStringArray InstrumentCustomType(1);
	InstrumentCustomType[0] = L"SynthoxInstrument";

	st = nodeDef.AddInputPort(Voicer_ID_IN_Instruments,Voicer_ID_G_100,InstrumentCustomType,siICENodeStructureAny,siICENodeContextSingleton,L"Instrument",L"Instrument",Voicer_ID_UNDEF,Voicer_ID_UNDEF,Voicer_ID_CTXT_CNS);
	st.AssertSucceeded( ) ;

	// Add output ports.
	st = nodeDef.AddOutputPort(Voicer_ID_OUT_Output,siICENodeDataFloat,siICENodeStructureSingle,siICENodeContextSingleton,L"Output",L"Output");
	st.AssertSucceeded( ) ;

	PluginItem nodeItem = in_reg.RegisterICENode(nodeDef);
	nodeItem.PutCategories(L"Custom ICENode");

	return CStatus::OK;
}

SICALLBACK SynthoxVoicer_Evaluate( ICENodeContext& in_ctxt )
{
	// Get the user data that we allocated in BeginEvaluate
	VoicerData* data = (VoicerData*)(CValue::siPtrType)in_ctxt.GetUserData( );

	// Node Enable
	CDataArrayBool enableData(in_ctxt, Voicer_ID_IN_Enable);
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

	// The current output port being evaluated...
	ULONG out_portID = in_ctxt.GetEvaluatedOutputPortID( );
  
	switch( out_portID )
	{	
		case Voicer_ID_OUT_Output :
		{
			// Force node to evaluate at each frame.
			in_ctxt.SetAsTimeVarying();

			//Time
			CTime timeData(in_ctxt.GetTime());
			data->timeCurrent = timeData.GetTime();
			
			//Volume
			CDataArrayFloat volumeData(in_ctxt, Voicer_ID_IN_Volume);
			data->volume = volumeData[0];

			// Frequency
			CDataArrayFloat frequencyData(in_ctxt, Voicer_ID_IN_Frequency);
			data->frequency = frequencyData[0];
			
			// Instruments
			siICENodeDataType inPortType;
			siICENodeStructureType inPortStruct;
			siICENodeContextType inPortContext;

			in_ctxt.GetPortInfo( Voicer_ID_IN_Instruments, inPortType, inPortStruct, inPortContext );
			if ( inPortStruct == XSI::siICENodeStructureSingle )
			{
				data->multiInput = false;
			}
			else if ( inPortStruct == XSI::siICENodeStructureArray )
			{
				data->multiInput = true;
			}
			else
			{
				Application().LogMessage(L"Getting Instruments Port Structure Error...");
				return CStatus::Fail;
			}
			
			// Multiple Instruments Input
			if(data->multiInput)
			{
				CDataArray2DCustomType InstrumentDataArray( in_ctxt, Voicer_ID_IN_Instruments );
				CDataArray2DCustomType::Accessor InstrumentSubArray = InstrumentDataArray[0];
				ULONG nbInstruments = InstrumentSubArray.GetCount( );
				
				if(data->nbInstruments!=nbInstruments||data->instrumentsData.size()!=nbInstruments)
				{
					data->nbInstruments = nbInstruments;
					data->initInstrumentsData();

					for (ULONG i=0; i<data->nbInstruments; i++)
					{
						CDataArray2DCustomType::TData* pBufferInstrument;
						ULONG nSizeInstrument;

						InstrumentSubArray.GetData(i,(const CDataArray2DCustomType::TData**)& pBufferInstrument, nSizeInstrument);
						::memcpy(data->instrumentsData[i],pBufferInstrument,sizeof(InstrumentData));

						Instrmnt* instrmnt = voiceByNumber3(data->instrumentsData[i]->instrumentCurrent);
						if(instrmnt == NULL)
 							instrmnt = voiceByNumber3(7);
						data->voicer->addInstrument(instrmnt,i);
						data->instruments.push_back(instrmnt);
					}
					data->dac.startStream();

					Application().LogMessage(L"Init Instrument Called...");
				}

				else
				{
					for (ULONG i=0; i<data->nbInstruments; i++)
					{
						CDataArray2DCustomType::TData* pBufferInstrument;
						ULONG nSizeInstrument;

						InstrumentSubArray.GetData(i,(const CDataArray2DCustomType::TData**)& pBufferInstrument, nSizeInstrument);
						::memcpy((void*)data->instrumentsData[i],(const void*)pBufferInstrument,sizeof(InstrumentData));
					}
				}
			}

			//Only ONE Instrument Input
			else
			{
				CDataArrayCustomType InstruData( in_ctxt, Voicer_ID_IN_Instruments);
				CDataArrayCustomType::TData* pBufferInstrument;
				ULONG nSizeInstrument;

				ULONG nbInstruments = 1;
				
				if(data->nbInstruments!=nbInstruments||data->instrumentsData.size()!=nbInstruments)
				{
					data->nbInstruments = nbInstruments;
					data->initInstrumentsData();

					InstruData.GetData( 0,(const CDataArrayCustomType::TData**)&pBufferInstrument, nSizeInstrument );
					::memcpy((void*)data->instrumentsData[0],(const void*)pBufferInstrument,sizeof(InstrumentData));

					Instrmnt* instrmnt = voiceByNumber3(data->instrumentsData[0]->instrumentCurrent);
					if(instrmnt == NULL)
						instrmnt = voiceByNumber3(7);
					data->voicer->addInstrument(instrmnt,0);
					data->instruments.push_back(instrmnt);

					data->dac.startStream();

					Application().LogMessage(L"Init Instruments Called...");
				}
				
				else
				{
					InstruData.GetData( 0,(const CDataArrayCustomType::TData**)&pBufferInstrument, nSizeInstrument );
					::memcpy((void*)data->instrumentsData[0],(const void*)pBufferInstrument,sizeof(InstrumentData));
				}
			}
			data->settling = false;
			data->counter = 0;

			//data->voicer->setFrequency(data->frequency);

		}
		break;

	};
	
	return CStatus::OK;
}

SICALLBACK SynthoxVoicer_Init( CRef& in_ctxt )
{
	ICENodeContext ctxt( in_ctxt );
	CTime nodeTime(ctxt.GetTime());
	double frameRate = nodeTime.GetFrameRate();

	double samplePerFrame = SAMPLE_RATE/(frameRate);
	//Application().LogMessage(L"Sample per Frame : "+(CString)(samplePerFrame+1));

	VoicerData* data = new VoicerData();
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

SICALLBACK SynthoxVoicer_Term( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	// Release memory allocated in BeginEvaluate
	CValue userData = ctxt.GetUserData( );
	if ( userData.IsEmpty() )
	{
		return CStatus::OK;
	}

	VoicerData* data = (VoicerData*)(CValue::siPtrType)userData;
	data->shutUp();
	data->cleanUp();

	delete data;

	// Clear user data; 
	ctxt.PutUserData( CValue() );
	return CStatus::OK;
}

// The tick() function handles sample computation and scheduling of
// control updates.  If doing realtime audio output, it will be called
// automatically when the system needs a new buffer of audio samples.
int TickSynthox( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
          double streamTime, RtAudioStreamStatus status, void *dataPointer )
{
	VoicerData* data = (VoicerData *) dataPointer;
	if(data->settling == true)goto settle;

	register StkFloat  sample, *samples = (StkFloat *) outputBuffer;

	if(data->instrumentsData.size()==0)
	{
		Application().LogMessage(L"TickAborted...");
		return 1;
	}

	for(ULONG i=0;i<data->nbInstruments;i++)
	{
		InstrumentData* currentInstru = data->instrumentsData[i];
		if(!currentInstru->enable)
		{
			data->instruments[i]->noteOff(64.0);
			//data->instruments[i]->s
			continue;
		}

		if(currentInstru->changeInstrument)
		{
			//Application().LogMessage(L"Change Instrument ID "+(CString)i +" Called...");
			data->voicer->removeInstrument(data->instruments[i]);
			delete data->instruments[i];

			Instrmnt* instrmnt = voiceByNumber3(currentInstru->instrumentCurrent);
			if(instrmnt == NULL)
				instrmnt = voiceByNumber3(7);
			data->voicer->addInstrument(instrmnt,i);
			data->instruments[i] = instrmnt;
		
		}

		//data->instruments[i]->setFrequency(currentInstru->frequency);
		if(currentInstru->noteOn)
		{
			currentInstru->noteTag = data->voicer->noteOn(currentInstru->note,currentInstru->volume,i);
			currentInstru->noteOn = false;
		}
		/*
		else
		{
			data->voicer->noteOff((LONG)currentInstru->noteTag,64.0);
		}
		*/
	}

	for(unsigned int i=0;i<nBufferFrames;i++)
	{
		sample = data->volume * data->voicer->tick();
		*samples++ = sample;
	}

	data->counter++;
	
	if(data->counter>data->maxTick)
	{
		data->settling = true;
		goto settle;
	}
	
	

	return 0;

  settle:
		Application().LogMessage(L"Settling called...");
		data->voicer->silence();
		data->dac.abortStream();
		
  return 0;

}

VoicerData::VoicerData()
{
	voicer = new Voicer();
	channels = 1;
	nbInstruments = nbVoices = 0;
}

VoicerData::~VoicerData()
{
	if(dac.isStreamOpen())
		dac.closeStream();

	// Delete Voicer
	delete voicer;
}

bool VoicerData::init()
{
	// Set the global sample rate before creating class instances.
	Stk::setSampleRate( SAMPLE_RATE );

	// Depending on how you compile STK, you may need to explicitly set
	// the path to the rawwave directory.
	Stk::setRawwavePath("C:\\Documents and Settings\\Julien\\Mes documents\\WORKS\\STK\\rawwaves");

	// Figure out how many bytes in an StkFloat and setup the RtAudio stream.
	RtAudio::StreamParameters parameters;
	parameters.deviceId = dac.getDefaultOutputDevice();
	parameters.nChannels = 1;
	RtAudioFormat format = ( sizeof(StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
	unsigned int bufferFrames = RT_BUFFER_SIZE;

	try 
	{
		dac.openStream( &parameters, NULL, format, (unsigned int)Stk::sampleRate(), &bufferFrames, &TickSynthox, (void *)this );
	}
	catch ( RtError &error ) 
	{
		return false;
	}
	return true;
}

bool VoicerData::shutUp()
{
	if(dac.isStreamRunning())
	{
		dac.abortStream();
		return true;
	}

	return false;
}

bool VoicerData::standUp()
{
	if(!dac.isStreamRunning())
	{
		try 
		{
			dac.startStream();
		}
		catch ( RtError &error ) 
		{
			return false;
		}
	}
	return true;
}

void VoicerData::initInstrumentsData()
{
	shutUp();
	cleanUp();

	// Reallocate InstrumentsData
	for(ULONG i=0;i<nbInstruments;i++)
	{
		InstrumentData* instru = new InstrumentData();
		instrumentsData.push_back(instru);
	}
}

void VoicerData::cleanUp()
{
	for(std::vector<Instrmnt*>::iterator it = instruments.begin();it!=instruments.end();it++)
	{
		voicer->removeInstrument(*it);
		delete *it;
	}
	instruments.clear();
	for(std::vector<InstrumentData*>::iterator it = instrumentsData.begin();it!=instrumentsData.end();it++)
	{
		delete *it;
	}
	instrumentsData.clear();
}