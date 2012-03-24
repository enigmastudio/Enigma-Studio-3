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

#ifndef QUAT_HPP
#define QUAT_HPP

#ifndef eHWSYNTH

class eQuat
{
public:
    eQuat() : x(0.0f), y(0.0f), z(0.0f), w(1.0f)
    {
    }

    eQuat(eF32 value) : x(value), y(value), z(value), w(value)
    {
    }

    eQuat(const eMatrix4x4 &m)
    {
        fromMatrix(m);
    }

    eQuat(eF32 nx, eF32 ny, eF32 nz, eF32 nw) : x(nx), y(ny), z(nz), w(nw)
    {
    }

    eQuat(const eVector3 &axis, eF32 angle)
    {
        fromAxisAngle(axis, angle);
    }

    // axis is assumed to be 0zyx
    eFORCEINLINE eQuat(const __m128& axis, eF32 angle)
    {
//        eASSERT(eAreFloatsEqual(axis.length(), 1.0f));
        eF32 sa, ca;
        eSinCos(angle*0.5f, sa, ca);

        __m128 vs = _mm_mul_ps(axis, _mm_set1_ps(sa));
        vs.m128_f32[3] = ca;

        // normalize
        __m128 mdot = _mm_mul_ps(vs, vs);
        __m128 mdotagg = _mm_hadd_ps(mdot, mdot);
        __m128 recipsqrt = _mm_rsqrt_ss( _mm_hadd_ps(mdotagg, mdotagg) );
        __m128 vsnorm = _mm_mul_ps(vs, _mm_shuffle_ps(recipsqrt, recipsqrt, _MM_SHUFFLE(0,0,0,0)));
        _mm_storeu_ps(&this->x, vsnorm);
    }


	// Shortest Arc rotation between axis0 and axis1
    eQuat(const eVector3 &axis0, const eVector3 &axis1)
    {
		(eFXYZ&)this->x = axis0^axis1;
		this->w = axis0*axis1;
		this->normalize();
    }


    void set(eF32 nx, eF32 ny, eF32 nz, eF32 nw)
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

    void negate()
    {
        set(-x, -y, -z, -w);
    }

    void conjugate()
    {
        set(-x, -y, -z, w);
    }

    eF32 length() const
    {
        return eSqrt(sqrLength());
    }

    eF32 sqrLength() const
    {
        return ((eVector3&)x).sqrLength()+w*w;
    }

    void normalize()
    {
        // Prevent danger of division by 0.
        eASSERT(!eIsFloatZero(length()));

        const eF32 invLen = 1.0f/length();
        x *= invLen;
        y *= invLen;
        z *= invLen;
        w *= invLen;
    }

    eQuat normalized() const
    {
        eQuat q = *this;

        q.normalize();
        return q;
    }

    // Operator * already overloaded with
    // concatenation of two quaternions.
    eF32 dot(const eQuat &q) const
    {
        eQuat t = q;
        t.conjugate();
        return (eVector3&)x*(eVector3&)t.x;
    }

    // Returns the inverse (1/q=q^(-1)).
    eQuat inverse() const
    {
        eQuat q = *this;
        q.conjugate();
        return q/sqrLength();
    }

    eQuat operator + (const eQuat &q) const
    {
        return eQuat(x+q.x, y+q.y, z+q.z, w+q.w);
    }

    eQuat operator - (const eQuat &q) const
    {
        return eQuat(x-q.x, y-q.y, z-q.z, w-q.w);
    }

    // Default quaternion multiplication
    // (not commutative), defined as:
    // (s,v)*(t,w)=(s*t-<v,w>,sw+tv+(v x w))
    eQuat operator * (const eQuat &q) const;

    // Cross product
    eQuat operator ^ (const eQuat &q) const
    {
		return ((*this) * q - q * (*this)) * 0.5f;
    }


    eQuat operator - () const
    {
        return eQuat(-x, -y, -z, -w);
    }

    // Scalar multiplication (scale).
    eQuat operator * (eF32 s) const
    {
        return eQuat(x*s, y*s, z*s, w*s);
    }

    eQuat operator / (eF32 s) const
    {
        eASSERT(!eIsFloatZero(s));
        return *this*(1.0f/s);
    }

    eQuat operator / (const eQuat &q) const
    {
        return *this*q.inverse();
    }

    friend eQuat operator * (eF32 s, const eQuat &q)
    {
        return q*s;
    }

    eQuat & operator += (const eQuat &q)
    {
        *this = *this+q;
        return *this;
    }

    eQuat & operator -= (const eQuat &q)
    {
        *this = *this-q;
        return *this;
    }

    eQuat & operator *= (const eQuat &q)
    {
        *this = *this*q;
        return *this;
    }

    eQuat & operator *= (eF32 s)
    {
        *this = *this*s;
        return *this;
    }

    eQuat & operator /= (eF32 s)
    {
        *this = *this/s;
        return *this;
    }

private:
    operator const eF32 * () const
    {
        return (eF32 *)this;
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
    // Bigger, not inlineable functions.
    eVector3        toEuler() const;
    void            fromRotation(const eVector3 &v);
    void            fromMatrix(const eMatrix4x4 &m);
    void            fromAxisAngle(const eVector3 &axis, eF32 angle);
    eQuat           log() const;
    eQuat           exp() const;
    eQuat           lerp(eF32 t, const eQuat &to) const;
    eQuat           slerp(eF32 t, const eQuat &to) const;
	eVector3		getVector(eU32 nr) const;

public:
    static eQuat    squad(eF32 t, const eQuat &prev, const eQuat &from, const eQuat &to, const eQuat &next);
    static eQuat    getInterpolationTangent(const eQuat &prev, const eQuat &cur, const eQuat &next);

//private:
    union
    {
        struct
        {
            eF32        x;
            eF32        y;
            eF32        z;
            eF32        w;
        };
    };  
};

#endif

#endif // QUAT_HPP
