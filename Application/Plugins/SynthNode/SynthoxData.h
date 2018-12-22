// SynthoxData.h
//------------------------------------------------------------

// STK Includes
//------------------------------------------------------------
#include "Instrmnt.h"
#include "Voicer.h"
#include "RtAudio.h"
#include "SineWave.h"
#include "Plucked.h"
#include "Clarinet.h"
#include "Sitar.h"
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

