// Miscellaneous parsing and error functions for use with STK demo program.
//
// Gary P. Scavone, 1999.

#include <stdlib.h>
#include <string.h>
#include "bSynthInstruments.h"

// STK Instrument Classes
#include "Clarinet.h"
#include "BlowHole.h"
#include "Saxofony.h"
#include "Flute.h"
#include "Brass.h"
#include "BlowBotl.h"
#include "Bowed.h"
#include "Plucked.h"
#include "StifKarp.h"
#include "Sitar.h"
#include "Mandolin.h"
#include "Rhodey.h"
#include "Wurley.h"
#include "TubeBell.h"
#include "HevyMetl.h"
#include "PercFlut.h"
#include "BeeThree.h"
#include "FMVoices.h"
#include "VoicForm.h"
#include "Moog.h"
#include "Simple.h"
#include "Drummer.h"
#include "BandedWG.h"
#include "Shakers.h"
#include "ModalBar.h"
#include "Mesh2D.h"
#include "Resonate.h"
#include "Whistle.h"

#include <xsi_application.h>

using namespace stk;
using namespace XSI;

#define NUM_INSTS 28

// The order of the following list is important.  The location of a particular
// instrument in the list should correspond to that instrument's ProgramChange
// number (i.e. Clarinet = ProgramChange 0).
char insts[NUM_INSTS][10] = { "Clarinet", "BlowHole", "Saxofony", "Flute", "Brass",
                              "BlowBotl", "Bowed", "Plucked", "StifKarp", "Sitar", "Mandolin",
                              "Rhodey", "Wurley", "TubeBell", "HevyMetl", "PercFlut",
                              "BeeThree", "FMVoices", "VoicForm", "Moog", "Simple", "Drummer",
                              "BandedWG", "Shakers", "ModalBar", "Mesh2D", "Resonate", "Whistle" };

Instrmnt* voiceByNumber3(int number)
{
	Instrmnt *instrument;

	if     (number==0)  instrument = new Clarinet(10.0);
	else if (number==1)  instrument = new BlowHole(10.0);
	else if (number==2)  instrument = new Saxofony(10.0);
	else if (number==3)  instrument = new Flute(10.0);
	else if (number==4)  instrument = new Brass(10.0);
	else if (number==5)  instrument = new BlowBotl();
	else if (number==6)  instrument = new Bowed(10.0);
	else if (number==7)  instrument = new Plucked(5.0);
	else if (number==8)  instrument = new StifKarp(5.0);
	else if (number==9)  instrument = new Sitar(5.0);
	else if (number==10) instrument = new Mandolin(5.0);

	else if (number==11) instrument = new Rhodey;
	else if (number==12) instrument = new Wurley;
	else if (number==13) instrument = new TubeBell;
	else if (number==14) instrument = new HevyMetl();
	else if (number==15) instrument = new PercFlut();
	else if (number==16) instrument = new BeeThree;
	else if (number==17) instrument = new FMVoices;

	else if (number==18) instrument = new VoicForm();
	else if (number==19) instrument = new Moog();
	else if (number==20) instrument = new Simple();
	else if (number==21) instrument = new Drummer();
	else if (number==22) instrument = new BandedWG();
	else if (number==23) instrument = new Shakers();
	else if (number==24) instrument = new ModalBar();
	else if (number==25) instrument = new Mesh2D(10, 10);
	else if (number==26) instrument = new Resonate();
	else if (number==27) instrument = new Whistle();

	else 
	{
		Application().LogMessage(L"\nUnknown instrument requested!\n");
		instrument = NULL;
	}

	return instrument;
}

int voiceByNumber2(int number, Instrmnt* instrument)
{
	int temp = number;
  
	if     (number==0)  instrument = new Clarinet(10.0);
	else if (number==1)  instrument = new BlowHole(10.0);
	else if (number==2)  instrument = new Saxofony(10.0);
	else if (number==3)  instrument = new Flute(10.0);
	else if (number==4)  instrument = new Brass(10.0);
	else if (number==5)  instrument = new BlowBotl();
	else if (number==6)  instrument = new Bowed(10.0);
	else if (number==7)  instrument = new Plucked(5.0);
	else if (number==8)  instrument = new StifKarp(5.0);
	else if (number==9)  instrument = new Sitar(5.0);
	else if (number==10) instrument = new Mandolin(5.0);

	else if (number==11) instrument = new Rhodey;
	else if (number==12) instrument = new Wurley;
	else if (number==13) instrument = new TubeBell;
	else if (number==14) instrument = new HevyMetl();
	else if (number==15) instrument = new PercFlut();
	else if (number==16) instrument = new BeeThree;
	else if (number==17) instrument = new FMVoices;

	else if (number==18) instrument = new VoicForm();
	else if (number==19) instrument = new Moog();
	else if (number==20) instrument = new Simple();
	else if (number==21) instrument = new Drummer();
	else if (number==22) instrument = new BandedWG();
	else if (number==23) instrument = new Shakers();
	else if (number==24) instrument = new ModalBar();
	else if (number==25) instrument = new Mesh2D(10, 10);
	else if (number==26) instrument = new Resonate();
	else if (number==27) instrument = new Whistle();

	else 
	{
		Application().LogMessage(L"\nUnknown instrument requested!\n");
		temp = -1;
	}

	return temp;
}

int voiceByNumber(int number, Instrmnt **instrument)
{
  int temp = number;
  
  if     (number==0)  *instrument = new Clarinet(10.0);
  else if (number==1)  *instrument = new BlowHole(10.0);
  else if (number==2)  *instrument = new Saxofony(10.0);
  else if (number==3)  *instrument = new Flute(10.0);
  else if (number==4)  *instrument = new Brass(10.0);
  else if (number==5)  *instrument = new BlowBotl();
  else if (number==6)  *instrument = new Bowed(10.0);
  else if (number==7)  *instrument = new Plucked(5.0);
  else if (number==8)  *instrument = new StifKarp(5.0);
  else if (number==9)  *instrument = new Sitar(5.0);
  else if (number==10) *instrument = new Mandolin(5.0);

  else if (number==11) *instrument = new Rhodey;
  else if (number==12) *instrument = new Wurley;
  else if (number==13) *instrument = new TubeBell;
  else if (number==14) *instrument = new HevyMetl();
  else if (number==15) *instrument = new PercFlut();
  else if (number==16) *instrument = new BeeThree;
  else if (number==17) *instrument = new FMVoices;

  else if (number==18) *instrument = new VoicForm();
  else if (number==19) *instrument = new Moog();
  else if (number==20) *instrument = new Simple();
  else if (number==21) *instrument = new Drummer();
  else if (number==22) *instrument = new BandedWG();
  else if (number==23) *instrument = new Shakers();
  else if (number==24) *instrument = new ModalBar();
  else if (number==25) *instrument = new Mesh2D(10, 10);
  else if (number==26) *instrument = new Resonate();
  else if (number==27) *instrument = new Whistle();

  else 
  {
		Application().LogMessage(L"\nUnknown instrument requested!\n");
		temp = -1;
  }

  return temp;
}

int voiceByName(char *name, Instrmnt **instrument)
{
  int i = 0, temp = -1, notFound = 1;;

  while ( i < NUM_INSTS && notFound ) {
    if ( !strcmp(name, insts[i]) ) {
      notFound = 0;
      temp = voiceByNumber(i, instrument);
    }
    i++;
  }

  if (temp < 0) 
	  Application().LogMessage("\nUnknown instrument or program change requested!\n");

  return temp;
}

int countVoices(int nArgs, char *args[])
{
  int i = 2, nInstruments = 1;

  while ( i < nArgs ) {
    if ( strncmp( args[i], "-n", 2) == 0 ) {
      if ( i+1 < nArgs && args[i+1][0] != '-' ) {
        nInstruments = atoi( args[i+1] );
        if ( nInstruments < 1 ) nInstruments = 1;
      }
    }
    i++;
  }

  return nInstruments;
}
