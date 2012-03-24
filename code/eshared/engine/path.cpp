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

#include "../eshared.hpp"

void ePath::addWaypointVec(eF32 time, const eVector4 &vec, InterpolationType interpolation)
{
    eASSERT(time >= 0.0f);

    Waypoint wp;

    wp.type           = ePath::WAYPOINT_WAYPOINT;
    wp.interpolation  = interpolation;
    wp.time           = time;
    wp.tshader        = eNULL;
    wp.vec            = vec;

    _insertWaypoint(wp);
}

void ePath::addWaypointScalar(eF32 time, eF32 scalar, InterpolationType interpolation)
{
    addWaypointVec(time, eVector4(scalar, 0.0f, 0.0f, 0.0f), interpolation);
}

void ePath::addWaypointTShader(eF32 time, eTShader *ts)
{
    eASSERT(time >= 0.0f);

    Waypoint wp;

    wp.type          = WAYPOINT_TSHADER;
    wp.interpolation = INTERPOLATE_LINEAR;
    wp.time          = time;
    wp.tshader       = ts;

    _insertWaypoint(wp);
}

void ePath::clear()
{
    m_waypoints.clear();
}

void ePath::merge(const ePath &path)
{
    for (eU32 i=0; i<path.getWaypointCount(); i++)
    {
        _insertWaypoint(path.getWaypoint(i));
    }
}

eVector4 ePath::process(eF32 time, eConstPtr callerId) const
{
    ePROFILER_ZONE("Process path");
    eVector4 res;

    if (m_waypoints.size() == 0)
    {
        return res;
    }

    const Waypoint &wpMin = m_waypoints[0];
    const Waypoint &wpMax = m_waypoints[m_waypoints.size()-1];

    // Time before first waypoint?
    if (time <= wpMin.time || m_waypoints.size() == 1)
    {
        if (wpMin.type == WAYPOINT_WAYPOINT)
        {
            res = wpMin.vec;
        }
        else if (wpMin.type == WAYPOINT_TSHADER)
        {
            res = _executeTShader(*wpMin.tshader, wpMin.time, callerId);
        }
    }
    //  Time after last waypoint?
    else if (time >= wpMax.time)
    {
        if (wpMax.type == WAYPOINT_WAYPOINT)
        {
            res = wpMax.vec;
        }
        else if (wpMax.type == WAYPOINT_TSHADER)
        {
            res = _executeTShader(*wpMax.tshader, wpMax.time, callerId);
        }
    }
    else // Time inbetween first and last waypoint?
    {
        for (eInt i=0; i<(eInt)m_waypoints.size()-1; i++)
        {
            const Waypoint &wpFrom = m_waypoints[i];
            const Waypoint &wpTo = m_waypoints[i+1];

            if (wpFrom.time <= time && wpTo.time > time)
            {
                const Waypoint &wpPrev = (i > 0 ? m_waypoints[i-1] : wpFrom);
                const Waypoint &wpNext = (i < (eInt)m_waypoints.size()-2 ? m_waypoints[i+2] : wpTo);

                if (wpFrom.type == WAYPOINT_WAYPOINT)
                {
                    const eF32 t = (time-wpFrom.time)/(wpTo.time-wpFrom.time);

                    if (wpFrom.interpolation == ePath::INTERPOLATE_LINEAR)
                    {
                        res = wpFrom.vec.lerp(wpTo.vec, t);
                    }
                    else if (wpFrom.interpolation == ePath::INTERPOLATE_STEP)
                    {
                        res = wpFrom.vec;
                    }
                    else
                    {
                        res = eVector4::catmullRom(t, wpPrev.vec, wpFrom.vec, wpTo.vec, wpNext.vec);
                    }
                }
                else if (wpFrom.type == WAYPOINT_TSHADER)
                {
                    res = _executeTShader(*wpFrom.tshader, time, callerId);
                }

                break;
            }
        }
    }

    return res;
}

#ifdef eEDITOR
void ePath::sample(eVector2Array rotSamples[3], eVector2Array transSamples[3], eVector2Array scaleSamples[3], eConstPtr callerId) const
{
    /*
    // Clear sampling arrays.
    for (eU32 i=0; i<3; i++)
    {
        rotSamples[i].clear();
        transSamples[i].clear();
        scaleSamples[i].clear();
    }

    // Sample the path.
    for (eInt i=0; i<(eInt)m_waypoints.size()-1; i++)
    {
        // Retrieve the four waypoints.
        const Waypoint &wpFrom = m_waypoints[i];
        const Waypoint &wpTo = m_waypoints[i+1];
        const Waypoint &wpPrev = (i > 0 ? m_waypoints[i-1] : wpFrom);
        const Waypoint &wpNext = (i < (eInt)m_waypoints.size()-2 ? m_waypoints[i+2] : wpTo);

        // Sample between from and to waypoints.
        if (wpFrom.type == WAYPOINT_WAYPOINT)
        {
            if (wpFrom.interpolation == ePath::INTERPOLATE_LINEAR)
            {
                _addWaypointToSamples(wpFrom, rotSamples, transSamples, scaleSamples);
            }
            else
            {
                _subdivideSpline(eVector4(wpPrev.posornt.pos, wpPrev.time),
                                 eVector4(wpFrom.posornt.pos, wpFrom.time),
                                 eVector4(wpTo.posornt.pos, wpTo.time),
                                 eVector4(wpNext.posornt.pos, wpNext.time), transSamples);

                _subdivideSpline(eVector4(wpPrev.posornt.rot, wpPrev.time),
                                 eVector4(wpFrom.posornt.rot, wpFrom.time),
                                 eVector4(wpTo.posornt.rot, wpTo.time),
                                 eVector4(wpNext.posornt.rot, wpNext.time), rotSamples);

                _subdivideSpline(eVector4(wpPrev.posornt.scale, wpPrev.time),
                                 eVector4(wpFrom.posornt.scale, wpFrom.time),
                                 eVector4(wpTo.posornt.scale, wpTo.time),
                                 eVector4(wpNext.posornt.scale, wpNext.time), scaleSamples);
            }
        }
        else if (wpFrom.type == WAYPOINT_TSHADER)
        {
            // Sample path approximately every 0.2 time steps.
            // To exactly arrive on the "to" waypoint when
            // sampling, we do some extra calculations.
            const eF32 duration = wpTo.time-wpFrom.time;
            const eU32 stepCount = eTrunc(50.0f*duration/10.0f);
            const eF32 step = duration/(eF32)stepCount;

            eF32 t = wpFrom.time;
            
            for (eU32 j=0; j<=stepCount; j++, t+=step)
            {
                const PosOrnt po = _executeTShader(*wpFrom.tshader, t, callerId);

                for (eU32 i=0; i<3; i++)
                {
                    transSamples[i].append(eVector2(t, po.pos[i]));
                    rotSamples[i].append(eVector2(t, po.rot[i]));
                    scaleSamples[i].append(eVector2(t, po.scale[i]));
                }
            }
        }

        _addWaypointToSamples(wpTo, rotSamples, transSamples, scaleSamples);
    }
    */
}
#endif

const ePath::Waypoint & ePath::getWaypoint(eU32 index) const
{
    return m_waypoints[index];
}

ePath::Waypoint & ePath::getWaypoint(eU32 index)
{
    return m_waypoints[index];
}

eU32 ePath::getWaypointCount() const
{
    return m_waypoints.size();
}

eF32 ePath::getRunningTime() const
{
    return getEndTime()-getStartTime();
}

eF32 ePath::getStartTime() const
{
    if (m_waypoints.size() == 0)
    {
        return 0.0f;
    }

    return m_waypoints[0].time;
}

eF32 ePath::getEndTime() const
{
    if (m_waypoints.size() == 0)
    {
        return 0.0f;
    }

    return m_waypoints[m_waypoints.size()-1].time;
}


void ePath::_insertWaypoint(const Waypoint &wp)
{
    for (eU32 i=0; i<m_waypoints.size(); i++)
    {
        if (m_waypoints[i].time > wp.time)
        {
            m_waypoints.insert(i, wp);
            return;
        }
    }

    // Waypoint is only appended, if loop above does not
    // find a correct position to insert waypoint at.
    m_waypoints.append(wp);
}

const ePath::Waypoint * ePath::_findWaypoint(eF32 time) const
{
    const Waypoint *wp = eNULL;

    for (eU32 i=0; i<m_waypoints.size(); i++)
    {
        if (m_waypoints[i].time > time)
        {
            break;
        }

        wp = &m_waypoints[i];
    }

    return wp;
}

eVector4 ePath::_executeTShader(eTShader &ts, eF32 time, eConstPtr callerId) const
{
    eASSERT(time >= 0.0f);

    eVector4 res;

    ts.setTime(callerId, time);

    tfPlayer &tf = eDemo::getSynth();
    ts.setAudio(callerId, 0, tf.getMasterPeak());

    for (eU32 i=0;i<TF_MAX_INPUTS;i++)
        ts.setAudio(callerId, 1+i, tf.getPeakInstr(i));

    ts.run(callerId, res);

    return res;
}

#ifdef eEDITOR
void ePath::_subdivideSpline(const eVector4 &wp0, const eVector4 &wp1, const eVector4 &wp2, const eVector4 &wp3, eVector2Array samples[3]) const
{
    eASSERT(wp0.w <= wp1.w);
    eASSERT(wp1.w <= wp2.w)
    eASSERT(wp2.w <= wp3.w);

    // Geometry matrices of Bezier and Catmull-Rom splines,
    // used for conversion of Catmull-Rom waypoints to
    // Bezier waypoints which are easier to subdivide.
    static const eMatrix4x4 geoMatBezier( 1.0f,  0.0f,  0.0f, 0.0f,
                                         -3.0f,  3.0f,  0.0f, 0.0f,
                                          3.0f, -6.0f,  3.0f, 0.0f,
                                         -1.0f,  3.0f, -3.0f, 1.0f);

    static const eMatrix4x4 geoMatCatmull( 0.0f,  1.0f,  0.0f,  0.0f,
                                          -0.5f,  0.0f,  0.5f,  0.0f,
                                           1.0f, -2.5f,  2.0f, -0.5f,
                                          -0.5f,  1.5f, -1.5f,  0.5f);

    static const eMatrix4x4 geoMatBezierInv = geoMatBezier.inverse();
    static const eMatrix4x4 matCatmullToBezier = geoMatBezierInv*geoMatCatmull;

    // Perform conversion of waypoints.
    const eVector4 xc = matCatmullToBezier*eVector4(wp0.x, wp1.x, wp2.x, wp3.x);
    const eVector4 yc = matCatmullToBezier*eVector4(wp0.y, wp1.y, wp2.y, wp3.y);
    const eVector4 zc = matCatmullToBezier*eVector4(wp0.z, wp1.z, wp2.z, wp3.z);
    const eVector4 ts = matCatmullToBezier*eVector4(wp0.w, wp1.w, wp2.w, wp3.w);

    // Perform the subdivisions.
    _subdivideBezier(eVector2(ts.x, xc.x), eVector2(ts.y, xc.y), eVector2(ts.z, xc.z), eVector2(ts.w, xc.w), samples[0]);
    _subdivideBezier(eVector2(ts.x, yc.x), eVector2(ts.y, yc.y), eVector2(ts.z, yc.z), eVector2(ts.w, yc.w), samples[1]);
    _subdivideBezier(eVector2(ts.x, zc.x), eVector2(ts.y, zc.y), eVector2(ts.z, zc.z), eVector2(ts.w, zc.w), samples[2]);
}

void ePath::_subdivideBezier(const eVector2 &wp0, const eVector2 &wp1, const eVector2 &wp2, const eVector2 &wp3, eVector2Array &samples) const
{
    if (_isBezierStraight(wp0, wp1, wp2, wp3))
    {
        samples.append(wp0);
    }
    else
    {
        const eVector2 t = wp1.midpoint(wp2);

        const eVector2 &wp00 = wp0;
        const eVector2 wp01 = wp0.midpoint(wp1);
        const eVector2 wp02 = wp01.midpoint(t);
        const eVector2 &wp13 = wp3;
        const eVector2 wp12 = wp2.midpoint(wp3);
        const eVector2 wp11 = wp12.midpoint(t);
        const eVector2 wp10 = wp02.midpoint(wp11);
        const eVector2 &wp03 = wp10;

        _subdivideBezier(wp00, wp01, wp02, wp03, samples);
        _subdivideBezier(wp10, wp11, wp12, wp13, samples);
    }
}

// Returns wether or not the bezier spline built by the
// given four control points is approximately a straight
// line or not.
eBool ePath::_isBezierStraight(const eVector2 &wp0, const eVector2 &wp1, const eVector2 &wp2, const eVector2 &wp3) const
{
    static const eF32 MIN_DIST = 0.05f;
    return (wp1.distanceToLine(wp0, wp3) < MIN_DIST && wp2.distanceToLine(wp0, wp3) < MIN_DIST);
}

void ePath::_addWaypointToSamples(const Waypoint &wp, eVector2Array rotSamples[3], eVector2Array transSamples[3], eVector2Array scaleSamples[3]) const
{
    /*
    for (eU32 i=0; i<3; i++)
    {
        rotSamples[i].append(eVector2(wp.time,  wp.posornt.rot[i]));
        transSamples[i].append(eVector2(wp.time, wp.posornt.pos[i]));
        scaleSamples[i].append(eVector2(wp.time, wp.posornt.scale[i]));
    }
    */
}
#endif

// Implementation of path sampler.

void ePathSampler::sample(const ePath &path, eF32 startTime, eF32 endTime, eU32 sampleCount)
{
    eASSERT(startTime >= 0.0f);
    eASSERT(endTime >= 0.0f);
    eASSERT(startTime <= endTime);

    const eF32 step = (endTime-startTime)/(eF32)(sampleCount-1);

    m_samples.resize(sampleCount);
    m_startTime = startTime;
    m_endTime = endTime;
    m_stepInv = 1.0f/step;

    eF32 curTime = startTime;

    for (eU32 i=0; i<m_samples.size(); i++, curTime+=step)
    {
        m_samples[i] = path.process(curTime, this);
    }
}

void ePathSampler::sample(const ePath &path)
{
    const eU32 sampleCount = eFtoL(path.getRunningTime()*(eF32)SAMPLES_PER_SEC);
    sample(path, path.getStartTime(), path.getEndTime(), sampleCount);
}

const eVector4 & ePathSampler::evaluate(eF32 time) const
{
    const eU32 p = eFtoL((time-m_startTime)*m_stepInv);
    const eU32 cp = eClamp((eU32)0, p, m_samples.size()-1);

    return m_samples[cp];
}