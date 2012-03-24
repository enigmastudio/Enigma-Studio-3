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

#ifndef VECTOR3_HPP
#define VECTOR3_HPP

//#define eUSE_SSE_EVECTOR3

#ifdef eUSE_SSE
#include <xmmintrin.h>
#endif

class ePlane;

class eFXYZ
{
public:

    eF32    x;
    eF32    y;
    eF32    z;

    void set(eF32 nx, eF32 ny, eF32 nz)
    {
        x = nx;
        y = ny;
        z = nz;
    }
};

// Three-dimensional vector class.
class eVector3 : public eFXYZ
{
public:
    eVector3()
    {
        null();
    }

    eVector3(const eFXYZ &fxyz)
    {
        set(fxyz.x, fxyz.y, fxyz.z);
		w = 0;
    }

    eVector3(const eIXYZ &ixyz)
    {
	    set((eF32)ixyz.x, (eF32)ixyz.y, (eF32)ixyz.z);
		w = 0;
    }

    // Sets x, y and z to value.
    eVector3(eF32 value)
    {
        set(value, value, value);
		w = 0;
    }

    eVector3(eF32 nx, eF32 ny, eF32 nz)
    {
        set(nx, ny, nz);
		w = 0;
    }

    eVector3(const eVector2 &xy, eF32 nz)
    {
        set(xy.x, xy.y, nz);
		w = 0;
    }

    eVector3(eF32 nx, const eVector2 &yz)
    {
        set(nx, yz.x, yz.y);
		w = 0;
    }

    void null()
    {
        set(0.0f, 0.0f, 0.0f);
		w = 0;
    }

    void negate()
    {
        set(-x, -y, -z);
    }

    eF32 length() const
    {
        return eSqrt(sqrLength());
    }

    eF32 sqrLength() const
    {
        return x*x+y*y+z*z;
    }

    eF32 distance(const eVector3 &v) const
    {
        return ((*this)-v).length();
    }

    eF32 distance(const eVector3 &support, const eVector3 &dir) const
    {
        return ((*this-support)^dir).length()/dir.length();
    }

    void normalize()
    {
        // Prevent danger of division by 0.
        if (eIsFloatZero(length()))
        {
            return;
        }

        const eF32 invLen = 1.0f/length();
        x *= invLen;
        y *= invLen;
        z *= invLen;
    }

    eVector3 normalized() const
    {
        eVector3 n = *this;

        n.normalize();
        return n;
    }

    eVector3 random() const
    {
		const eF32 newz = z*eRandomF(-1.0f, 1.0f);
		const eF32 newy = y*eRandomF(-1.0f, 1.0f);
		const eF32 newx = x*eRandomF(-1.0f, 1.0f);
		
        return eVector3(newx, newy, newz);
    }

	eVector3 random(eU32 &seed) const
	{
		const eF32 newz = z*eRandomF(-1.0f, 1.0f, seed);
		const eF32 newy = y*eRandomF(-1.0f, 1.0f, seed);
		const eF32 newx = x*eRandomF(-1.0f, 1.0f, seed);

		return eVector3(newx, newy, newz);
	}

    void absolute()
    {
        x = eAbs(x);
        y = eAbs(y);
        z = eAbs(z);
    }

    void minimum(const eVector3 &v)
    {
#if defined(eUSE_SSE) && defined(eUSE_SSE_EVECTOR3)
		this->w = 0;
	    const __m128 v0 = _mm_loadu_ps(&x);
	    const __m128 v1 = _mm_loadu_ps(&v.x);
		const __m128 vmin = _mm_min_ps(v0,v1);
		_mm_storeu_ps(&x, vmin);
#else
        x = eMin(x, v.x);
        y = eMin(y, v.y);
        z = eMin(z, v.z);
#endif
    }

    void maximum(const eVector3 &v)
    {
#if defined(eUSE_SSE) && defined(eUSE_SSE_EVECTOR3)
		this->w = 0;
	    const __m128 v0 = _mm_loadu_ps(&x);
	    const __m128 v1 = _mm_loadu_ps(&v.x);
		const __m128 vmax = _mm_max_ps(v0,v1);
		_mm_storeu_ps(&x, vmax);
#else
        x = eMax(x, v.x);
        y = eMax(y, v.y);
        z = eMax(z, v.z);
#endif
    }

    void clamp(eF32 min, eF32 max)
    {
        x = eClamp(min, x, max);
        y = eClamp(min, y, max);
        z = eClamp(min, z, max);
    }

    void scale(const eVector3 &v)
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;
    }

    // The angles are expected to be in radians.
    void rotate(const eVector3 &v)
    {
        eF32 sx, cx, sy, cy, sz, cz;

        eSinCos(v.x, sx, cx);
        eSinCos(v.y, sy, cy);
        eSinCos(v.z, sz, cz);

        // Rotate around x-axis.
        eF32 temp = y*cx-z*sx;
        z = y*sx+z*cx;
        y = temp;

        // Rotate around y-axis.
        temp = z*sy+x*cy;
        z = z*cy-x*sy;
        x = temp;

        // Rotate around z-axis.
        temp = x*cz-y*sz;
        y = x*sz+y*cz;
        x = temp;
    }

    // Rotates this vector around the given origin.
    // The angles are expected to be in radians.
    void rotate(const eVector3 &origin, const eVector3 &v)
    {
        *this -= origin;
        rotate(v);
        *this += origin;
    }

    void translate(const eVector3 &v)
    {
        *this += v;
    }

    eVector3 midpoint(const eVector3 &v) const
    {
        return (*this+v)*0.5f;
    }

    // Linear interpolation (0 <= t <= 1).
    eVector3 lerp(eF32 t, const eVector3 &to) const
    {
        eASSERT(t >= 0.0f && t <= 1.0f);
        return eVector3(*this+(to-*this)*t);
    }

    eVector3 operator + (const eVector3 &v) const
    {
#if defined(eUSE_SSE) && defined(eUSE_SSE_EVECTOR3)
		eF32 result[4];
		const __m128 v0 = _mm_loadu_ps(&x);
	    const __m128 v1 = _mm_loadu_ps(&v.x);
		const __m128 vr = _mm_add_ps(v0,v1);
		_mm_storeu_ps(&result[0], vr);
		return (eVector3&)result;
#else

        return eVector3(x+v.x, y+v.y, z+v.z);
#endif
    }

    eVector3 operator - (const eVector3 &v) const
    {
#if defined(eUSE_SSE) && defined(eUSE_SSE_EVECTOR3)
		eF32 result[4];
		const __m128 v0 = _mm_loadu_ps(&x);
	    const __m128 v1 = _mm_loadu_ps(&v.x);
		const __m128 vr = _mm_sub_ps(v0,v1);
		_mm_storeu_ps(&result[0], vr);
		return (eVector3&)result;
#else
       return eVector3(x-v.x, y-v.y, z-v.z);
#endif
    }

    // Returns dot product.
    eF32 operator * (const eVector3 &v) const
    {
        return x*v.x+y*v.y+z*v.z;
    }

    // Scalar multiplication (scale).
    eVector3 operator * (eF32 s) const
    {
        return eVector3(x*s, y*s, z*s);
    }

    friend eVector3 operator * (eF32 s, const eVector3 &v)
    {
        return v*s;
    }

    friend eVector3 operator / (eF32 s, const eVector3 &v)
    {
        return eVector3(s/v.x,s/v.y,s/v.z);
    }

    // Returns vector multiplied with matrix.
    eVector3 operator * (const eMatrix4x4 &m) const
    {
#if defined(eUSE_SSE) && defined(eUSE_SSE_EVECTOR3)
		eF32 result[4];
		const __m128 v0 = _mm_loadu_ps(&x);
		const __m128 x0 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(0, 0, 0, 0));
		const __m128 y0 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(1, 1, 1, 1));
		const __m128 z0 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(2, 2, 2, 2));
		const __m128 mat0 = _mm_loadu_ps(&m.m11);
		const __m128 mat1 = _mm_loadu_ps(&m.m21);
		const __m128 mat2 = _mm_loadu_ps(&m.m31);
		const __m128 mat3 = _mm_loadu_ps(&m.m41);
		
		const __m128 a1 = _mm_mul_ps(x0, mat0);
		const __m128 a2 = _mm_mul_ps(y0, mat1);
		const __m128 a3 = _mm_mul_ps(z0, mat2);
		const __m128 sum1 = _mm_add_ps(a1, a2);
		const __m128 sum2 = _mm_add_ps(a3, mat3);
		const __m128 sum = _mm_add_ps(sum1, sum2);
		
		_mm_storeu_ps(&result[0], sum);
		return (eVector3&)result;
#else
        return eVector3(x*m.m11+y*m.m21+z*m.m31+m.m41,
                        x*m.m12+y*m.m22+z*m.m32+m.m42,
                        x*m.m13+y*m.m23+z*m.m33+m.m43);
#endif
    }

    eVector3 operator / (eF32 s) const
    {
        eASSERT(!eIsFloatZero(s));
        return *this*(1.0f/s);
    }

    // Returns cross product.
    eVector3 operator ^ (const eVector3 &v) const
    {
        return eVector3(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x);
    }

    eVector3 & operator += (const eVector3 &v)
    {
#if defined(eUSE_SSE) && defined(eUSE_SSE_EVECTOR3)
		this->w = 0;
		const __m128 v0 = _mm_loadu_ps((float*)this);
	    const __m128 v1 = _mm_loadu_ps((float*)&v);
		const __m128 vr = _mm_add_ps(v0,v1);
		_mm_storeu_ps((float*)this, vr);
#else
        *this = *this+v;
#endif
        return *this;
    }

    eVector3 & operator -= (const eVector3 &v)
    {
#if defined(eUSE_SSE) && defined(eUSE_SSE_EVECTOR3)
		this->w = 0;
		const __m128 v0 = _mm_loadu_ps(&x);
	    const __m128 v1 = _mm_loadu_ps(&v.x);
		const __m128 vr = _mm_sub_ps(v0,v1);
		_mm_storeu_ps(&x, vr);
#else
        *this = *this-v;
#endif
        return *this;
    }

    eVector3 & operator *= (eF32 s)
    {
        *this = *this*s;
        return *this;
    }

    eVector3 & operator *= (const eVector3 &v)
    {
#if defined(eUSE_SSE) && defined(eUSE_SSE_EVECTOR3)
		this->w = 0;
		const __m128 v0 = _mm_loadu_ps(&x);
	    const __m128 v1 = _mm_loadu_ps(&v.x);
		const __m128 vr = _mm_mul_ps(v0,v1);
		_mm_storeu_ps(&x, vr);
#else
		this->x *= v.x;
		this->y *= v.y;
		this->z *= v.z;
#endif
        return *this;
    }

    eVector3 & operator *= (const eMatrix4x4 &m)
    {
        *this = *this*m;
        return *this;
    }

    eVector3 & operator /= (eF32 s)
    {
        *this = *this/s;
        return *this;
    }

    eVector3 operator - () const
    {
        return eVector3(-x, -y, -z);
    }

    eBool operator == (const eVector3& other) const
    {
		return eAreFloatsEqual(x, other.x) && eAreFloatsEqual(y, other.y) &&  eAreFloatsEqual(z, other.z); 
    }

    operator const eF32 * () const
    {
        return (eF32 *)this;
    }

    const eF32 & operator [] (eInt index) const
    {
        eASSERT(index < 3);
        return ((eF32 *)this)[index];
    }

    eF32 & operator [] (eInt index)
    {
        eASSERT(index < 3);
        return ((eF32 *)this)[index];
    }

public:
    eBool           isInsideCube(const ePlane cubePlanes[6]) const;
    eVector3        distanceToTriangle(const eVector3& base, const eVector3& e0, const eVector3& e1);
    eVector3        distanceToTriangleOptimized(const eVector3& base, const eVector3& e0, const eVector3& e1, eF32 e0_DOT_e0, eF32 e0_DOT_e1, eF32 e1_DOT_e1);

public:
	static	void    cubicBezier(eF32 t, const eVector3 &cp0, const eVector3 &cp1, const eVector3 &cp2, const eVector3 &cp3, eVector3 &resPos, eVector3 &resTangent);
    static eVector3 catmullRom(eF32 t, const eVector3 &v0, const eVector3 &v1, const eVector3 &v2, const eVector3 &v3);

public:
    static const eVector3 XAXIS;
    static const eVector3 YAXIS;
    static const eVector3 ZAXIS;
    static const eVector3 ORIGIN;

    eF32 w;
private:
};

typedef eArray<eVector3> eVector3Array;

#endif // VECTOR3_HPP
