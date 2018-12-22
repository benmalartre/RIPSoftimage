// bSynthInstruments.h

#include "Instrmnt.h"
stk::Instrmnt* voiceByNumber3(int number);
int voiceByNumber2(int number, stk::Instrmnt *instrument);
int voiceByNumber(int number, stk::Instrmnt **instrument);
int voiceByName(char *name, stk::Instrmnt **instrument);

int countVoices(int nArgs, char *args[]);