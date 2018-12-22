// bSynthNodeData.h
//------------------------------------------------------------

// STK Includes
//------------------------------------------------------------
#include "Instrmnt.h"
#include "Voicer.h"
#include "RtAudio.h"
#include "SineWave.h"
#include "Plucked.h"
#include "Clarinet.h"
#include "FileWvOut.h"
#include "RtWvOut.h"

// Softimage Includes(only for debugging purpose)
//------------------------------------------------------------
#include <xsi_application.h>
using namespace XSI;

#define SAMPLE_RATE 44100.0

// Specific Includes
//------------------------------------------------------------
#include <cstdlib>
#include <signal.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

// namespaces
//------------------------------------------------------------
using namespace stk;

class synthNodeData
{
public:
	RtAudio dac;
	std::vector<Instrmnt*> instruments;

	int channels;
	int maxTick;

	Voicer* voicer;
	int nbVoices;
	bool enable;
	StkFloat frequency;
	StkFloat volume;
	StkFloat note;
	StkFloat velocity;
	StkFloat timeCurrent;
	StkFloat timeLast;
	int instrumentCurrent;
	int instrumentLast;
	StkFloat t60;
	int counter;
	bool settling;
	int currentVoice;
	StkFloat tempo;
	int sampler;
	StkFloat* toXSI;

	synthNodeData();
	~synthNodeData();

	bool shutUp();
	bool standUp();

	bool init();
};