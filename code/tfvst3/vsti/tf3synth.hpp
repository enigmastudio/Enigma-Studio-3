/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   This file is part of
 *
 *    ------  /   /  /\   /  /---  -----  /  ----  /   /
 *       /   /   /  /  \ /  /---   -/-   /   \--  /---/   version 3
 *      /    \---  /    /  /---    /    /  ----/ /   /.
 *
 *       t i n y   m u s i c   s y n t h e s i z e r
 *
 *   Copyright © 2003-2010 Brain Control, all rights reserved.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __tf3Synth__
#define __tf3Synth__

#ifdef TF_LOGGING
#define LOG(x) FILE *out = fopen("logfile.txt", "ab"); \
fprintf(out, "%s", x); \
fclose(out);
#else
#define LOG(x)
#endif


//------------------------------------------------------------------------------------------
enum
{
	// Global
	kNumPrograms = 128,
	kNumOutputs = 2,
};

//------------------------------------------------------------------------------------------
// tf3SynthProgram
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
class tf3SynthProgram
{
friend class tf3Synth;
public:
	void loadDefault(int i);
private:
	float params[TF_PARAM_COUNT];
	char name[24];
};

//------------------------------------------------------------------------------------------
// tf3Synth
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
class tf3Synth : public AudioEffectX
{
public:
	tf3Synth (audioMasterCallback audioMaster, void* hInstance);
	~tf3Synth ();

	virtual void process (float **inputs, float **outputs, long sampleframes);
	virtual void processReplacing (float **inputs, float **outputs, long sampleframes);
	virtual long processEvents (VstEvents* events);

	virtual void setProgram (long program);
	virtual void setProgramName (char *name);
	virtual bool setProgramNameIndexed (long category, long index, const char* text);
	virtual void getProgramName (char *name);
	virtual bool getProgramNameIndexed (long category, long index, char* text);
	virtual bool copyProgram (long destination);

	virtual void setParameter (long index, float value);
	virtual float getParameter (long index);
	virtual void getParameterLabel (long index, char *label);
	virtual void getParameterDisplay (long index, char *text);
	virtual void getParameterName (long index, char *text);
	
	virtual void setSampleRate (float sampleRate);
	virtual void setBlockSize (long blockSize);
	
	virtual void resume ();

	virtual bool getOutputProperties (long index, VstPinProperties* properties);
		
	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual long getVendorVersion () { return 1000; }
	virtual long canDo (char* text);

	virtual long getMidiProgramName (long channel, MidiProgramName* midiProgramName);
	virtual long getCurrentMidiProgram (long channel, MidiProgramName* currentProgram);
	virtual long getMidiProgramCategory (long channel, MidiProgramCategory* category);
	virtual bool hasMidiProgramsChanged (long channel);
	virtual bool getMidiKeyName (long channel, MidiKeyName* keyName);

	bool loadProgram();
	bool loadProgram(long index);
	bool loadProgramAll();
	bool saveProgram();
	bool saveProgram(long index);
	bool saveProgramAll();
    bool copyProgram();
    bool pasteProgram();

    void getProgramData(long index, tf3SynthProgram *data);
    void setProgramData(long index, tf3SynthProgram *data);

    void writeProgramToPresets();
    void loadProgramFromPresets();

    tfInstrument * getTunefish();

private:
	void initProcess ();
	void noteOn (long note, long velocity, long delta);
	void noteOff ();
	void fillProgram (long channel, long prg, MidiProgramName* mpn);

	tf3SynthProgram programs[kNumPrograms];
    tf3SynthProgram copiedProgram;

	tfInstrument * tf;

	long channelPrograms[16];
	QString modulePath;
};

#endif
