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

#include "../system/system.hpp"
#include "../math/math.hpp"

#ifdef eUSE_SSE
#include <intrin.h>
#endif

void eMatrix4x4::fromQuat(const eQuat &q)
{
    const eF32 sqw = q.w*q.w;
    const eF32 sqx = q.x*q.x;
    const eF32 sqy = q.y*q.y;
    const eF32 sqz = q.z*q.z;

//	eASSERT(eAreFloatsEqual(q.length(), 1.0f));

	m11 = ( sqx - sqy - sqz + sqw); 
    m22 = (-sqx + sqy - sqz + sqw);
    m33 = (-sqx - sqy + sqz + sqw);
    
    eF32 tmp1 = q.x*q.y;
    eF32 tmp2 = q.z*q.w;
    m21 = 2.0f * (tmp1 + tmp2);
    m12 = 2.0f * (tmp1 - tmp2);
    
    tmp1 = q.x*q.z;
    tmp2 = q.y*q.w;
    m31 = 2.0f * (tmp1 - tmp2);
    m13 = 2.0f * (tmp1 + tmp2);
    tmp1 = q.y*q.z;
    tmp2 = q.x*q.w;
    m32 = 2.0f * (tmp1 + tmp2);
    m23 = 2.0f * (tmp1 - tmp2);
	m14 = 0.0f;
	m24 = 0.0f;
	m34 = 0.0f;
	m41 = 0.0f;
	m42 = 0.0f;
	m43 = 0.0f;
	m44 = 1.0f;
}

eF32 eMatrix4x4::det() const
{
    return +(m11*m22-m12*m21)*(m33*m44-m34*m43)
           -(m11*m23-m13*m21)*(m32*m44-m34*m42)
           +(m11*m24-m14*m21)*(m32*m43-m33*m42)
           +(m12*m23-m13*m22)*(m31*m44-m34*m41)
           -(m12*m24-m14*m22)*(m31*m43-m33*m41)
           +(m13*m24-m14*m23)*(m31*m42-m32*m41);
}

eF32 eMatrix4x4::det3x3() const
{
    return m11*(m22*m33-m23*m32)+
           m12*(m23*m31-m21*m33)+
           m13*(m21*m32-m22*m31);
}

void eMatrix4x4::invert()
{
    ePROFILER_ZONE("Matrix inverse");

#ifdef eUSE_SSE
    // intel's optimized version (available on the interweb)
    __m128 zero = _mm_setzero_ps();
    __m128 minor0, minor1, minor2, minor3;
    __m128 row0, row1, row2, row3;
    __m128 det, tmp1;
    tmp1 = _mm_loadh_pi(_mm_loadl_pi(zero, (__m64*)(this->m)), (__m64*)(this->m+ 4));
    row1 = _mm_loadh_pi(_mm_loadl_pi(zero, (__m64*)(this->m+8)), (__m64*)(this->m+12));
    row0 = _mm_shuffle_ps(tmp1, row1, 0x88);
    row1 = _mm_shuffle_ps(row1, tmp1, 0xDD);
    tmp1 = _mm_loadh_pi(_mm_loadl_pi(zero, (__m64*)(this->m+ 2)), (__m64*)(this->m+ 6));
    row3 = _mm_loadh_pi(_mm_loadl_pi(zero, (__m64*)(this->m+10)), (__m64*)(this->m+14));
    row2 = _mm_shuffle_ps(tmp1, row3, 0x88);
    row3 = _mm_shuffle_ps(row3, tmp1, 0xDD);
// -----------------------------------------------
    tmp1 = _mm_mul_ps(row2, row3);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
    minor0 = _mm_mul_ps(row1, tmp1);
    minor1 = _mm_mul_ps(row0, tmp1);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
    minor0 = _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
    minor1 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
    minor1 = _mm_shuffle_ps(minor1, minor1, 0x4E);
// -----------------------------------------------
    tmp1 = _mm_mul_ps(row1, row2);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
    minor0 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
    minor3 = _mm_mul_ps(row0, tmp1);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
    minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
    minor3 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
    minor3 = _mm_shuffle_ps(minor3, minor3, 0x4E);
// -----------------------------------------------
    tmp1 = _mm_mul_ps(_mm_shuffle_ps(row1, row1, 0x4E), row3);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
    row2 = _mm_shuffle_ps(row2, row2, 0x4E);
    minor0 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
    minor2 = _mm_mul_ps(row0, tmp1);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
    minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
    minor2 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
    minor2 = _mm_shuffle_ps(minor2, minor2, 0x4E);
// -----------------------------------------------
    tmp1 = _mm_mul_ps(row0, row1);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
    minor2 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
    minor3 = _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
    minor2 = _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
    minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));
// -----------------------------------------------
    tmp1 = _mm_mul_ps(row0, row3);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
    minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
    minor2 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
    minor1 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
    minor2 = _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));
// -----------------------------------------------
    tmp1 = _mm_mul_ps(row0, row2);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
    minor1 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
    minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
    minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
    minor3 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);
// -----------------------------------------------
    det = _mm_mul_ps(row0, minor0);
    det = _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
    det = _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);
    tmp1 = _mm_rcp_ss(det);
    det = _mm_sub_ss(_mm_add_ss(tmp1, tmp1), _mm_mul_ss(det, _mm_mul_ss(tmp1, tmp1)));
    det = _mm_shuffle_ps(det, det, 0x00);
    minor0 = _mm_mul_ps(det, minor0);
    _mm_storel_pi((__m64*)(this->m), minor0);
    _mm_storeh_pi((__m64*)(this->m+2), minor0);
    minor1 = _mm_mul_ps(det, minor1);
    _mm_storel_pi((__m64*)(this->m+4), minor1);
    _mm_storeh_pi((__m64*)(this->m+6), minor1);
    minor2 = _mm_mul_ps(det, minor2);
    _mm_storel_pi((__m64*)(this->m+ 8), minor2);
    _mm_storeh_pi((__m64*)(this->m+10), minor2);
    minor3 = _mm_mul_ps(det, minor3);
    _mm_storel_pi((__m64*)(this->m+12), minor3);
    _mm_storeh_pi((__m64*)(this->m+14), minor3);

#else
    const eF32 d = det();
    const eF32 od = 1.0f/d;

    // Matrix is only invertable, if determinant isn't zero.
    //eASSERT(!eIsFloatZero(d));

    eMatrix4x4 r;
    r.m11 = od*(m22*(m33*m44-m34*m43)+m23*(m34*m42-m32*m44)+m24*(m32*m43-m33*m42));
    r.m12 = od*(m32*(m13*m44-m14*m43)+m33*(m14*m42-m12*m44)+m34*(m12*m43-m13*m42));
    r.m13 = od*(m42*(m13*m24-m14*m23)+m43*(m14*m22-m12*m24)+m44*(m12*m23-m13*m22));
    r.m14 = od*(m12*(m24*m33-m23*m34)+m13*(m22*m34-m24*m32)+m14*(m23*m32-m22*m33));

    r.m21 = od*(m23*(m31*m44-m34*m41)+m24*(m33*m41-m31*m43)+m21*(m34*m43-m33*m44));
    r.m22 = od*(m33*(m11*m44-m14*m41)+m34*(m13*m41-m11*m43)+m31*(m14*m43-m13*m44));
    r.m23 = od*(m43*(m11*m24-m14*m21)+m44*(m13*m21-m11*m23)+m41*(m14*m23-m13*m24));
    r.m24 = od*(m13*(m24*m31-m21*m34)+m14*(m21*m33-m23*m31)+m11*(m23*m34-m24*m33));

    r.m31 = od*(m24*(m31*m42-m32*m41)+m21*(m32*m44-m34*m42)+m22*(m34*m41-m31*m44));
    r.m32 = od*(m34*(m11*m42-m12*m41)+m31*(m12*m44-m14*m42)+m32*(m14*m41-m11*m44));
    r.m33 = od*(m44*(m11*m22-m12*m21)+m41*(m12*m24-m14*m22)+m42*(m14*m21-m11*m24));
    r.m34 = od*(m14*(m22*m31-m21*m32)+m11*(m24*m32-m22*m34)+m12*(m21*m34-m24*m31));

    r.m41 = od*(m21*(m33*m42-m32*m43)+m22*(m31*m43-m33*m41)+m23*(m32*m41-m31*m42));
    r.m42 = od*(m31*(m13*m42-m12*m43)+m32*(m11*m43-m13*m41)+m33*(m12*m41-m11*m42));
    r.m43 = od*(m41*(m13*m22-m12*m23)+m42*(m11*m23-m13*m21)+m43*(m12*m21-m11*m22));
    r.m44 = od*(m11*(m22*m33-m23*m32)+m12*(m23*m31-m21*m33)+m13*(m21*m32-m22*m31));

    *this = r;
#endif
}

// Rotation angles have to be in radians.
void eMatrix4x4::rotate(const eVector3 &rot)
{
    //Do x-rotation.
    eMatrix4x4 m;

    m.m22 =  eCos(rot.x);
    m.m32 = -eSin(rot.x);
    m.m23 = -m.m32;
    m.m33 =  m.m22;
    *this *= m;

    //Do y-rotation.
    m.identity();
    m.m11 =  eCos(rot.y);
    m.m31 =  eSin(rot.y);
    m.m13 = -m.m31;
    m.m33 =  m.m11;
    *this *= m;

    //Do z-rotation.
    m.identity();
    m.m11 =  eCos(rot.z);
    m.m21 = -eSin(rot.z);
    m.m12 = -m.m21;
    m.m22 =  m.m11;
    *this *= m;
}

void eMatrix4x4::translate(const eVector3 &trans)
{
    eMatrix4x4 m;

    m.m41 = trans.x;
    m.m42 = trans.y;
    m.m43 = trans.z;

    *this *= m;
}

void eMatrix4x4::scale(const eVector3 &scale)
{
    eMatrix4x4 m;

    m.m11 = scale.x;
    m.m22 = scale.y;
    m.m33 = scale.z;

    *this *= m;
}

void eMatrix4x4::transformation(const eVector3 &rot, const eVector3 &trans, const eVector3 &scaling, eBool reverse)
{
    identity();
    scale(scaling);

    if (reverse)
    {
        translate(trans);
        rotate(rot);
    }
    else
    {
        rotate(rot);
        translate(trans);
    }
}

// Field of view is assumed to be in degree.
void eMatrix4x4::perspective(eF32 fov, eF32 aspect, eF32 znear, eF32 zfar)
{
    eASSERT(fov > 0.0f);
    eASSERT(aspect > 0.0f);

    identity();

    const eF32 yScale = eCot(eDegToRad(fov)*0.5f);
    const eF32 xScale = yScale/aspect;

    m11 = xScale;
    m22 = yScale;
    m33 = zfar/(zfar-znear);
    m43 = -znear*zfar/(zfar-znear);
    m34 = 1.0f;
    m44 = 0.0f;
}

void eMatrix4x4::ortho(eF32 left, eF32 right, eF32 top, eF32 bottom, eF32 znear, eF32 zfar)
{
    identity();

    m11 =          2.0f/(right-left);
    m41 =  (left+right)/(left-right);
    m22 =          2.0f/(bottom-top);
    m42 = -(top+bottom)/(bottom-top);
    m33 =          1.0f/(zfar-znear);
    m43 =         znear/(znear-zfar);
}

// face is in [0,..,5], with
// 0: positive x
// 1: negative x
// 2: positive y
// 3: negative y
// 4: positive z
// 5: negative z.
void eMatrix4x4::cubemap(eU32 face)
{
    eASSERT(face < 6);

    null();

    switch(face)
    {
        case 0: // positive x
        {
            m31 = -1.0f;
            m22 =  1.0f;
            m13 =  1.0f;
            m44 =  1.0f;
            break;
        }
        
        case 1: // negative x
        {
            m31 =  1.0f;
            m22 =  1.0f;
            m13 = -1.0f;
            m44 =  1.0f;
            break;
        }
        
        case 2: // positive y
        {
            m11 =  1.0f;
            m32 = -1.0f;
            m23 =  1.0f;
            m44 =  1.0f;
            break;
        }
        
        case 3: // negative y
        {
            m11 =  1.0f;
            m32 =  1.0f;
            m23 = -1.0f;
            m44 =  1.0f;
            break;
        }
        
        case 4: // positive z
        {
            m11 =  1.0f;
            m22 =  1.0f;
            m33 =  1.0f;
            m44 =  1.0f;
            break;
        }
        
        case 5: // negative z
        {
            m11 = -1.0f;
            m22 =  1.0f;
            m33 = -1.0f;
            m44 =  1.0f;
            break;
        }
    }
}

void eMatrix4x4::lookAt(const eVector3 &pos, const eVector3 &lookAt, const eVector3 &up)
{
    identity();

    const eVector3 zAxis = (lookAt-pos).normalized();
    const eVector3 xAxis = (up^zAxis).normalized();
    const eVector3 yAxis = zAxis^xAxis;

    m11 = xAxis.x;
    m12 = yAxis.x;
    m13 = zAxis.x;

    m21 = xAxis.y;
    m22 = yAxis.y;
    m23 = zAxis.y;

    m31 = xAxis.z;
    m32 = yAxis.z;
    m33 = zAxis.z;

    m41 = -xAxis*pos;
    m42 = -yAxis*pos;
    m43 = -zAxis*pos;
}

eVector3 eMatrix4x4::getVector(eU32 which) const
{
	eASSERT(which >= 0 && which < 4);
	return eVector3(m[which], m[which+4], m[which+8]);
}


eVector3 eMatrix4x4::getTranslation() const
{
    return eVector3(m41, m42, m43);
}

eVector4 eMatrix4x4::getTranslation4() const
{
    return eVector4(m41, m42, m43, m44);
}

eMatrix4x4 eMatrix4x4::operator + (const eMatrix4x4 &m) const
{
    return eMatrix4x4(m11+m.m11, m12+m.m12, m13+m.m13, m14+m.m14,
                      m21+m.m21, m22+m.m22, m23+m.m23, m24+m.m24,
                      m31+m.m31, m32+m.m32, m33+m.m33, m34+m.m34,
                      m41+m.m41, m42+m.m42, m43+m.m43, m44+m.m44);
}

eMatrix4x4 eMatrix4x4::operator - (const eMatrix4x4 &m) const
{
    return eMatrix4x4(m11-m.m11, m12-m.m12, m13-m.m13, m14-m.m14,
                      m21-m.m21, m22-m.m22, m23-m.m23, m24-m.m24,
                      m31-m.m31, m32-m.m32, m33-m.m33, m34-m.m34,
                      m41-m.m41, m42-m.m42, m43-m.m43, m44-m.m44);
}

eMatrix4x4 eMatrix4x4::operator * (const eMatrix4x4 &mtx) const
{
    ePROFILER_ZONE("Matrix multiply");
/*
// BUGGY !!!!!!!!!!

    eMatrix4x4 result(*this);
    result *= m;
    return result;
/**/
#ifdef eUSE_SSE
	eMatrix4x4 res;

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

		_mm_storeu_ps(&res.m[i], a2);
	}

    return res;
#else
    return eMatrix4x4(m11*mtx.m11+m12*mtx.m21+m13*mtx.m31+m14*mtx.m41,
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
/**/
}

eVector4 eMatrix4x4::operator * (const eVector4 &v) const
{
    return eVector4(m11*v.x+m12*v.y+m13*v.z+m14*v.w,
                    m21*v.x+m22*v.y+m23*v.z+m24*v.w,
                    m31*v.x+m32*v.y+m33*v.z+m34*v.w,
                    m41*v.x+m42*v.y+m43*v.z+m44*v.w);
}

eVector3 eMatrix4x4::operator * (const eVector3 &v) const
{
    return eVector3(m11*v.x+m12*v.y+m13*v.z+m14,
                    m21*v.x+m22*v.y+m23*v.z+m24,
                    m31*v.x+m32*v.y+m33*v.z+m34);
}

eMatrix4x4  eMatrix4x4::toRotationMatrix() const {
    eMatrix4x4 result;
    for(eU32 i = 0; i < 3; i++) {
        eF32 invLen = 1.0f / this->getVector(i).length();
        result.m[i] = m[i] * invLen;
        result.m[i+4] = m[i+4] * invLen;
        result.m[i+8] = m[i+8] * invLen;
    }
    return result;
}
