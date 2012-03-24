//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// -
// Example tf3Synth (VST 2.0)
// A simple 2 oscillators test 'synth',
// Each oscillator has waveform, frequency, and volume
//
// *very* basic monophonic 'synth' example. you should not attempt to use this
// example to start a serious virtual instrument; it is intended to demonstrate
// how VstEvents ('MIDI') are handled, but not how a virtual analog synth works.
// there are numerous much better examples on the web which show how to deal with
// bandlimited waveforms etc.
// -
// © 2003, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#include "tfvsti.hpp"

bool oome = false;

#if MAC
#pragma export on
#endif

//------------------------------------------------------------------------
// Prototype of the export function main
//------------------------------------------------------------------------
#if BEOS
#define main main_plugin
extern "C" __declspec(dllexport) AEffect *main_plugin (audioMasterCallback audioMaster);

#elif MACX
#define main main_macho
extern "C" AEffect *main_macho (audioMasterCallback audioMaster);

#else
int __declspec(dllexport) main (audioMasterCallback audioMaster);
#endif

//------------------------------------------------------------------------
#if WIN32
#include <windows.h>
void* hInstance;
BOOL WINAPI DllMain (HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved)
{
	hInstance = hInst;

	static bool ownApplication = FALSE;

    if ( dwReason == DLL_PROCESS_ATTACH )
    {
         ownApplication = QMfcApp::pluginInstance( hInstance );
		 qApp->setStyle("clearlooks");
    }
    if ( dwReason == DLL_PROCESS_DETACH && ownApplication )
    {
         delete qApp;
    }

    return TRUE;
}
#endif


//------------------------------------------------------------------------
int main (audioMasterCallback audioMaster)
{
	// Get VST Version
	if (!audioMaster (0, audioMasterVersion, 0, 0, 0, 0))
		return 0;  // old version

	// Create the AudioEffect
	tf3Synth* effect = new tf3Synth (audioMaster, hInstance);
	if (!effect)
		return 0;

	// Check if no problem in constructor of AGain
	if (oome)
	{
		delete effect;
		return 0;
	}
	return (int)effect->getAeffect ();
}

#if MAC
#pragma export off
#endif

