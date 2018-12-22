// bSynthData.cpp
//------------------------------------------------------------

// Includes
//------------------------------------------------------------
#include "bSynthData.h"
#include "bSynthInstruments.h"

// The tick() function handles sample computation and scheduling of
// control updates.  If doing realtime audio output, it will be called
// automatically when the system needs a new buffer of audio samples.
int TickSynth( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
          double streamTime, RtAudioStreamStatus status, void *dataPointer )
{
	synthNodeData *data = (synthNodeData *) dataPointer;
	if(data->settling == true)goto settle;

	register StkFloat  sample, *samples = (StkFloat *) outputBuffer;

	if((int)data->timeCurrent%(int)data->tempo==0)
	{
		if(data->velocity == 0.0)
			data->voicer->noteOff(data->note,64.0);
		else
			data->voicer->noteOn(data->note,data->velocity);
	}

	for(unsigned int i=0;i<nBufferFrames;i++)
	{
		sample = data->volume * data->voicer->tick();;
		*samples++ = sample;
		//data->wvOut->tick(sample);
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

synthNodeData::synthNodeData()
{
	voicer = new Voicer();
	channels = 1;
	nbVoices = 6;

	for(int i=0;i<nbVoices;i++)
	{
		Instrmnt* instru = new Clarinet(10.0);
		instruments.push_back(instru);
		voicer->addInstrument(instruments[i]);
	}
}

synthNodeData::~synthNodeData()
{
	if(dac.isStreamOpen())
		dac.closeStream();

	for(std::vector<Instrmnt*>::iterator it = instruments.begin();it!=instruments.end();it++)
	{
		voicer->removeInstrument(*it);
		delete *it;
	}
	instruments.clear();

	// Delete Voicer
	delete voicer;
}

bool synthNodeData::init()
{
	// Set the global sample rate before creating class instances.
	Stk::setSampleRate( SAMPLE_RATE );

	// Depending on how you compile STK, you may need to explicitly set
	// the path to the rawwave directory.
	Stk::setRawwavePath("E:\\Works\\RnD\\STK\\rawwaves");

	// Figure out how many bytes in an StkFloat and setup the RtAudio stream.
	RtAudio::StreamParameters parameters;
	parameters.deviceId = dac.getDefaultOutputDevice();
	parameters.nChannels = 1;
	RtAudioFormat format = ( sizeof(StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
	unsigned int bufferFrames = RT_BUFFER_SIZE;

	try 
	{
		dac.openStream( &parameters, NULL, format, (unsigned int)Stk::sampleRate(), &bufferFrames, &TickSynth, (void *)this );
	}
	catch ( RtError &error ) 
	{
		return false;
	}
	return true;
}

bool synthNodeData::shutUp()
{
	if(dac.isStreamRunning())
	{
		dac.abortStream();
		return true;
	}

	return false;
}

bool synthNodeData::standUp()
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
