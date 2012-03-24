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

#ifndef VECTOR4_HPP
#define VECTOR4_HPP

class eFXYZW
{
public:
    union
    {
        struct
        {
            eF32    x;
            eF32    y;
            eF32    z;
            eF32    w;
        };

        struct
        {
            eF32    r;
            eF32    g;
            eF32    b;
            eF32    a;
        };
    };
};

// Four-dimensional vector class.
class eVector4 : public eFXYZW
{
public:
    eVector4()
    {
        set(0.0f, 0.0f, 0.0f, 1.0f);
    }

    eVector4(const eFXYZW &fxyzw)
    {
        set(fxyzw.x, fxyzw.y, fxyzw.z, fxyzw.w);
    }

    // Sets x, y, z and w to value.
    eVector4(eF32 value)
    {
        set(value, value, value, value);
    }

    eVector4(eF32 nx, eF32 ny, eF32 nz, eF32 nw=1.0f)
    {
        set(nx, ny, nz, nw);
    }

    eVector4(const eVector2 &v0, const eVector2 &v1)
    {
        set(v0.x, v0.y, v1.x, v1.y);
    }

    eVector4(const eVector3 &v, eF32 w)
    {
        set(v.x, v.y, v.z, w);
    }

    void set(eF32 nx, eF32 ny, eF32 nz, eF32 nw=1.0f)
    {
        x = nx;
        y = ny;
        z = nz;
        w = nw;
    }

    void null()
    {
        set(0.0f, 0.0f, 0.0f, 0.0f);
    }

    eVector3 toVec3()
    {
        return eVector3(x, y, z);
    }

    void negate()
    {
        set(-x, -y, -z, -w);
    }

    eF32 length() const
    {
        return eSqrt(sqrLength());
    }

    eF32 sqrLength() const
    {
        return x*x+y*y+z*z+w*w;
    }

    eF32 distance(const eVector4 &v) const
    {
        return ((*this)-v).length();
    }

    void normalize()
    {
        // Prevent danger of division by 0.
        if(eIsFloatZero(length()))
        {
            return;
        }

        const eF32 invLen = 1.0f/length();
        x *= invLen;
        y *= invLen;
        z *= invLen;
        w *= invLen;
    }

    eVector4 normalized() const
    {
        eVector4 n = *this;

        n.normalize();
        return n;
    }

    eVector4 random() const
    {
        return eVector4(x*eRandomF(-1.0f, 1.0f),
                        y*eRandomF(-1.0f, 1.0f),
                        z*eRandomF(-1.0f, 1.0f),
                        w*eRandomF(-1.0f, 1.0f));
    }

    void absolute()
    {
        x = eAbs(x);
        y = eAbs(y);
        z = eAbs(z);
        w = eAbs(w);
    }

    void minimum(const eVector4 &v)
    {
        x = eMin(x, v.x);
        y = eMin(y, v.y);
        z = eMin(z, v.z);
        w = eMin(w, v.w);
    }

    void maximum(const eVector4 &v)
    {
        x = eMax(x, v.x);
        y = eMax(y, v.y);
        z = eMax(y, v.z);
        w = eMax(y, v.w);
    }

    void clamp(eF32 min, eF32 max)
    {
        x = eClamp(min, x, max);
        y = eClamp(min, y, max);
        z = eClamp(min, z, max);
        w = eClamp(min, w, max);
    }

    void scale(const eVector4 &v)
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        w *= v.w;
    }

    void translate(const eVector4 &v)
    {
        *this += v;
    }

    eVector4 midpoint(const eVector4 &v) const
    {
        return (*this+v)*0.5f;
    }

    // Linear interpolation (0 <= t <= 1).
    eVector4 lerp(const eVector4 &to, eF32 t) const
    {
        eASSERT(t >= 0.0f && t <= 1.0f);
        return eVector4(*this+(to-*this)*t);
    }

    eVector4 operator + (const eVector4 &v) const
    {
        return eVector4(x+v.x, y+v.y, z+v.z, w+v.w);
    }

    eVector4 operator - (const eVector4 &v) const
    {
        return eVector4(x-v.x, y-v.y, z-v.z, w-v.w);
    }

    // Returns dot product.
    eF32 operator * (const eVector4 &v) const
    {
        return x*v.x+y*v.y+z*v.z+w*v.w;
    }

    eVector4 operator ^ (const eVector4 &v) const
    {
        return eVector4(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x, 1.0f);
    }

    // Scalar multiplication (scale).
    eVector4 operator * (eF32 s) const
    {
        return eVector4(x*s, y*s, z*s, w*s);
    }

    friend eVector4 operator * (eF32 s, const eVector4 &v)
    {
        return v*s;
    }

    friend eVector4 operator / (eF32 s, const eVector4 &v)
    {
        return eVector4(s/v.x,s/v.y,s/v.z, s/v.w);
    }

    // Returns vector multiplied with matrix.
    eVector4 operator * (const eMatrix4x4 &m) const
    {
        return eVector4((x*m.m11)+(y*m.m21)+(z*m.m31)+(w*m.m41),
                        (x*m.m12)+(y*m.m22)+(z*m.m32)+(w*m.m42),
                        (x*m.m13)+(y*m.m23)+(z*m.m33)+(w*m.m43),
                        (x*m.m14)+(y*m.m24)+(z*m.m34)+(w*m.m44));
    }

    eVector4 operator / (eF32 s) const
    {
        eASSERT(!eIsFloatZero(s));
        return *this*(1.0f/s);
    }

    eVector4 & operator += (const eVector4 &v)
    {
        *this = *this+v;
        return *this;
    }

    eVector4 & operator -= (const eVector4 &v)
    {
        *this = *this-v;
        return *this;
    }

    eVector4 & operator *= (eF32 s)
    {
        *this = *this*s;
        return *this;
    }

    eVector4 & operator *= (const eMatrix4x4 &m)
    {
        *this = *this*m;
        return *this;
    }

    eVector4 & operator /= (eF32 s)
    {
        *this = *this/s;
        return *this;
    }

    eVector4 operator - () const
    {
        return eVector4(-x, -y, -z, -w);
    }

    operator const eF32 * () const
    {
        return (eF32 *)this;
    }

    eBool operator != (const eVector4 &v) const
    {
        return (!eAreFloatsEqual(x, v.x) ||
                !eAreFloatsEqual(y, v.y) ||
                !eAreFloatsEqual(z, v.z) || 
                !eAreFloatsEqual(w, v.w));
    }

    eBool operator == (const eVector4 &v) const
    {
        return !(*this != v);
    }

    const eF32 & operator [] (eInt index) const
    {
        eASSERT(index < 4);
        return ((eF32 *)this)[index];
    }

    eF32 & operator [] (eInt index)
    {
        eASSERT(index < 4);
        return ((eF32 *)this)[index];
    }

public:
    static eVector4    catmullRom(eF32 t, const eVector4 &v0, const eVector4 &v1, const eVector4 &v2, const eVector4 &v3);

};

#endif // VECTOR4_HPP