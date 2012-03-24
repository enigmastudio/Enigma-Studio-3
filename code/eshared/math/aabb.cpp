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

// Transformation of AABBs is a bit tricky in order
// to compensate for rotations from model- to world-
// space (kills axis alignment).
void eAABB::transform(const eMatrix4x4 &mtx)
{
    ePROFILER_ZONE("bbox transform");
#ifdef eUSE_SSE
static const __m128 SIGNMASK =
               _mm_castsi128_ps(_mm_set1_epi32(0x80000000));
    __m128 size = _mm_loadu_ps(&m_size.x);
    __m128 newsize = _mm_andnot_ps(SIGNMASK, _mm_mul_ps(_mm_loadu_ps(&mtx.m11), _mm_shuffle_ps(size, size, _MM_SHUFFLE(0,0,0,0))));
    newsize = _mm_add_ps(newsize, _mm_andnot_ps(SIGNMASK, _mm_mul_ps(_mm_loadu_ps(&mtx.m21), _mm_shuffle_ps(size, size, _MM_SHUFFLE(1,1,1,1)))));
    newsize = _mm_add_ps(newsize, _mm_andnot_ps(SIGNMASK, _mm_mul_ps(_mm_loadu_ps(&mtx.m31), _mm_shuffle_ps(size, size, _MM_SHUFFLE(2,2,2,2)))));
    _mm_storeu_ps(&m_size.x, newsize);
	__m128 halfsize = _mm_mul_ps(size, _mm_set1_ps(0.5f));

    __m128 center = _mm_loadu_ps(&m_center.x);
	const __m128 a1 = _mm_mul_ps(_mm_loadu_ps(&mtx.m11), _mm_shuffle_ps(center, center, _MM_SHUFFLE(0, 0, 0, 0)));
	const __m128 a2 = _mm_mul_ps(_mm_loadu_ps(&mtx.m21), _mm_shuffle_ps(center, center, _MM_SHUFFLE(1, 1, 1, 1)));
	const __m128 a3 = _mm_mul_ps(_mm_loadu_ps(&mtx.m31), _mm_shuffle_ps(center, center, _MM_SHUFFLE(2, 2, 2, 2)));
	const __m128 sum1 = _mm_add_ps(a1, a2);
	const __m128 sum2 = _mm_add_ps(a3, _mm_loadu_ps(&mtx.m41));
	const __m128 newcenter = _mm_add_ps(sum1, sum2);
    _mm_storeu_ps(&m_center.x, newcenter );

    _mm_storeu_ps(&m_min.x, _mm_sub_ps(newcenter, halfsize));
    _mm_storeu_ps(&m_max.x, _mm_add_ps(newcenter, halfsize));
#else
    eVector3 nheX(mtx.m11*m_size.x, mtx.m12*m_size.x, mtx.m13*m_size.x);
    eVector3 nheY(mtx.m21*m_size.y, mtx.m22*m_size.y, mtx.m23*m_size.y);
    eVector3 nheZ(mtx.m31*m_size.z, mtx.m32*m_size.z, mtx.m33*m_size.z);

    nheX.absolute();
    nheY.absolute();
    nheZ.absolute();

    m_center *= mtx;
    m_size = nheX+nheY+nheZ;
    m_min = m_center-0.5f*m_size;
    m_max = m_center+0.5f*m_size;
#endif
}
