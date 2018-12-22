// SynthoxRegister.h
//-----------------------------------------

#ifndef _SYNTHOXREGISTER_H_
#define _SYNTHOXREGISTER_H_

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

// Specific Includes
//------------------------------------------------------------
#include "SynthoxInstruments.h"
#include "SynthoxInstrument.h"

// namespaces
//------------------------------------------------------------
using namespace XSI;
using namespace stk;

#endif