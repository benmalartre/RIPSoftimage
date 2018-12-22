#ifndef FLAG_INCLUDES_XSI_COMMON
#define FLAG_INCLUDES_XSI_COMMON

#include <xsi_application.h>
#include <xsi_status.h>
#include <xsi_math.h>
#include <xsi_utils.h>
#include <xsi_decl.h>
#include <xsi_factory.h>
#include <xsi_selection.h>
#include <xsi_comapihandler.h>
#include <xsi_time.h>
#include <xsi_random.h>

#include <xsi_base.h>
#include <xsi_value.h>
#include <xsi_ref.h>
#include <xsi_boolarray.h>
#include <xsi_vector3.h>
#include <xsi_string.h>

using namespace XSI;
using namespace MATH;

#include <stdio.h>							// required by misc
#include <math.h>							// required by math stuff
#include <limits.h>							// required by grid class
#include <string>							// required by wstring
#include <ctime>							// required by clock()
#include <fstream>							// required by ifstream, ofstream, ios_base
#include <stdlib.h>							// required by srand()
#include <sys/stat.h>						// required by stat, mkdir
#include <vector>
#ifdef _WIN32
	#include <direct.h>						// required by _mkdir()
#endif 

using namespace std;

#endif // FLAG_INCLUDES_XSI_COMMON