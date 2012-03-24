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

eVector3 eQuat::toEuler() const
{
    const eF32 ww = w*w;
    const eF32 xx = x*x;
    const eF32 yy = y*y;
    const eF32 zz = z*z;

    const eF32 rx = eATan2(2.0f*(y*z+x*w), -xx-yy+zz+ww);
    const eF32 ry = eASin(-2.0f*(x*z-y*w));
    const eF32 rz = eATan2(2.0f*(x*y+z*w), xx-yy-zz+ww);

    return eVector3(rx, ry, rz);
}

void eQuat::fromRotation(const eVector3 &v)
{
    const eF32 roll = v.x;
    const eF32 pitch = v.y;
    const eF32 yaw = v.z;

    const eF32 cyaw    = eCos(0.5f*yaw);
    const eF32 cpitch  = eCos(0.5f*pitch);
    const eF32 croll   = eCos(0.5f*roll);
    const eF32 syaw    = eSin(0.5f*yaw);
    const eF32 spitch  = eSin(0.5f*pitch);
    const eF32 sroll   = eSin(0.5f*roll);

    const eF32 cyawcpitch = cyaw*cpitch;
    const eF32 syawspitch = syaw*spitch;
    const eF32 cyawspitch = cyaw*spitch;
    const eF32 syawcpitch = syaw*cpitch;

    x = cyawcpitch*sroll-syawspitch*croll;
    y = cyawspitch*croll+syawcpitch*sroll;
    z = syawcpitch*croll-syawcpitch*sroll;
    w = cyawcpitch*croll+syawspitch*sroll;
}

void eQuat::fromMatrix(const eMatrix4x4 &m)
{
    const eF32 trace = m.m11+m.m22+m.m33+1.0f;

    if (!eIsFloatZero(trace)) 
    {
        const eF32 s = 0.5f/eSqrt(trace);

        x = (m.m32-m.m23)*s;
        y = (m.m13-m.m31)*s;
        z = (m.m21-m.m12)*s;
        w = 0.25f/s;
    } 
    else 
    {
        if (m.m11 > m.m22 && m.m11 > m.m33) 
        {
            const eF32 s = 2.0f*eSqrt(1.0f+m.m11-m.m22-m.m33);

            x = 0.25f*s;
            y = (m.m12+m.m21)/s;
            z = (m.m13+m.m31)/s;
            w = (m.m23-m.m32)/s;
        } 
        else if (m.m22 > m.m33) 
        {
            const eF32 s = 2.0f*eSqrt(1.0f+m.m22-m.m11-m.m33);

            x = (m.m12+m.m21)/s;
            y = 0.25f*s;
            z = (m.m23+m.m32)/s;
            w = (m.m13-m.m31)/s;
        } 
        else 
        {
            const eF32 s = 2.0f*eSqrt(1.0f+m.m33-m.m11-m.m22);

            x = (m.m13+m.m31)/s;
            y = (m.m23+m.m32)/s;
            z = 0.25f*s;
            w = (m.m12-m.m21)/s;
        }
    }
}

// Axis has to be normalized and angle
// has be in radians.
void eQuat::fromAxisAngle(const eVector3 &axis, eF32 angle)
{
    eASSERT(eAreFloatsEqual(axis.length(), 1.0f));

    eF32 sa, ca;

    eSinCos(angle*0.5f, sa, ca);
    (eFXYZ&)x = axis*sa;
    w = ca;

    normalize();
}

eQuat eQuat::lerp(eF32 t, const eQuat &to) const
{
    return eQuat(eLerp(x, to.x, t),
                 eLerp(y, to.y, t),
                 eLerp(z, to.z, t),
                 eLerp(w, to.w, t));
}

eQuat eQuat::slerp(eF32 t, const eQuat &to) const
{

    eASSERT(t >= 0.0f && t <= 1.0f);

	eQuat qb = to;
	// quaternion to return
	eQuat qm;
	// Calculate angle between them.
	eF32 cosHalfTheta = this->w * qb.w + this->x * qb.x + this->y * qb.y + this->z * qb.z;
	if (cosHalfTheta < 0.0f) {
	  qb.w = -qb.w; qb.x = -qb.x; qb.y = -qb.y; qb.z = qb.z;
	  cosHalfTheta = -cosHalfTheta;
	}
	// if qa=qb or qa=-qb then theta = 0 and we can return qa
	if (eAbs(cosHalfTheta) >= 1.0f){
		qm.w = this->w;
		qm.x = this->x;
		qm.y = this->y;
		qm.z = this->z;
		return qm;
	}
	// Calculate temporary values.
	eF32 halfTheta = eACos(cosHalfTheta);
	eF32 sinHalfTheta = eSqrt(1.0f - cosHalfTheta*cosHalfTheta);
	// if theta = 180 degrees then result is not fully defined
	// we could rotate around any axis normal to qa or qb
	if (eAbs(sinHalfTheta) < 0.001f){ // fabs is floating point absolute
		qm.w = (this->w * 0.5f + qb.w * 0.5f);
		(eFXYZ&)qm.x = ((eVector3&)this->x * 0.5f + (eVector3&)qb.x * 0.5f);
		return qm;
	}
	eF32 ratioA = eClamp(0.0f, eSin((1.0f - t) * halfTheta) / sinHalfTheta, 1.0f);
	eF32 ratioB = eClamp(0.0f, eSin(t * halfTheta) / sinHalfTheta, 1.0f); 
	//calculate Quaternion.
	qm.w = (this->w * ratioA + qb.w * ratioB);
	(eFXYZ&)qm.x = ((eVector3&)this->x * ratioA + (eVector3&)qb.x * ratioB);
	return qm;

}

eQuat eQuat::log() const
{
    const eF32 theta = eACos(w);
    const eF32 sinTheta = eSin(theta);

    if (!eIsFloatZero(sinTheta))
    {
        const eVector3 a = (eVector3&)x/sinTheta*theta;
        return eQuat(a.x, a.y, a.z, 0.0f);
    }

    return eQuat(x, y, z, 0.0f);
}

eQuat eQuat::exp() const
{
    const eF32 theta = eSqrt((eVector3&)x*(eVector3&)x);

    eF32 sinTheta, cosTheta;
    eSinCos(theta, sinTheta, cosTheta);

    if (!eIsFloatZero(sinTheta))
    {
        const eVector3 a = (eVector3&)x*sinTheta/theta;
        return eQuat(a.x, a.y, a.z, cosTheta);
    }
 
    return eQuat(x, y, z, cosTheta);
}

eQuat eQuat::squad(eF32 t, const eQuat &prev, const eQuat &from, const eQuat &to, const eQuat &next)
{
    eASSERT(t >= 0.0f && t <= 1.0f);

    const eQuat q0 = prev.slerp(t, from);
    const eQuat q1 = to.slerp(t, next);

    return q0.slerp(2.0f*(1.0f-t)*t, q1);
}

eQuat eQuat::getInterpolationTangent(const eQuat &prev, const eQuat &cur, const eQuat &next)
{
    eQuat qinv = cur;
    qinv.conjugate();

    const eQuat q0 = (qinv*prev).log();
    const eQuat q1 = (qinv*next).log();
    const eQuat qr = -((q0+q1)/4.0f);

    return cur*q0.exp();
}

eVector3 eQuat::getVector(eU32 nr) const
{
    const eF32 sqw = this->w*this->w;
    const eF32 sqx = this->x*this->x;
    const eF32 sqy = this->y*this->y;
    const eF32 sqz = this->z*this->z;

    const eF32 invs = 2.0f / (sqx + sqy + sqz + sqw);

	switch(nr) {
		case 0:
			return eVector3(0.5f * ( sqx - sqy - sqz + sqw),
						    x*y + z*w,
						    x*z - y*w) * invs;
		case 1:
			return eVector3(x*y - z*w,
							0.5f * (-sqx + sqy - sqz + sqw),
							y*z + x*w) * invs;
		default:
			return eVector3(x*z + y*w,
							y*z - x*w,
							0.5f * (-sqx - sqy + sqz + sqw)) * invs;
	}
}

#define vec4f_swizzle(v,p,q,r,s) (_mm_shuffle_ps( (v),(v), ((s)<<6|(r)<<4|(q)<<2|(p))))
const int zero=0;
const int flipSign=0x80000000;
const __m128 quat_mask=_mm_setr_ps( *(float*)&zero,
*(float*)&zero,
*(float*)&zero,
*(float*)&flipSign);

eQuat eQuat::operator * (const eQuat &q) const {

#ifdef eUSE_SSE
	__declspec(align(16)) eQuat result;
const __m128 a = _mm_loadu_ps(&this->x);
const __m128 b = _mm_loadu_ps(&q.x);
__m128 swiz1=vec4f_swizzle(b,3,3,3,3);
__m128 swiz2=vec4f_swizzle(a,2,0,1,0);
__m128 swiz3=vec4f_swizzle(b,1,2,0,0);
__m128 swiz4=vec4f_swizzle(a,3,3,3,1);
__m128 swiz5=vec4f_swizzle(b,0,1,2,1);
__m128 swiz6=vec4f_swizzle(a,1,2,0,2);
__m128 swiz7=vec4f_swizzle(b,2,0,1,2);
__m128 mul4=_mm_mul_ps(swiz6,swiz7);
__m128 mul3=_mm_mul_ps(swiz4,swiz5);
__m128 mul2=_mm_mul_ps(swiz2,swiz3);
__m128 mul1=_mm_mul_ps(a,swiz1);
__m128 flip1=_mm_xor_ps(mul4,quat_mask);
__m128 flip2=_mm_xor_ps(mul3,quat_mask);
__m128 retVal=_mm_sub_ps(mul1,mul2);
__m128 retVal2=_mm_add_ps(flip1,flip2);
__m128 res = _mm_add_ps(retVal,retVal2);

	_mm_storeu_ps(&result.x, res);
	return result;
#else

    const eF32 w2 = w*q.w-(eVector3&)x*(eVector3&)q.x;

    const eVector3 vcp = (eVector3&)x^(eVector3&)q.x;
    const eVector3 vq0 = (eVector3&)x*q.w;
    const eVector3 vq1 = (eVector3&)q.x*w;
    const eVector3 res = vq0+vq1+vcp;

    return eQuat(res.x, res.y, res.z, w2);
#endif
}
