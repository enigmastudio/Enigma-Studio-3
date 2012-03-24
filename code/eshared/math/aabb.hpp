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

#ifndef AABB_HPP
#define AABB_HPP

// Implementation of an axis aligned bounding box.
class eAABB
{
public:
    enum Collision
    {
        INSIDE,
        OUTSIDE,
        INTERSECTS
    };

public:
    static const eU32 CORNER_COUNT = 8;

public:
    eAABB()
    {
        clear();
    }

    eAABB(const eVector3 &min, const eVector3 &max)
    {
        setMinMax(min, max);
    }

    // Can't use vector for size here, because
    // then two constructors with same declaration
    // would exist.
    eAABB(const eVector3 &center, eF32 width, eF32 height, eF32 depth)
    {
        setCenterSize(center, eVector3(width, height, depth));
    }

    void clear()
    {
        m_min.set(eF32_MAX, eF32_MAX, eF32_MAX);
        m_max.set(eF32_MIN, eF32_MIN, eF32_MIN);

        m_center.null();
        m_size.null();
    }

    void setMinMax(const eVector3 &min, const eVector3 &max)
    {
        m_min    = min;
        m_max    = max;
        m_center = (min+max)*0.5f;
        m_size   = max-min;
    }

    void setCenterSize(const eVector3 &center, const eVector3 &size)
    {
        const eVector3 halfeSize = size*0.5f;

        m_size   = size;
        m_center = center;
        m_min    = center-halfeSize;
        m_max    = center+halfeSize;
    }

    Collision intersects(const eAABB &aabb) const
    {
        const eVector3 aabbMin = aabb.getMinimum();
        const eVector3 aabbMax = aabb.getMaximum();

        if (aabbMin.x >= m_min.x && aabbMax.x <= m_max.x &&
            aabbMin.y >= m_min.y && aabbMax.y <= m_max.y &&
            aabbMin.z >= m_min.z && aabbMax.z <= m_max.z)
        {
            return INSIDE;
        }

        if (m_max.x < aabbMin.x || m_min.x > aabbMax.x ||
            m_max.y < aabbMin.y || m_min.y > aabbMax.y ||
            m_max.z < aabbMin.z || m_min.z > aabbMax.z)
        {
            return OUTSIDE;
        }

        return INTERSECTS;
    }

    // Checks if one of the three vector axis
    // components are outside of bounding box.
    // If so, bounding box is resized accordingly.
    void updateExtent(const eVector3 &v)
    {
        m_min.minimum(v);
        m_max.maximum(v);

        m_center = (m_min+m_max)*0.5f;
        m_size = m_max-m_min;
    }

    void updateExtentFast(const eVector3 &v)
    {
        m_min.minimum(v);
        m_max.maximum(v);
    }


    void translate(const eVector3 &v)
    {
        m_min += v;
        m_max += v;
        m_center += v;
    }

    void scale(const eVector3 &v)
    {
        m_min.scale(v);
        m_max.scale(v);
        m_size.scale(v);
    }

    // Transformation of AABBs is a bit tricky in order
    // to compensate for rotations from model- to world-
    // space (kills axis alignment).
    void transform(const eMatrix4x4 &mtx);

    void merge(const eAABB &aabb)
    {
        m_min.minimum(aabb.getMinimum());
        m_max.maximum(aabb.getMaximum());

        m_center = (m_min+m_max)*0.5f;
        m_size = m_max-m_min;
    }

    void mergeFast(const eAABB &aabb)
    {
        m_min.minimum(aabb.getMinimum());
        m_max.maximum(aabb.getMaximum());
    }


    eBool contains(const eVector3 &pos)
    {
        for (eU32 i=0; i<3; i++)
        {
            if (pos[i] < m_min[i] || pos[i] > m_max[i])
            {
                return eFALSE;
            }
        }

        return eTRUE;
    }

    const eVector3 & getMinimum() const
    {
        return m_min;
    }

    const eVector3 & getMaximum() const
    {
        return m_max;
    }

    const eVector3 & getCenter() const
    {
        return m_center;
    }

    const eVector3 & getSize() const
    {
        return m_size;
    }

    void getCorners(eVector3 corners[CORNER_COUNT]) const
    {
        eASSERT(corners != eNULL);

        corners[0] = m_min;
        corners[1] = eVector3(m_max.x, m_min.y, m_min.z);
        corners[2] = eVector3(m_max.x, m_max.y, m_min.z);
        corners[3] = eVector3(m_min.x, m_max.y, m_min.z);
        corners[4] = eVector3(m_min.x, m_min.y, m_max.z);
        corners[5] = eVector3(m_max.x, m_min.y, m_max.z);
        corners[6] = m_max;
        corners[7] = eVector3(m_min.x, m_max.y, m_max.z);
    }

private:
    eVector3 m_min;
    eVector3 m_max;
    eVector3 m_center;
    eVector3 m_size;
};

#endif // AABB_HPP