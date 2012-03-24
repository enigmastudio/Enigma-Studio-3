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

#if defined(eUSE_PROFILER) && defined(eEDITOR)

eProfiler::Zone::Zone()
{
}

eProfiler::Zone::Zone(const eChar *name) :
    m_selfTotal(0),
    m_hierTotal(0),
    m_selfStart(0),
    m_hierStart(0),
    m_callCount(0)
{
    eASSERT(name != eNULL);

    eStrNCopy(m_name, name, eMAX_NAME_LENGTH);

    m_zoneIndex = m_zoneCount;
    m_zonesByIndex[m_zoneCount++] = this;

    eRandomize(eHashStr(name));
    m_color.fromHsv(eRandom(0, 359), eRandom(128, 255), eRandom(128, 255));
}

void eProfiler::Zone::enter(Zone *lastZone)
{
    const eU64 enterTime = eTimer::getTickCount();

    m_selfStart = enterTime;
    m_hierStart = enterTime;
    m_callCount++;

    if (lastZone)
    {
        lastZone->m_selfTotal += m_selfStart-lastZone->m_selfStart;
    }
}

void eProfiler::Zone::leave(Zone *nextZone)
{
    const eU64 endTime = eTimer::getTickCount();

    m_selfTotal += (endTime-m_selfStart);
    m_hierTotal += (endTime-m_hierStart);

    if (nextZone)
    {
        nextZone->m_selfStart = endTime;
    }
}

void eProfiler::Zone::clear()
{
    m_callCount = 0;
    m_selfTotal = 0;
    m_hierTotal = 0;
}

const eChar * eProfiler::Zone::getName() const
{
    return m_name;
}

eColor eProfiler::Zone::getColor() const
{
    return m_color;
}

eU64 eProfiler::Zone::getSelfTicks() const
{
    return m_selfTotal;
}

eU64 eProfiler::Zone::getHierTicks() const
{
    return m_hierTotal;
}

eF32 eProfiler::Zone::getSelfTimeMs() const
{
    return (eF32)((eF64)m_selfTotal/(eF64)eTimer::getFrequency()*1000.0);
}

eF32 eProfiler::Zone::getHierTimeMs() const
{
    return (eF32)((eF64)m_hierTotal/(eF64)eTimer::getFrequency()*1000.0);
}

eU32 eProfiler::Zone::getCallCount() const
{
    return m_callCount;
}

ePROFILER_DEFINE(g_profGlobal, "Global zone");

// Initialize static members.
eProfiler::Zone *   eProfiler::m_zoneStack[eProfiler::MAX_STACK_DEPTH] =
{
#ifdef eUSE_PROFILER
    &g_profGlobal
#endif
};

eProfiler::Zone *   eProfiler::m_zonesByIndex[eProfiler::MAX_ZONE_COUNT];
eProfiler::Zone     eProfiler::m_zonesLastFrame[eProfiler::MAX_ZONE_COUNT];
eU32                eProfiler::m_stackIndex = 0;
eU32                eProfiler::m_zoneCount = 0;
eU64                eProfiler::m_frameStartTime = 0;
eU64                eProfiler::m_frameDuration = 0;
eProfiler::SortMode eProfiler::m_sortMode = eProfiler::SORT_SELFTIME;

void eProfiler::enterZone(Zone &zone)
{
    Zone *lastZone = m_zoneStack[m_stackIndex];
    eASSERT(lastZone != eNULL);

    m_zoneStack[++m_stackIndex] = &zone;
    zone.enter(lastZone);
}

void eProfiler::leaveZone()
{
    Zone *zone = m_zoneStack[m_stackIndex--];
    eASSERT(zone != eNULL);
    Zone *nextZone = m_zoneStack[m_stackIndex];
    eASSERT(nextZone != eNULL);

    zone->leave(nextZone);
}

void eProfiler::beginFrame()
{
#ifdef eUSE_PROFILER
    for (eU32 i=0; i<m_zoneCount; i++)
    {
        m_zonesByIndex[i]->clear();
    }

    g_profGlobal.enter(eNULL);
    m_frameStartTime = eTimer::getTickCount();
#endif
}

void eProfiler::endFrame()
{
#ifdef eUSE_PROFILER
    m_frameDuration = eTimer::getTickCount()-m_frameStartTime;
    g_profGlobal.leave(eNULL);

    // Do a bubble-sort by self-time, spend in the
    // corresponding zone.
    eBool sorted;

    do
    {
        sorted = eTRUE;

        for (eU32 i=0; i<m_zoneCount-1; i++)
        {
            if (m_zonesByIndex[i]->getSelfTicks() > m_zonesByIndex[i+1]->getSelfTicks())
            {
                eSwap(m_zonesByIndex[i], m_zonesByIndex[i+1]);
                sorted = eFALSE;
            }
        }
    }
    while (!sorted);

    // Finally backup zones of last frame, so that
    // zones can be fetched in the next frame.
    for (eU32 i=0; i<m_zoneCount; i++)
    {
        m_zonesLastFrame[i] = *m_zonesByIndex[i];
    }
#endif
}

void eProfiler::setSortMode(SortMode mode)
{
    m_sortMode = mode;
}

eProfiler::SortMode eProfiler::getSortMode()
{
    return m_sortMode;
}

eU32 eProfiler::getZoneCount()
{
    return m_zoneCount;
}

const eProfiler::Zone & eProfiler::getZone(eU32 index)
{
    eASSERT(index < m_zoneCount);
    return m_zonesLastFrame[index];
}

eF32 eProfiler::getLastFrameTimeMs()
{
    return (eF32)((eF64)m_frameDuration/(eF64)eTimer::getFrequency()*1000.0);
}

#endif