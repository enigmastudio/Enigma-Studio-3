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

#ifndef POINT_HPP
#define POINT_HPP

struct eIXYZ
{
public:
    eInt            x;
    eInt            y;
    eInt            z;

    const eInt & operator [] (const eInt index) const {
        return ((const eInt *)this)[index];
    }
};

class eIXY
{
public:
    union
    {
        struct
        {
            eInt    x;
            eInt    y;
        };

        struct
        {
            eInt    width;
            eInt    height;
        };
    };
};

// Class encapsulates an integral point.
class ePoint : public eIXY
{
public:
    ePoint()
    {
        x = 0;
        y = 0;
    }

    ePoint(const eIXY &ixy)
    {
        set(ixy.x, ixy.y);
    }

    ePoint(eInt nx, eInt ny)
    {
        set(nx, ny);
    }

    void set(eInt nx, eInt ny)
    {
        x = nx;
        y = ny;
    }

    // Returns distance between two points.
    eF32 distance(const ePoint &p) const
    {
        const eInt a = x-p.x;
        const eInt b = y-p.y;

        return eSqrt((eF32)(a*a+b*b));
    }

    ePoint operator + (const ePoint &p) const
    {
        return ePoint(x+p.x, y+p.y);
    }

    ePoint operator - (const ePoint &p) const
    {
        return ePoint(x-p.x, y-p.y);
    }

    // Scalar multiplication (scale).
    ePoint operator * (eF32 s) const
    {
        return ePoint(eFtoL(x*s), eFtoL(y*s));
    }

    friend ePoint operator * (eF32 s, const ePoint &p)
    {
        return p*s;
    }

    ePoint & operator += (const ePoint &p)
    {
        *this = *this+p;
        return *this;
    }

    ePoint & operator -= (const ePoint &p)
    {
        *this = *this-p;
        return *this;
    }

    ePoint & operator *= (eF32 s)
    {
        *this = *this*s;
        return *this;
    }

    eInt operator [] (eInt index) const
    {
        eASSERT(index < 2);
        return ((eInt *)this)[index];
    }

    eInt & operator [] (eInt index)
    {
        eASSERT(index < 2);
        return ((eInt *)this)[index];
    }

    eBool operator == (const ePoint &p) const
    {
        return (p.x == x && p.y == y);
    }

    eBool operator != (const ePoint &p) const
    {
        return !(*this == p);
    }
};

typedef ePoint eSize;
typedef eArray<ePoint> ePointArray;
typedef eArray<eSize> eSizeArray;

#endif // POINT_HPP