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

#ifndef PROFILER_HPP
#define PROFILER_HPP

// Depending on define enable or disable
// profiler. The profiler should be
// disabled when shipping intros.
//#ifdef eUSE_PROFILER 
#if defined(eUSE_PROFILER) && defined(eEDITOR)
    #define ePROFILER_DEFINE(zone, name)        eProfiler::Zone zone(name)
    #define ePROFILER_SCOPE(zone)               eProfiler::Scope scope(zone)

    #define ePROFILER_NAMED_ZONE(zone, name)    static eProfiler::Zone zone(name);  \
                                                eProfiler::Scope scope(zone);

    #define ePROFILER_ZONE(name)                ePROFILER_NAMED_ZONE(eTOKENPASTE(zone_, eTOKENPASTE(__LINE__, __COUNTER__)), name)
#else
    #define ePROFILER_DEFINE(zone, name)
    #define ePROFILER_SCOPE(zone)
    #define ePROFILER_NAMED_ZONE(zone, name)
    #define ePROFILER_ZONE(name)
#endif

#if defined(eUSE_PROFILER) && defined(eEDITOR)

// Profiling system's manager class. This class
// manages all profling zones and calculates all
// related values using information from zones.
class eProfiler
{
public:
    // Defines by which criterion the zones are sorted.
    enum SortMode
    {
        SORT_NAME,
        SORT_HIERTIME,
        SORT_SELFTIME,
        SORT_CALLCOUNT
    };

public:
    // Represents a profiling zone. For convenience,
    // use the ePROFILER_DEFINE macro to declare a new
    // in your global scope.
    class Zone;
    typedef eArray<Zone *> ZonePtrArray;

    class Zone
    {
        friend class eProfiler;

    public:
        Zone();
        Zone(const eChar *name);

        void            enter(Zone *lastZone);
        void            leave(Zone *nextZone);

        void            clear();

        const eChar *   getName() const;
        eColor          getColor() const;
        eU64            getSelfTicks() const;
        eU64            getHierTicks() const;
        eF32            getSelfTimeMs() const;
        eF32            getHierTimeMs() const;
        eU32            getCallCount() const;

    private:
        eChar           m_name[eMAX_NAME_LENGTH];
        eColor          m_color;
        eU64            m_selfTotal;
        eU64            m_hierTotal;
        eU64            m_selfStart;
        eU64            m_hierStart;
        eU32            m_callCount;
        eU32            m_zoneIndex;
    };

    // Used to profile a particular code section.
    // Zone is automatically left, when class
    // gets out of scope. For convenience use the
    // ePROFILE_SCOPE macro.
    class Scope
    {
    public:
        eFORCEINLINE Scope(Zone &zone)
        {
            enterZone(zone);
        }

        eFORCEINLINE ~Scope()
        {
            leaveZone();
        }
    };

public:
    static void         enterZone(Zone &zone);
    static void         leaveZone();
    static void         beginFrame();
    static void         endFrame();

    static void         setSortMode(SortMode mode);

    static SortMode     getSortMode();
    static eU32         getZoneCount();
    static const Zone & getZone(eU32 index);
    static eF32         getLastFrameTimeMs();

private:
    static const eU32   MAX_STACK_DEPTH = 512;
    static const eU32   MAX_ZONE_COUNT = 256;

private:
    static Zone *       m_zoneStack[MAX_STACK_DEPTH];
    static Zone *       m_zonesByIndex[MAX_ZONE_COUNT];
    static Zone         m_zonesLastFrame[MAX_ZONE_COUNT];
    static eU32         m_stackIndex;
    static eU32         m_zoneCount;
    static eU64         m_frameStartTime;
    static eU64         m_frameDuration;
    static SortMode     m_sortMode;
};

#endif

#endif // PROFILER_HPP