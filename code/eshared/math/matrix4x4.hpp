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

#ifndef MATRIX4X4_HPP
#define MATRIX4X4_HPP

class eQuat;
class eVector3;
class eVector4;

class eMatrix4x4
{
public:
    eMatrix4x4()
    {
        identity();
    }

    eMatrix4x4(const eF32 v[16]) :
        m11(v[ 0]), m12(v[ 1]), m13(v[ 2]), m14(v[ 3]),
        m21(v[ 4]), m22(v[ 5]), m23(v[ 6]), m24(v[ 7]),
        m31(v[ 8]), m32(v[ 9]), m33(v[10]), m34(v[11]),
        m41(v[12]), m42(v[13]), m43(v[14]), m44(v[15])
    {
        eASSERT(v != eNULL);
    }

    eMatrix4x4(eF32 n11, eF32 n12, eF32 n13, eF32 n14,
               eF32 n21, eF32 n22, eF32 n23, eF32 n24,
               eF32 n31, eF32 n32, eF32 n33, eF32 n34,
               eF32 n41, eF32 n42, eF32 n43, eF32 n44) :
        m11(n11), m12(n12), m13(n13), m14(n14),
        m21(n21), m22(n22), m23(n23), m24(n24),
        m31(n31), m32(n32), m33(n33), m34(n34),
        m41(n41), m42(n42), m43(n43), m44(n44)
    {
    }

    eMatrix4x4(eF32 _m44, const eVector3& axis0, const eVector3& axis1, const eVector3& axis2) :
		m11(((eF32*)&axis0)[0]),	m12(((eF32*)&axis1)[0]), m13(((eF32*)&axis2)[0]), m14(0),
        m21(((eF32*)&axis0)[1]),	m22(((eF32*)&axis1)[1]), m23(((eF32*)&axis2)[1]), m24(0),
        m31(((eF32*)&axis0)[2]),	m32(((eF32*)&axis1)[2]), m33(((eF32*)&axis2)[2]), m34(0),
        m41(0),						m42(0),					 m43(0),				  m44(_m44)
    {
    }

    eMatrix4x4(const eQuat &q)
    {
        fromQuat(q);
    }

    eMatrix4x4(const eVector3 &rot, const eVector3 &trans, const eVector3 &scaling, eBool reverse=eFALSE)
    {
        transformation(rot, trans, scaling, reverse);
    }

    void null()
    {
        m11 = 0.0f;
        m12 = 0.0f;
        m13 = 0.0f;
        m14 = 0.0f;

        m21 = 0.0f;
        m22 = 0.0f;
        m23 = 0.0f;
        m24 = 0.0f;

        m31 = 0.0f;
        m32 = 0.0f;
        m33 = 0.0f;
        m34 = 0.0f;

        m41 = 0.0f;
        m42 = 0.0f;
        m43 = 0.0f;
        m44 = 0.0f;
    }

    void identity()
    {
        null();

        m11 = 1.0f;
        m22 = 1.0f;
        m33 = 1.0f;
        m44 = 1.0f;
    }

    eF32 trace() const
    {
        return m11+m22+m33+m44;
    }

    eMatrix4x4 inverse() const
    {
        eMatrix4x4 m = *this;
        m.invert();
        return m;
    }

    void transpose()
    {
        eSwap(m12, m21);
        eSwap(m13, m31);
        eSwap(m14, m41);
        eSwap(m23, m32);
        eSwap(m24, m42);
        eSwap(m34, m43);
    }

    // Scalar multiplication (scale).
    eMatrix4x4 operator * (eF32 s) const
    {
        return eMatrix4x4(m11*s, m12*s, m13*s, m14*s,
                          m21*s, m22*s, m23*s, m24*s,
                          m31*s, m32*s, m33*s, m34*s,
                          m41*s, m42*s, m43*s, m44*s);
    }

    eMatrix4x4 operator / (eF32 s) const
    {
        eASSERT(!eIsFloatZero(s));
        return *this*(1.0f/s);
    }

    friend eMatrix4x4 operator * (eF32 s, const eMatrix4x4 &m)
    {
        return m*s;
    }

    eMatrix4x4 & operator += (const eMatrix4x4 &m)
    {
        *this = *this+m;
        return *this;
    }

    eMatrix4x4 & operator -= (const eMatrix4x4 &m)
    {
        *this = *this-m;
        return *this;
    }

    // inline matrix multiplication is much faster than function calling,
    // we should only move it if we need size
    eMatrix4x4 & operator *= (const eMatrix4x4 &mtx)
    {
#ifdef eUSE_SSE

	    const __m128 in10 = _mm_loadu_ps(&mtx.m11);
	    const __m128 in11 = _mm_loadu_ps(&mtx.m21);
	    const __m128 in12 = _mm_loadu_ps(&mtx.m31);
	    const __m128 in13 = _mm_loadu_ps(&mtx.m41);

	    for (eU32 i=0; i<16; i+=4)
        {
		    const __m128 in2 = _mm_loadu_ps(&m[i]);

		    const __m128 e0 = _mm_shuffle_ps(in2, in2, _MM_SHUFFLE(0, 0, 0, 0));
		    const __m128 e1 = _mm_shuffle_ps(in2, in2, _MM_SHUFFLE(1, 1, 1, 1));
		    const __m128 e2 = _mm_shuffle_ps(in2, in2, _MM_SHUFFLE(2, 2, 2, 2));
		    const __m128 e3 = _mm_shuffle_ps(in2, in2, _MM_SHUFFLE(3, 3, 3, 3));

		    const __m128 m0 = _mm_mul_ps(in10, e0);
		    const __m128 m1 = _mm_mul_ps(in11, e1);
		    const __m128 m2 = _mm_mul_ps(in12, e2);
		    const __m128 m3 = _mm_mul_ps(in13, e3);

		    const __m128 a0 = _mm_add_ps(m0, m1);
		    const __m128 a1 = _mm_add_ps(m2, m3);
		    const __m128 a2 = _mm_add_ps(a0, a1);

		    _mm_storeu_ps(&this->m[i], a2);
	    }
#else
        *this = eMatrix4x4(m11*mtx.m11+m12*mtx.m21+m13*mtx.m31+m14*mtx.m41,
                          m11*mtx.m12+m12*mtx.m22+m13*mtx.m32+m14*mtx.m42,
                          m11*mtx.m13+m12*mtx.m23+m13*mtx.m33+m14*mtx.m43,
                          m11*mtx.m14+m12*mtx.m24+m13*mtx.m34+m14*mtx.m44,
                          m21*mtx.m11+m22*mtx.m21+m23*mtx.m31+m24*mtx.m41,
                          m21*mtx.m12+m22*mtx.m22+m23*mtx.m32+m24*mtx.m42,
                          m21*mtx.m13+m22*mtx.m23+m23*mtx.m33+m24*mtx.m43,
                          m21*mtx.m14+m22*mtx.m24+m23*mtx.m34+m24*mtx.m44,
                          m31*mtx.m11+m32*mtx.m21+m33*mtx.m31+m34*mtx.m41,
                          m31*mtx.m12+m32*mtx.m22+m33*mtx.m32+m34*mtx.m42,
                          m31*mtx.m13+m32*mtx.m23+m33*mtx.m33+m34*mtx.m43,
                          m31*mtx.m14+m32*mtx.m24+m33*mtx.m34+m34*mtx.m44,
                          m41*mtx.m11+m42*mtx.m21+m43*mtx.m31+m44*mtx.m41,
                          m41*mtx.m12+m42*mtx.m22+m43*mtx.m32+m44*mtx.m42,
                          m41*mtx.m13+m42*mtx.m23+m43*mtx.m33+m44*mtx.m43,
                          m41*mtx.m14+m42*mtx.m24+m43*mtx.m34+m44*mtx.m44);
#endif
        return *this;
    }

    eMatrix4x4 & operator *= (eF32 s)
    {
        *this = *this*s;
        return *this;
    }

    eMatrix4x4 & operator /= (eF32 s)
    {
        *this = *this/s;
        return *this;
    }

    eMatrix4x4 operator - () const
    {
        return eMatrix4x4(-m11, -m12, -m13, -m14,
                          -m21, -m22, -m23, -m24,
                          -m31, -m32, -m33, -m34,
                          -m41, -m42, -m43, -m44);
    }

    operator const eF32 * () const
    {
        return (eF32 *)this;
    }

    // Returns element with given index.
    eF32 & operator [] (eInt index)
    {
        eASSERT(index < ROWS*COLS);
        return m[index];
    }

    eF32 operator [] (eInt index) const
    {
        eASSERT(index < ROWS*COLS);
        return m[index];
    }

    // Index operators, return element in
    // given row and column.

    const eF32 & operator () (eU32 row, eU32 col) const
    {
        eASSERT(row < ROWS);
        eASSERT(col < COLS);

        return mm[row][col];
    }

    eF32 & operator () (eU32 row, eU32 col)
    {
        eASSERT(row < ROWS);
        eASSERT(col < COLS);

        return mm[row][col];
    }

    // Bigger, not inlineable functions.
public:
    void        fromQuat(const eQuat &q);

    eF32        det() const;
    eF32        det3x3() const;
    void        invert();

    void        rotate(const eVector3 &rot);
    void        translate(const eVector3 &trans);
    void        scale(const eVector3 &scale);
    void        transformation(const eVector3 &rot, const eVector3 &trans, const eVector3 &scaling, eBool reverse=eFALSE);
    void        perspective(eF32 fov, eF32 aspect, eF32 znear, eF32 zfar);
    void        ortho(eF32 left, eF32 right, eF32 top, eF32 bottom, eF32 znear, eF32 zfar);
    void        cubemap(eU32 face);
    void        lookAt(const eVector3 &pos, const eVector3 &lookAt, const eVector3 &up);

	eVector3	getVector(eU32 which) const;
    eVector3    getTranslation() const; // Can't be inlined, because vector is only forward declared.
    eVector4    getTranslation4() const;
    eMatrix4x4  toRotationMatrix() const;

    eMatrix4x4  operator + (const eMatrix4x4 &m) const;
    eMatrix4x4  operator - (const eMatrix4x4 &m) const;
    eMatrix4x4  operator * (const eMatrix4x4 &m) const;
    eVector4    operator * (const eVector4 &v) const;
    eVector3    operator * (const eVector3 &v) const;

public:
    static const eInt ROWS = 4;
    static const eInt COLS = 4;

public:
    union
    {
        struct
        {
            eF32 m11, m12, m13, m14;
            eF32 m21, m22, m23, m24;
            eF32 m31, m32, m33, m34;
            eF32 m41, m42, m43, m44;
        };

        eF32 m[ROWS*COLS];
        eF32 mm[ROWS][COLS];
    };
};

typedef eArray<eMatrix4x4> eMatrix4x4Array;

#endif // MATRIX4X4_HPP