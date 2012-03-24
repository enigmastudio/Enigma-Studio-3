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

#ifndef RAY_HPP
#define RAY_HPP

// Class encapsulating a ray (an infinite line) in 3D space.
class eRay
{
public:
    eRay(const eVector3 &origin, const eVector3 &dir) :
        m_origin(origin),
        m_dir(dir)
    {
        eASSERT(!eIsFloatZero(dir.sqrLength()));
    }

    void setOrigin(const eVector3 &origin)
    {
        m_origin = origin;
    }

    void setDirection(const eVector3 &dir)
    {
        m_dir = dir;
    }

    eVector3 evaluate(eF32 t) const
    {
        return m_origin+t*m_dir;
    }

    eBool intersects(const ePlane &p) const
    {
        return !eIsFloatZero(p.getNormal()*m_dir);
    }

    // Returns wether or not the ray intersects the
    // triangle given by the three points a, b, c.
    eBool intersects(const eVector3 &triA, const eVector3 &triB, const eVector3 &triC) const
    {
        const eVector3 e0 = triB-triA;
        const eVector3 e1 = triC-triA;
        const eVector3 e2 = m_dir^e1;

        const eF32 a = e0*e2;

        if (eIsFloatZero(a))
        {
            return eFALSE;
        }

        const eF32 f = 1.0f/a;
        const eVector3 s = m_origin-triA;
        const eF32 u = f*(s*e2);

        if (u < 0.0f || u > 1.0f)
        {
            return eFALSE;
        }

        const eVector3 q = s^e0;
        const eF32 v = f*(m_dir*q);

        if (v < 0.0f || u+v > 1.0f)
        {
            return eFALSE;
        }

        const eF32 t = f*(e1*q);
        return (t > 0.0f);
    }

    const eVector3 & getOrigin() const
    {
        return m_origin;
    }

    const eVector3 & getDirection() const
    {
        return m_dir;
    }

private:
    eVector3    m_origin;
    eVector3    m_dir;
};

#endif // RAY_HPP