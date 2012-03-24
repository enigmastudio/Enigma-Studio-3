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

#ifndef PATH_HPP
#define PATH_HPP

class ePath
{
public:
    enum InterpolationType
    {
        INTERPOLATE_LINEAR,
        INTERPOLATE_SMOOTH,
        INTERPOLATE_STEP
    };

    enum WaypointType
    {
        WAYPOINT_WAYPOINT,
        WAYPOINT_TSHADER
    };

    struct Waypoint
    {
        WaypointType        type;
        InterpolationType   interpolation;
        eF32                time;
        eVector4            vec;
        eTShader *          tshader;
    };

public:
    void                    addWaypointVec(eF32 time, const eVector4 &vec, InterpolationType interpolation);
    void                    addWaypointScalar(eF32 time, eF32 scalar, InterpolationType interpolation);
    void                    addWaypointTShader(eF32 time, eTShader *ts);

    void                    clear();
    void                    merge(const ePath &path);
    eVector4                process(eF32 time, eConstPtr callerId) const;

#ifdef eEDITOR
    void                    sample(eVector2Array rotSamples[3], eVector2Array transSamples[3], eVector2Array scaleSamples[3], eConstPtr callerId) const;
#endif

    const Waypoint &        getWaypoint(eU32 index) const;
    Waypoint &              getWaypoint(eU32 index);
    eU32                    getWaypointCount() const;

    eF32                    getRunningTime() const;
    eF32                    getStartTime() const;
    eF32                    getEndTime() const;

private:
    void                    _insertWaypoint(const Waypoint &wp);
    const Waypoint *        _findWaypoint(eF32 time) const;
    eVector4                _executeTShader(eTShader &ts, eF32 time, eConstPtr callerId) const;

#ifdef eEDITOR
    void                    _subdivideSpline(const eVector4 &wp0, const eVector4 &wp1, const eVector4 &wp2, const eVector4 &wp3, eVector2Array samples[3]) const;
    void                    _subdivideBezier(const eVector2 &wp0, const eVector2 &wp1, const eVector2 &wp2, const eVector2 &wp3, eVector2Array &samples) const;
    eBool                   _isBezierStraight(const eVector2 &wp0, const eVector2 &wp1, const eVector2 &wp2, const eVector2 &wp3) const;
    void                    _addWaypointToSamples(const Waypoint &wp, eVector2Array rotSamples[3], eVector2Array transSamples[3], eVector2Array scaleSamples[3]) const;
#endif

private:
    typedef eArray<Waypoint> WaypointArray;

private:
    WaypointArray           m_waypoints;
};

// Class for sampling a path and storing those samples.
// This is used for a faster evaluation in time critical code.
class ePathSampler
{
public:
    void                    sample(const ePath &path, eF32 startTime, eF32 endTime, eU32 sampleCount);
    void                    sample(const ePath &path);
    const eVector4 &        evaluate(eF32 time) const;

private:
    static const eU32       SAMPLES_PER_SEC = 100;

private:
    eArray<eVector4>        m_samples;
    eF32                    m_startTime;
    eF32                    m_endTime;
    eF32                    m_stepInv;
};

#endif // PATH_HPP