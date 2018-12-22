// SynthoxVoicer.h
//------------------------------------------------------------
#ifndef _SYNTHOXVOICER_H_
#define _SYNTHOXVOICER_H_
// STK Includes
//------------------------------------------------------------
#include "Instrmnt.h"
#include "Voicer.h"
#include "RtAudio.h"

#include "SynthoxRegister.h"

// Defines port, group and map identifiers used for registering the ICENode
enum VoicerIDs
{
	Voicer_ID_IN_Enable = 0,
	Voicer_ID_IN_Volume = 1,
	Voicer_ID_IN_Frequency = 2,
	Voicer_ID_IN_Instruments = 3,
	Voicer_ID_G_100,
	Voicer_ID_OUT_Output = 200,
	Voicer_ID_TYPE_CNS = 400,
	Voicer_ID_STRUCT_CNS,
	Voicer_ID_CTXT_CNS,
	Voicer_ID_UNDEF = ULONG_MAX
};

class VoicerData
{
public:
	RtAudio dac;
	std::vector<Instrmnt*> instruments;
	std::vector<InstrumentData*> instrumentsData;

	int channels;
	int maxTick;

	Voicer* voicer;
	int nbVoices;
	bool enable;
	StkFloat frequency;
	StkFloat volume;
	StkFloat timeCurrent;
	StkFloat timeLast;
	LONG nbInstruments;
	StkFloat t60;
	int counter;
	bool settling;
	int currentVoice;
	int sampler;
	bool instrumentChange;
	bool multiInput;

	VoicerData();
	~VoicerData();

	bool shutUp();
	bool standUp();

	bool init();

	void initInstrumentsData();
	void cleanUp();
};
#endif