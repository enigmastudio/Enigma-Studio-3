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

// Initialize static variables.
const eVector3 eVector3::XAXIS(1.0f, 0.0f, 0.0f);
const eVector3 eVector3::YAXIS(0.0f, 1.0f, 0.0f);
const eVector3 eVector3::ZAXIS(0.0f, 0.0f, 1.0f);
const eVector3 eVector3::ORIGIN(0.0f, 0.0f, 0.0f);

eBool eVector3::isInsideCube(const ePlane cubePlanes[6]) const
{
    for (eU32 i=0; i<6; i++)
    {
        if (cubePlanes[i].getSide(*this) == ePlane::SIDE_BACK)
        {
            return eFALSE;
        }
    }

    return eTRUE;
}

void eVector3::cubicBezier(eF32 t, const eVector3 &cp0, const eVector3 &cp1, const eVector3 &cp2, const eVector3 &cp3, eVector3 &resPos, eVector3 &resTangent)
{
#ifdef eUSE_SSE
    __m128 mt = _mm_set1_ps(t);
    __m128 mtinv = _mm_set1_ps(1.0f - t);
    __m128 mcp0 = _mm_loadu_ps(&cp0.x);
    __m128 mcp1 = _mm_loadu_ps(&cp1.x);
    __m128 m0 = _mm_add_ps(_mm_mul_ps(mcp0, mtinv), _mm_mul_ps(mcp1, mt));
    __m128 mcp2 = _mm_loadu_ps(&cp2.x);
    __m128 m1 = _mm_add_ps(_mm_mul_ps(mcp1, mtinv), _mm_mul_ps(mcp2, mt));
    __m128 mm0 = _mm_add_ps(_mm_mul_ps(m0, mtinv), _mm_mul_ps(m1, mt));
    __m128 mcp3 = _mm_loadu_ps(&cp3.x);
    __m128 m2 = _mm_add_ps(_mm_mul_ps(mcp2, mtinv), _mm_mul_ps(mcp3, mt));
    __m128 mm1 = _mm_add_ps(_mm_mul_ps(m1, mtinv), _mm_mul_ps(m2, mt));
    __m128 mrespos = _mm_add_ps(_mm_mul_ps(mm0, mtinv), _mm_mul_ps(mm1, mt));
    _mm_storeu_ps(&resPos.x, mrespos);
    __m128 mrestangent = _mm_sub_ps(mm1, mm0);
    __m128 mdot = _mm_mul_ps(mrestangent, mrestangent);
    mdot = _mm_and_ps(mdot, _mm_set_ps(0x0,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF));
    __m128 mdotagg = _mm_hadd_ps(mdot, mdot);
    __m128 dotsum = _mm_hadd_ps(mdotagg, mdotagg);
    __m128 recipsqrt = _mm_rsqrt_ss( dotsum );
    __m128 tangentnorm = _mm_mul_ps(mrestangent, _mm_shuffle_ps(recipsqrt, recipsqrt, _MM_SHUFFLE(0,0,0,0)));
    _mm_storeu_ps(&resTangent.x, tangentnorm);
#else
    const eF32 tinv = 1.0f-t;

	const eVector3 m0 = cp0*tinv+cp1*t;
	const eVector3 m1 = cp1*tinv+cp2*t;
	const eVector3 m2 = cp2*tinv+cp3*t;
	const eVector3 mm0 = m0*tinv+m1*t;
	const eVector3 mm1 = m1*tinv+m2*t;

	resPos = mm0*tinv+mm1*t;
	resTangent = mm1-mm0;
#endif
}

eVector3 eVector3::catmullRom(eF32 t, const eVector3 &v0, const eVector3 &v1, const eVector3 &v2, const eVector3 &v3)
{
    const eF32 tt = t*t;
    return 0.5f*((2.0f*v1)+(-v0+v2)*t+(2.0f*v0-5.0f*v1+4.0f*v2-v3)*tt+(-v0+3.0f*(v1-v2)+v3)*tt*t);
}

// Calculates the distance to the given triangle (base, base + e0, base + e1), 
// Returns an eVector3(squaredDistance, s, t), where the closest point inside the triangle can be calculated as cp = base + e0 * s  + e1 * t
eVector3 eVector3::distanceToTriangle(const eVector3& base, const eVector3& e0, const eVector3& e1) {
    return distanceToTriangleOptimized(base, e0, e1, e0 * e0, e0 * e1, e1 * e1);
}

eVector3 eVector3::distanceToTriangleOptimized(const eVector3& base, const eVector3& e0, const eVector3& e1, eF32 e0_DOT_e0, eF32 e0_DOT_e1, eF32 e1_DOT_e1) {
    // find closest face
    // calculate distance to face according to http://www.geometrictools.com/Documentation/DistancePoint3Triangle3.pdf
    eVector3 result;
    eVector3& P = *this;
    const eVector3 D = base - P;
    eF32& a = e0_DOT_e0;
    eF32& b = e0_DOT_e1;
    eF32& c = e1_DOT_e1;
    eF32 d = e0 * D;
    eF32 e = e1 * D;
    eF32 f = D * D;

    eF32 det = a*c-b*b; 

    eF32& sqrDistance = result.x;
    eF32& s = result.y;
    eF32& t = result.z;

    s = b*e-c*d; 
    t = b*d-a*e;
    if (s+t <= det) {
        if(s < 0) { 
            if (t < 0) { 
                if (d < 0) {
                    t = 0;
                    if(-d >= a) {
                        s = 1.0f;
                        sqrDistance = a + 2*d + f;
                    } else {
                        s = -d/a;
                        sqrDistance = a + 2*d + f;
                    }
                } else {
                    s = 0;
                    if (e >= 0) {
                        t = 0;
                        sqrDistance = f;
                    } else 
                        if (-e >= c) {
                            t = 1;
                            sqrDistance = c + 2*e + f;
                        } else {
                            t = -e/c;
                            sqrDistance = e*t + f;
                        }
                }
            } else { 
                // F(t) = Q(0,t) = ct^2 + 2et + f
                // F’(t)/2 = ct+e
                // F’(T) = 0 when T = -e/c
                s = 0;
                if(e >= 0) {
                    t = 0;
                    sqrDistance = f;
                } else 
                    if(-e >= c) {
                        t = 1;
                        sqrDistance = c + 2*e +f;
                    } else {
                        t = -e/c;
                        sqrDistance = e*t + f;
                    }
            } 
        } else if (t < 0) { 
            t = 0;
            if(d >= 0) {
                s = 0;
                sqrDistance = f;
            } else 
                if(-d >= a) {
                    s = 1;
                    sqrDistance = a + 2*d + f;
                } else { 
                    s = -d/a;
                    sqrDistance = d*s + f;
                }
        } else { 
            eF32 invDet = 1.0f/det;
            s *= invDet;
            t *= invDet;
            sqrDistance = s*(a*s + b*t + 2*d) + t*(b*s + c*t + 2*e) + f;
        }
    } else {
        if(s < 0) { 
            // Grad(Q) = 2(as+bt+d,bs+ct+e)
            // (0,-1)*Grad(Q(0,1)) = (0,-1)*(b+d,c+e) = -(c+e)
            // (1,-1)*Grad(Q(0,1)) = (1,-1)*(b+d,c+e) = (b+d)-(c+e)
            // min on edge s+t=1 if (1,-1)*Grad(Q(0,1)) < 0 )
            // min on edge s=0 otherwise
            eF32 tmp0 = b+d;
            eF32 tmp1 = c+e;
            if (tmp1 > tmp0) { // minimum on edge s+t=1
                eF32 numer = tmp1 - tmp0;
                eF32 denom = a-2*b+c;
                if(numer >= denom) {
                    s = 1;
                    t = 0;
                    sqrDistance = a + 2*d + f; 
                } else {
                    s = numer/denom;
                    t = 1-s;
                    sqrDistance = s*(a*s + b*t + 2*d) + t*(b*s + c*t + 2*e) + f;
                }
            } else { // minimum on edge s=0
                s = 0;
                if(tmp1 <= 0) {
                    t = 1;
                    sqrDistance = c + 2*e + f;
                } else
                    if (e >= 0) {
                        t = 0;
                        sqrDistance = f;
                    } else {
                        t = -e/c;
                        sqrDistance = e*t + f;
                    }
            }
        } else if (t < 0) { 
            eF32 tmp0 = b + e;
            eF32 tmp1 = a + d;
            if (tmp1 > tmp0) {
                eF32 numer = tmp1 - tmp0;
                eF32 denom = a-2*b+c;
                if (numer >= denom) {
                    t = 1;
                    s = 0;
                    sqrDistance = c + 2*e + f;
                } else {
                    t = numer/denom;
                    s = 1 - t;
                    sqrDistance = s*(a*s + b*t + 2*d) + t*(b*s + c*t + 2*e) + f;
                }
            } else {
                t = 0;
                if (tmp1 <= 0) {
                    s = 1;
                    sqrDistance = a + 2*d + f;
                } else 
                    if (d >= 0) {
                        s = 0;
                        sqrDistance = f;
                    } else {
                        s = -d/a;
                        sqrDistance = d*s + f;
                    }
            }
        } else { 
            // F(s) = Q(s,1-s) = (a-2b+c)s^2 + 2(b-c+d-e)s + (c+2e+f)
            // F’(s)/2 = (a-2b+c)s + (b-c+d-e)
            // F’(S) = 0 when S = (c+e-b-d)/(a-2b+c)
            // a-2b+c = |E0-E1|^2 > 0, so only sign of c+e-b-d need be considered
            eF32 numer = c + e - b - d;
            if(numer <= 0) {
                s = 0;
                t = 1;
                sqrDistance = c + 2*e + f;
            } else {
                eF32 denom = a-2*b+c; // positive quantity
                if(numer >= denom) {
                    s = 1;
                    t = 0;
                    sqrDistance = a + 2*d + f;
                } else {
                    s = numer/denom;
                    t = 1-s;
                    sqrDistance = s*(a*s + b*t + 2*d) + t*(b*s + c*t + 2*e) + f;
                }
            }
        }
    }

    // account for numerical round-off error
    if (sqrDistance < 0)
        sqrDistance = 0;

    return result;
}

