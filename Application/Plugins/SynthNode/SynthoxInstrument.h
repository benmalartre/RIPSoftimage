// SynthoxInstrument.h
//------------------------------------------------------------
#ifndef _SYNTHOXINSTRUMENT_H_
#define _SYNTHOXINSTRUMENT_H_
struct InstrumentData
{
public:
	bool changeInstrument;
	LONG instrumentCurrent;
	float note;
	float volume;
	bool noteOn;
	LONG noteTag;
	bool enable;
};

// Defines port, group and map identifiers used for registering the ICENode
enum InstrumentIDs
{
	Instrument_ID_IN_Enable = 0,
	Instrument_ID_IN_Volume = 1,
	Instrument_ID_IN_Note = 2,
	Instrument_ID_IN_NoteOn = 3,
	Instrument_ID_IN_Instrument = 4,
	Instrument_ID_G_100,
	Instrument_ID_OUT_Output = 200,
	Instrument_ID_TYPE_CNS = 400,
	Instrument_ID_STRUCT_CNS,
	Instrument_ID_CTXT_CNS,
	Instrument_ID_UNDEF = ULONG_MAX
};
#endif