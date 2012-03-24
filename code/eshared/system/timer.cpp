/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   This file is part of
 *       _______   ______________  ______     _____
 *      / ____/ | / /  _/ ____/  |/  /   |   |__  /
 *     / __/ /  |/ // // / __/ /|_/ / /| |    /_ <
 *    / /___/ /|  // // /_/ / /  / / ___ |  ___/ /
 *   /_____/_/ |_/___/\____/_/  /_/_/  |_| /____/.
 *
 *   Copyright © 2003-2010 Brain Control, all rights reserved.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "system.hpp"

eU64 eTimer::m_freq = 0;
eU64 eTimer::m_correction = 0;
eBool eTimer::m_inited = eFALSE;

eTimer::eTimer()
{
    _initialize();
    restart();
}

void eTimer::restart()
{
#ifdef _WIN32
    QueryPerformanceCounter((LARGE_INTEGER *)&m_startTime);
#endif
}

eU32 eTimer::getElapsedMs() const
{
#ifdef _WIN32
    eU64 current;

    QueryPerformanceCounter((LARGE_INTEGER *)&current);
    return eFtoL((eF32)((eF64)(current-m_startTime-m_correction)/(eF64)m_freq*1000.0));
#else
    return 0;
#endif
}

eU32 eTimer::getTimeMs()
{
    return eFtoL((eF32)((eF64)getTickCount()/(eF64)m_freq*1000.0));
}

eU64 eTimer::getTickCount()
{
#ifdef _WIN32
    eU64 current;

    QueryPerformanceCounter((LARGE_INTEGER *)&current);
    return current-m_correction;
#else
    return 0;
#endif
}

eU64 eTimer::getFrequency()
{
    return m_freq;
}

void eTimer::_initialize()
{
    if (m_inited)
    {
        return;
    }

#ifdef _WIN32
    // Get timer frequency.
    const BOOL res = QueryPerformanceFrequency((LARGE_INTEGER *)&m_freq);
    eASSERT(res == TRUE);
    
    // Get correction value.
    eU64 start, stop;

    QueryPerformanceCounter((LARGE_INTEGER *)&start);
    QueryPerformanceCounter((LARGE_INTEGER *)&stop);

    m_correction = stop-start;
    m_inited = eTRUE;
#endif
}