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

#ifndef PLANE_HPP
#define PLANE_HPP

// Plane which is defined in 3D space by the
// equation Ax + By + Cz + D = 0.
class ePlane
{
public:
    // The front side of the plane is the half
    // space the normal is pointing to. The back
    // side is the other one. The side on plane
    // indicates the plane it-self.
    enum Side
    {
        SIDE_FRONT,
        SIDE_BACK,
        SIDE_ONPLANE
    };

public:
    ePlane() : m_d(0.0f)
    {
    }

    ePlane(const eVector3 &v0, const eVector3 &v1, const eVector3 &v2) :
        m_normal((v2-v1)^(v0-v1))
    {
        m_normal.normalize();
        m_absNormal = m_normal;
        m_absNormal.absolute();
        m_d = -m_normal*v0;
    }

    ePlane(const eVector3 &normal, eF32 d) :
        m_normal(normal),
        m_d(d)
    {
        m_normal.normalize();
        m_absNormal = m_normal;
        m_absNormal.absolute();
    }

    const eVector3 & getNormal() const
    {
        return m_normal;
    }

    const eVector3 & getAbsNormal() const
    {
        return m_absNormal;
    }

    eF32 getCoeffD() const
    {
        return m_d;
    }

    Side getSide(const eVector3 &v) const
    {
        const eF32 dist = getDistance(v);

        if (dist > eALMOST_ZERO)
        {
            return SIDE_FRONT;
        }
        else if (dist < -eALMOST_ZERO)
        {
            return SIDE_BACK;
        }

        return SIDE_ONPLANE;
    }

    eF32 getDistance(const eVector3 &v) const
    {
        return m_normal*v+m_d;
    }

    void transform(const eMatrix4x4 &m)
    {
        // This is how normal vectors have to be transformed.
        eMatrix4x4 ms = m;
        ms.invert();
        ms.transpose();

        const eVector4 r = eVector4(m_normal, m_d)*ms;
        m_normal.set(r.x, r.y, r.z);
        m_absNormal = m_normal;
        m_absNormal.absolute();
        m_d = r.w;
    }

private:
    eVector3    m_normal;
    eVector3    m_absNormal;
    eF32        m_d;
};

#endif // PLANE_HPP