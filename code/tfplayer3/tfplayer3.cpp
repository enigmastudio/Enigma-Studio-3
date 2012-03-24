#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../eshared/system/system.hpp"
#include "resource.h"

#ifdef eDEBUG
eInt WINAPI WinMain(HINSTANCE inst, HINSTANCE prevInst, eChar *cmdLine, eInt showCmd)
#else
void WinMainCRTStartup()
#endif
{
#ifdef eDEBUG
    return 0;
#endif
}