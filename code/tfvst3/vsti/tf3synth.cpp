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

#include <QtCore/QFile>

#include <windows.h>

#include "tfvsti.hpp"
#include "gmnames.hpp"

//-----------------------------------------------------------------------------------------
// tf3SynthProgram
//-----------------------------------------------------------------------------------------

void tf3SynthProgram::loadDefault(int i)
{
	// Default Program Values
	memcpy(params, TF_DEFAULTPROG, TF_PARAM_COUNT * sizeof(eF32));

    char str[10];
	_itoa(i, str, 10);
    strcpy(name, "Prog ");
    strcat(name, str);
}

//-----------------------------------------------------------------------------------------
// tf3Synth
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
tf3Synth::tf3Synth (audioMasterCallback audioMaster, void* hInstance)
	: AudioEffectX (audioMaster, kNumPrograms, TF_PARAM_COUNT)
{
	// Initialize module path
	eChar mpath[512];
	eMemSet(mpath, 0, 512);
	GetModuleFileName((HMODULE)hInstance, mpath, 512);

	eChar *str = &mpath[511];
	while (str != mpath && *str!='/' && *str!='\\')
	{
		*str-- = '\0';
	}
	
	modulePath = QString(mpath);

	// Initialize tunefish
	tf = new tfInstrument();

	// initialize programs
	for (long i = 0; i < kNumPrograms; i++)
		programs[i].loadDefault(i);

	loadProgramAll();

	for (long i = 0; i < 16; i++)
		channelPrograms[i] = i;

	if (programs)
		setProgram (0);

    editor = new tfEditor(this);
	
	if (audioMaster)
	{
		setNumInputs (0);				// no inputs
		setNumOutputs (kNumOutputs);	// 2 outputs, 1 for each oscillator
		canProcessReplacing ();
		hasVu (false);
		hasClip (false);
		isSynth ();
		setUniqueID ('TF3');			// <<<! *must* change this!!!!
	}
	initProcess ();

	suspend ();
}

//-----------------------------------------------------------------------------------------
tf3Synth::~tf3Synth ()
{
	eSAFE_DELETE(editor);
	eSAFE_DELETE(tf);
}

//-----------------------------------------------------------------------------------------
void tf3Synth::setProgram (long program)
{
    char str[1024];
    sprintf(str, "setProgram(%i)\n", program);
    LOG(str);

	if (program < 0 || program >= kNumPrograms)
		return;

	// write program from tunefish to program list before switching
	tf3SynthProgram *ap = &programs[curProgram];
	for(int i=0;i<TF_PARAM_COUNT;i++)
		ap->params[i] = tf->getParam(i);

	// switch program
	curProgram = program;
	
	// load new program to into tunefish
	ap = &programs[curProgram];	
	for(int i=0;i<TF_PARAM_COUNT;i++)
		tf->setParam(i, ap->params[i]);
}

void tf3Synth::writeProgramToPresets()
{
    tf3SynthProgram *ap = &programs[curProgram];
    for(int i=0;i<TF_PARAM_COUNT;i++)
        ap->params[i] = tf->getParam(i);
}

void tf3Synth::loadProgramFromPresets()
{
    tf3SynthProgram *ap = &programs[curProgram];	
    for(int i=0;i<TF_PARAM_COUNT;i++)
        tf->setParam(i, ap->params[i]);
}

//-----------------------------------------------------------------------------------------
void tf3Synth::setProgramName (char *name)
{
    LOG("setProgramName()\n");
	strcpy (programs[curProgram].name, name);
}

//-----------------------------------------------------------------------------------------
void tf3Synth::getProgramName (char *name)
{
    LOG("getProgramName()\n");
	strcpy (name, programs[curProgram].name);
}

//-----------------------------------------------------------------------------------------
void tf3Synth::getParameterLabel (long index, char *label)
{
    LOG("getParameterLabel()\n");
	strcpy(label, "");
}

//-----------------------------------------------------------------------------------------
void tf3Synth::getParameterDisplay (long index, char *text)
{
    LOG("getParameterDisplay()\n");
	text[0] = 0;
	float2string (tf->getParam(index), text);
}

//-----------------------------------------------------------------------------------------
void tf3Synth::getParameterName (long index, char *label)
{
    char str[1024];
    sprintf(str, "setParameterName(%i)\n", index);
    LOG(str);

	strcpy(label, TF_NAMES[index]);
}

//-----------------------------------------------------------------------------------------
void tf3Synth::setParameter (long index, float value)
{
    LOG("setParameter()\n");
	tf3SynthProgram *ap = &programs[curProgram];
	tf->setParam(index, value);
    ap->params[index] = value;
}

//-----------------------------------------------------------------------------------------
float tf3Synth::getParameter (long index)
{
    char str[1024];
    sprintf(str, "getParameter(%i)\n", index);
    LOG(str);
	return tf->getParam(index);
}

//-----------------------------------------------------------------------------------------
bool tf3Synth::getOutputProperties (long index, VstPinProperties* properties)
{
    LOG("getOutputProperties()\n");
	if (index < kNumOutputs)
	{
		sprintf (properties->label, "Vstx %1d", index + 1);
		properties->flags = kVstPinIsActive;
		if (index < 2)
			properties->flags |= kVstPinIsStereo;	// make channel 1+2 stereo
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------
bool tf3Synth::getProgramNameIndexed (long category, long index, char* text)
{
    char str[1024];
    sprintf(str, "getProgramNameIndexed(%i, %i)\n", category, index);
    LOG(str);

	if (index < kNumPrograms)
	{
		strcpy (text, programs[index].name);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------
bool tf3Synth::setProgramNameIndexed (long category, long index, const char* text)
{
	char str[1024];
	sprintf(str, "setProgramNameIndexed(%i, %i)\n", category, index);
	LOG(str);

	if (index < kNumPrograms)
	{
		strcpy (programs[index].name, text);
		return true;
	}
	return false;
}

void tf3Synth::getProgramData(long index, tf3SynthProgram *data)
{
    eMemCopy(data, &programs[index], sizeof(tf3SynthProgram));
}

void tf3Synth::setProgramData(long index, tf3SynthProgram *data)
{
    eMemCopy(&programs[index], data, sizeof(tf3SynthProgram));
}

//-----------------------------------------------------------------------------------------
bool tf3Synth::copyProgram (long destination)
{
    LOG("copyProgram()\n");
	if (destination < kNumPrograms)
	{
		programs[destination] = programs[curProgram];
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------
bool tf3Synth::getEffectName (char* name)
{
    LOG("getEffectName()\n");
	strcpy (name, "Tunefish v3");
	return true;
}

//-----------------------------------------------------------------------------------------
bool tf3Synth::getVendorString (char* text)
{
    LOG("getVendorString()\n");
	strcpy (text, "Brain Control");
	return true;
}

//-----------------------------------------------------------------------------------------
bool tf3Synth::getProductString (char* text)
{
    LOG("getProductString()\n");
	strcpy (text, "Brain Control Tunefish v3");
	return true;
}

//-----------------------------------------------------------------------------------------
long tf3Synth::canDo (char* text)
{
    LOG("canDo()\n");

	if (!strcmp (text, "receiveVstEvents"))
		return 1;
	if (!strcmp (text, "receiveVstMidiEvent"))
		return 1;
	if (!strcmp (text, "midiProgramNames"))
		return 1;
	return -1;	// explicitly can't do; 0 => don't know
}

// midi program names:
// as an example, GM names are used here. in fact, tf3Synth doesn't even support
// multi-timbral operation so it's really just for demonstration.
// a 'real' instrument would have a number of voices which use the
// programs[channelProgram[channel]] parameters when it receives
// a note on message.

//------------------------------------------------------------------------
long tf3Synth::getMidiProgramName (long channel, MidiProgramName* mpn)
{
    LOG("getMidiProgramName()\n");
	long prg = mpn->thisProgramIndex;
	if (prg < 0 || prg >= 128)
		return 0;
	fillProgram (channel, prg, mpn);
	if (channel == 9)
		return 1;
	return 128L;
}

//------------------------------------------------------------------------
long tf3Synth::getCurrentMidiProgram (long channel, MidiProgramName* mpn)
{
    LOG("getCurrentMidiProgram()\n");
	if (channel < 0 || channel >= 16 || !mpn)
		return -1;
	long prg = channelPrograms[channel];
	mpn->thisProgramIndex = prg;
	fillProgram (channel, prg, mpn);
	return prg;
}

//------------------------------------------------------------------------
void tf3Synth::fillProgram (long channel, long prg, MidiProgramName* mpn)
{
    LOG("fillProgram()\n");
	mpn->midiBankMsb =
	mpn->midiBankLsb = -1;
	mpn->reserved = 0;
	mpn->flags = 0;

	if (channel == 9)	// drums
	{
		strcpy (mpn->name, "Standard");
		mpn->midiProgram = 0;
		mpn->parentCategoryIndex = 0;
	}
	else
	{
		strcpy (mpn->name, GmNames[prg]);
		mpn->midiProgram = (char)prg;
		mpn->parentCategoryIndex = -1;	// for now

		for (long i = 0; i < kNumGmCategories; i++)
		{
			if (prg >= GmCategoriesFirstIndices[i] && prg < GmCategoriesFirstIndices[i + 1])
			{
				mpn->parentCategoryIndex = i;
				break;
			}
		}
	}
}

//------------------------------------------------------------------------
long tf3Synth::getMidiProgramCategory (long channel, MidiProgramCategory* cat)
{
    LOG("getMidiProgramCategory()\n");
	cat->parentCategoryIndex = -1;	// -1:no parent category
	cat->flags = 0;					// reserved, none defined yet, zero.
	long category = cat->thisCategoryIndex;
	if (channel == 9)
	{
		strcpy (cat->name, "Drums");
		return 1;
	}
	if (category >= 0 && category < kNumGmCategories)
		strcpy (cat->name, GmCategories[category]);
	else
		cat->name[0] = 0;
	return kNumGmCategories;
}

//------------------------------------------------------------------------
bool tf3Synth::hasMidiProgramsChanged (long channel)
{
    LOG("hasMidiProgramsChanged()\n");
	return false;	// updateDisplay ()
}

//------------------------------------------------------------------------
bool tf3Synth::getMidiKeyName (long channel, MidiKeyName* key)
								// struct will be filled with information for 'thisProgramIndex' and 'thisKeyNumber'
								// if keyName is "" the standard name of the key will be displayed.
								// if false is returned, no MidiKeyNames defined for 'thisProgramIndex'.
{
    LOG("getMidiKeyName()\n");
	// key->thisProgramIndex;		// >= 0. fill struct for this program index.
	// key->thisKeyNumber;			// 0 - 127. fill struct for this key number.
	key->keyName[0] = 0;
	key->reserved = 0;				// zero
	key->flags = 0;					// reserved, none defined yet, zero.
	return false;
}

bool tf3Synth::loadProgramAll()
{
	for(int i=0;i<kNumPrograms;i++)
	{
		if (!loadProgram(i))
			return false;

        if (i == curProgram)
        {
            // load new program to into tunefish
            tf3SynthProgram *ap = &programs[curProgram];	
            for(int i=0;i<TF_PARAM_COUNT;i++)
                tf->setParam(i, ap->params[i]);
        }
	}

	return true;
}

bool tf3Synth::loadProgram()
{
	return loadProgram(curProgram);
}

bool tf3Synth::loadProgram(long index)
{
	QString path = modulePath + "tf3programs\\program" + QString::number(index) + ".txt";
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly))
		return false;

	QString name(file.readLine());
	eStrCopy(programs[index].name, name.trimmed().toAscii().constData());

	while(true)
	{
		QString line(file.readLine());

		if (line.size() == 0)
		{
			file.close();
			return true;
		}

		QStringList parts = line.split(";");
		if (parts.size() == 2)
		{
			QString key = parts[0];
			eF32 value = parts[1].toFloat();

			for(eU32 i=0;i<TF_PARAM_COUNT;i++)
			{
				if (key == TF_NAMES[i])
				{
					programs[index].params[i] = value;
					break;
				}
			}
		}
	}
}

bool tf3Synth::saveProgramAll()
{
	for(int i=0;i<kNumPrograms;i++)
	{
		if (!saveProgram(i))
			return false;
	}

	return true;
}

bool tf3Synth::saveProgram()
{
	return saveProgram(curProgram);
}

bool tf3Synth::saveProgram(long index)
{
	QString path = modulePath + "tf3programs\\program" + QString::number(index) + ".txt";
	QFile file(path);
	
	if (!file.open(QIODevice::WriteOnly))
		return false;
	
	file.write(programs[index].name);
	file.write("\r\n");

	for(eU32 i=0;i<TF_PARAM_COUNT;i++)
	{
		file.write(TF_NAMES[i]);
		file.write(";");
		file.write(QString::number(programs[index].params[i]).toAscii().constData());
		file.write("\r\n");
	}

	file.close();

	return true;
}

bool tf3Synth::copyProgram()
{
    tf3SynthProgram *ap = &copiedProgram;
	for(int i=0;i<TF_PARAM_COUNT;i++)
		ap->params[i] = tf->getParam(i);

    eStrCopy(ap->name, programs[curProgram].name);

    return true;
}

bool tf3Synth::pasteProgram()
{
    tf3SynthProgram *ap = &copiedProgram;
	for(int i=0;i<TF_PARAM_COUNT;i++)    
        tf->setParam(i, ap->params[i]);

    eStrCopy(programs[curProgram].name, ap->name);

    return true;
}

tfInstrument * tf3Synth::getTunefish()
{
    return tf;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void tf3Synth::setSampleRate (eF32 sampleRate)
{
    LOG("setSampleRate()\n");
	AudioEffectX::setSampleRate (sampleRate);
	
	tf->setSampleRate(sampleRate);
}

//-----------------------------------------------------------------------------------------
void tf3Synth::setBlockSize (long blockSize)
{
    LOG("setBlockSize()\n");
	AudioEffectX::setBlockSize (blockSize);
}

//-----------------------------------------------------------------------------------------
void tf3Synth::resume ()
{
    LOG("resume()\n");
	wantEvents ();
}

//-----------------------------------------------------------------------------------------
void tf3Synth::initProcess ()
{
	
}

//-----------------------------------------------------------------------------------------
void tf3Synth::process (eF32 **inputs, eF32 **outputs, long sampleFrames)
{
	// process () is required, and accumulating (out += h)
	// processReplacing () is optional, and in place (out = h). even though
	// processReplacing () is optional, it is very highly recommended to support it

	tf->process(outputs, sampleFrames);
}

//-----------------------------------------------------------------------------------------
void tf3Synth::processReplacing (eF32 **inputs, eF32 **outputs, long sampleFrames)
{
	memset (outputs[0], 0, sampleFrames * sizeof (eF32));
	memset (outputs[1], 0, sampleFrames * sizeof (eF32));
	process (inputs, outputs, sampleFrames);
}

//-----------------------------------------------------------------------------------------
long tf3Synth::processEvents (VstEvents* ev)
{
    LOG("processEvents()\n");
	for (eS32 i = 0; i < ev->numEvents; i++)
	{
		if ((ev->events[i])->type != kVstMidiType)
			continue;
		VstMidiEvent* event = (VstMidiEvent*)ev->events[i];
		char* midiData = event->midiData;
		eS32 status = midiData[0] & 0xf0;		// ignoring channel
		if (status == 0x90 || status == 0x80)	// we only look at notes
		{
			eS32 note = midiData[1] & 0x7f;
			eS32 velocity = midiData[2] & 0x7f;
			if (status == 0x80)
				velocity = 0;	// note off by velocity 0
			if (!velocity)
				tf->noteOff(note);
			else
				tf->noteOn(note, velocity, 0, 0);
		}
		else if (status == 0xb0)
		{
			if (midiData[1] == 0x7e || midiData[1] == 0x7b)	// all notes off
				tf->allNotesOff();
		}
		event++;
	}
	return 1;	// want more
}


