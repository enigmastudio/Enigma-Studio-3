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

#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#ifndef _WIN32
#include <math.h>
#include <stddef.h>
#endif

#ifdef _WIN32
#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#endif

// Some global constants. (DONT CHANGE 'EM INTO CONSTS !!! ITSA SIZE THANG !!!)

#define eSQRT2            1.41421356237f
#define ePI               3.1415926535897932384626433832795f
#define eTWOPI            ePI*2.0f
#define eHALFPI           ePI*0.5f
#define eSQRPI            ePI*ePI
#define eINVHALFPI        1.0f/eHALFPI
#define eEXPONE           2.718281828459f
#define eALMOST_ZERO      0.0001f
#define eMAX_RAND         2147483647
#define eMAX_NAME_LENGTH  64

// Some bigger macros.

#ifdef eDEBUG
    #define eASSERT(expr)                                   \
    {                                                       \
        if (!(expr))                                        \
        {                                                   \
            if (eShowAssertion(#expr, __FILE__, __LINE__))  \
            {                                               \
                __asm int 3                                 \
            }                                               \
        }                                                   \
    }

    #define eCLEAR_UNDENORMALIZED()
    #define eASSERT_UNDENORMALIZED()
#else   
    #define eASSERT(x)
    #define eCLEAR_UNDENORMALIZED()
    #define eASSERT_UNDENORMALIZED()
#endif

#define eSAFE_DELETE(p)         \
{                               \
    delete p;                   \
    p = eNULL;                  \
}

#define eSAFE_DELETE_ARRAY(p)   \
{                               \
    delete [] p;                \
    p = eNULL;                  \
}

#define eSAFE_DELETE_ALIGNED_ARRAY(p)   \
{                               \
    eFreeAlignedArray(p);                \
    p = eNULL;                  \
}

#define eSAFE_RELEASE_COM(p)    \
{                               \
    if (p)                      \
    {                           \
        p->Release();           \
        p = eNULL;              \
    }                           \
}

#ifdef _WIN32
// Overloaded new and delete operators. WinAPI
// versions for release build and memory tracking
// versions for debug build.
#ifdef eRELEASE
    ePtr __cdecl operator new [] (eU32 size);
    void __cdecl operator delete [] (ePtr p);
    ePtr __cdecl operator new(eU32 size);
    void __cdecl operator delete(ePtr p);
#else
    #include <crtdbg.h>
    #undef new

    eINLINE ePtr __cdecl operator new(eU32 size, const eChar *file, eU32 line)
    {
        return _malloc_dbg(size, _NORMAL_BLOCK, file, line);
    }

    eINLINE void __cdecl operator delete(ePtr p, const eChar *file, eU32 line)
    {
        _free_dbg(p, _NORMAL_BLOCK);
    }

    #define new new(__FILE__, __LINE__)
#endif
#endif

#ifdef eUSE_SSE
eBool eSSECmpTrue(__m128 value);
#endif

eBool   eVerifyInstructionSets();
void    eSetSSEFlushToZeroMode();
void    eInitGlobalsStatics();
void    eFreeGlobalsStatics();
void    eMemTrackerStart();
void    eMemTrackerStop();
eBool   eShowAssertion(const eChar *exp, const eChar *file, eU32 line);
void    eShowError(const eChar *error);

// Bigger, not inlineable functions.

ePtr	eMemAllocAlignedAndZero(eU32 size, size_t alignment);
void	eFreeAligned(ePtr ptr);
void	eFreeAlignedArray(ePtr ptr);
ePtr    eMemRealloc(ePtr ptr, eU32 oldLength, eU32 newLength);
void    eMemSet(ePtr dst, eU8 value, eU32 count);
void    eMemCopy(ePtr dst, eConstPtr src, eU32 count);
void    eMemMove(ePtr dst, eConstPtr src, eU32 count);
eBool   eMemEqual(eConstPtr mem0, eConstPtr mem1, eU32 count);
void    eStrClear(eChar *str);
void    eStrCopy(eChar *dst, const eChar *src);
void    eStrNCopy(eChar *dst, const eChar *src, eU32 count);
eChar * eStrClone(const eChar *str);
eU32    eStrLength(const eChar *str);
eChar * eStrAppend(eChar *dst, const eChar *src);
eInt    eStrCompare(const eChar *str0, const eChar *str1);
eChar * eStrUpper(eChar *str);
eChar * eIntToStr(eInt value);
eInt    eStrToInt(const eChar *str);
void    eRandomize(eU32 seed);
eU32    eRandom();
eU32	eRandom(eU32 &seed);
eU32	eRandomSeed();
eF32    ePow(eF32 base, eF32 exp);
eF32    eSinH(eF32 x);
eF32    eTanH(eF32 x);
eF32    eASin(eF32 x);
eF32    eACos(eF32 x);
eF32    eExp(eF32 x);
eInt    eFloor(eF32 x);
eInt    eCeil(eF32 x);

// Small, inlineable functions.

eINLINE eF32 eAbs(eF32 x)
{
#ifdef _WIN32
    __asm
    {
        fld     dword ptr [x]
        fabs
    }
#else
    return fabs(x);
#endif
}

eINLINE eU32 eAbs(eInt x)
{
    return ((x^(x>>31))-(x>>31));
}

eINLINE eBool eIsFloatZero(eF32 x)
{
    return (eAbs(x) < eALMOST_ZERO);
}

eINLINE eBool eAreFloatsEqual(eF32 x, eF32 y)
{
    return eIsFloatZero(x-y);
}

eINLINE eBool eIsNAN(eF32 x)
{
    return x != x;
}


// Returns base 10 logarithm (x > 0).
eINLINE eF32 eLog(eF32 x)
{
#ifdef _WIN32
    __asm
    {
        fld1
        fld     dword ptr [x]
        fyl2x
        fldl2t
        fdiv
    }
#else
    return logf(x);
#endif
}

eINLINE eF32 eLog2(eF32 x)
{
#ifdef _WIN32
    __asm
    {
        fld1
        fld     dword ptr [x]
        fyl2x
    }
#else
    return log2f(x);
#endif
}


// Returns base e logarithm (x > 0).
eINLINE eF32 eLn(eF32 x)
{
#ifdef _WIN32
    __asm
    {
        fld1
        fld     dword ptr [x]
        fyl2x
        fldl2e
        fdiv
    }
#else
    return 0.0f;
#endif
}

eINLINE eF32 eSin(eF32 x)
{
#ifdef _WIN32
    __asm
    {
        fld     dword ptr [x]
        fsin
    }
#else
    return sinf(x);
#endif
}

eINLINE eF32 eCos(eF32 x)
{
#ifdef _WIN32
    __asm
    {
        fld     dword ptr [x]
        fcos
    }
#else
    return cosf(x);
#endif
}

// Faster cosine by approximating result.
// Maximum absolute error is 1.1880e-03.
// Speedup is about factor 2.14.
eINLINE eF32 eFastCos(eF32 x)
{
    const eF32 xx = x*x;
    return (3.705e-02f*xx-4.967e-01f)*xx+1.0f;
}

// Implemented this way, because linker
// crashes else. Shitty M$ software.
eINLINE static eNAKED eF32 eFASTCALL eTan(eF32 x)
{
#ifdef _WIN32
    __asm
    {
        fld     dword ptr [esp+4]
        fptan
        fstp    dword ptr [esp+4]
        ret     4
    }
#else
    return tanf(x);
#endif
}

eINLINE eF32 eCot(eF32 x)
{
    eASSERT(!eIsFloatZero(eTan(x)));
    return 1.0f/eTan(x);
}

eINLINE eF32 eATan(eF32 x)
{
#ifdef _WIN32
    __asm
    {
        fld     dword ptr [x]
        fld1
        fpatan
        fstp    dword ptr [x]
        fld     dword ptr [x]
    }
#else
    return atanf(x);
#endif
}

eINLINE eF32 eATan2(eF32 y, eF32 x)
{
#ifdef _WIN32
    __asm
    {
        fld     dword ptr [y]
        fld     dword ptr [x]
        fpatan
        fstp    dword ptr [x]
        fld     dword ptr [x]
    }
#else
    return atan2f(y, x);
#endif
}

eINLINE eF32 eATanh(eF32 x)
{
	return 0.5f*eLn((1.0f+x)/(1.0f-x));
}

eINLINE void eSinCos(eF32 x, eF32 &sine, eF32 &cosine)
{
#ifdef _WIN32
    __asm
    {
        fld     dword ptr [x]
        fsincos
        mov     eax, dword ptr [cosine]
        fstp    dword ptr [eax]
        mov     eax, dword ptr [sine]
        fstp    dword ptr [eax]
    }
#else
#endif
}

eINLINE eF32 eSqrt(eF32 x)
{
#ifdef _WIN32
    __asm
    {
        fld     dword ptr [x]
        fsqrt
    }
#else
    return sqrt(x);
#endif
}

eINLINE eF32 eInvSqrt(eF32 x)
{
    eASSERT(x >= 0.0f);
    return 1.0f/eSqrt(x);
}

// Returns the floating point remainder of a.
// and b (so a = i*b+f, with f being returned
// and i is an integer value).
eINLINE eF32 eMod(eF32 a, eF32 b)
{
#ifdef _WIN32
    __asm
    {
        fld     dword ptr [b]
        fld     dword ptr [a]
        fprem
        fstp    st(1)
    }
#else
    return fmodf(a, b);
#endif
}

eINLINE eF32 eDegToRad(eF32 degrees)
{
    return degrees/360.0f*eTWOPI;
}

eINLINE eF32 eRadToDeg(eF32 radians)
{
    return radians/eTWOPI*360.0f;
}

// Returns wether or not the given pointer is
// properly aligned.
eINLINE eBool isAligned(eConstPtr data, eU32 alignment)
{
    // Check that the alignment is a power-of-two.
    eASSERT((alignment & (alignment-1)) == 0); 

    return (((eU32)data & (alignment-1)) == 0);
}

// Casts a floating point value to a 32-bit
// unsigned integer. Mainly used to pass float
// values as parameters to DirectX functions.
eINLINE eU32 eFtoDW(eF32 f)
{
    return *((eU32 *)&f);
}

// Hashing functions for integers. Can be used
// to build more complex hashing functions for
// different data types.
eINLINE eU32 eHashInt(const eInt &key)
{
    eU32 hash = (eU32)key;
    hash = (hash^61)^(hash>>16);
    hash = hash+(hash<<3);
    hash = hash^(hash>>4);
    hash = hash*0x27d4eb2d;
    hash = hash^(hash>>15);

    return hash;
}

// Implements the DJB2 hash function as first
// reported by Dan Bernstein.
eINLINE eU32 eHashStr(const eChar *str)
{
    eASSERT(str != eNULL);

    eU32 hash = 5381;
    eChar c;

    while (c = *str++)
    {
        // Does a hash*33+c.
        hash = ((hash<<5)+hash)+c;
    }

    return hash;
}

template<class T> eU32 eHashPtr(const T * const &ptr)
{
    return eHashInt((eInt)ptr);
}

// Faster float to long conversion that c-lib's
// default version. Must be called explicitly.
eINLINE eInt eFtoL(eF32 x)
{
#ifdef _WIN32
    __asm
    {
        fld     dword ptr [x]
        push    eax
        fistp   dword ptr [esp]
        pop     eax
    }
#else
    return (eInt)x;
#endif
}

eINLINE eF32 eRound(eF32 x)
{
#ifdef _WIN32
    __asm
    {
        fld     dword ptr [x]
        frndint
    }
#else
    return roundf(x);
#endif
}

eINLINE eU32 eRoundToMultiple(eU32 x, eU32 multiple)
{
    eASSERT(multiple > 0);

    const eU32 remainder = x%multiple;

    if (remainder == 0)
    {
        return x;
    }

    return x+multiple-remainder;
}

eINLINE eInt eTrunc(eF32 x)
{
#ifdef _WIN32
#ifdef eUSE_SSE
    //__asm cvttss2si eax, dword ptr [x]

// SSE version: (but slower)
	__asm fld DWORD ptr [x]
	__asm fisttp DWORD ptr [x]
	__asm mov eax, DWORD ptr [x]

#else
    const eF32 roundTowardsMi = -0.5f;
    eInt i;

    __asm
    {
        fld     dword ptr [x]
        fadd    st, st (0)
        fabs
        fadd    roundTowardsMi
        fistp   dword ptr [i]
        sar     dword ptr [i], 1
    }

    if (x < 0.0f)
    {
        i = -i;
    }

    return i;
#endif
#else
    return eFtoL(floorf(x));
#endif
}

eINLINE eInt eRandom(eInt min, eInt max)
{
    return eRandom()%(max-min)+min;
}

eINLINE eInt eRandom(eInt min, eInt max, eU32 &seed)
{
	return eRandom(seed)%(max-min)+min;
}

// Returns a random number in range [1/MAX_RAND,1-1/MAX_RAND].
eINLINE eF32 eRandomF()
{
    return (eF32)eRandom()/(eF32)eMAX_RAND;
}

eINLINE eF32 eRandomF(eU32 &seed)
{
	return (eF32)eRandom(seed)/(eF32)eMAX_RAND;
}

eINLINE eF32 eRandomF(eF32 min, eF32 max)
{
    return eRandomF()*(max-min)+min;
}

eINLINE eF32 eRandomF(eF32 min, eF32 max, eU32 &seed)
{
	return eRandomF(seed)*(max-min)+min;
}

/* boxmuller.c           Implements the Polar form of the Box-Muller
                         Transformation

                      (c) Copyright 1994, Everett F. Carter Jr.
                          Permission is granted by the author to use
			  this software for any application provided this
			  copyright notice is preserved.

*/
eINLINE eF32 eRandomFNormal(eF32 mean, eF32 standardDeviation, eU32 &seed)	/* normal random variate generator */
{
    // NOTE: code seems to be buggy due to static variables (non-deterministic)

	eF32 x1, x2, w, y1;
	static eF32 y2;
	static eS32 use_last = 0;

	if (use_last)		        /* use value from previous call */
	{
		y1 = y2;
		use_last = 0;
	}	else	{
		do {
			x1 = 2.0f * eRandomF(seed) - 1.0f;
			x2 = 2.0f * eRandomF(seed) - 1.0f;
			w = x1 * x1 + x2 * x2;
		} while ( (w >= 1.0) || (w <= eALMOST_ZERO));

		w = eSqrt( (-2.0f * eLn( w ) ) / w );
		y1 = x1 * w;
		y2 = x2 * w;
		use_last = 1;
	}

	return( mean + y1 * standardDeviation );
}

eINLINE eF32 eRandomFNormal(eU32 &seed)	/* normal random distributed number in (-1.0,1.0f)*/
{
	return eRandomFNormal(0.0f, 1.0f, seed);
}




eINLINE eF32 eSign(eF32 x)
{
    // Test exponent and mantissa bits: is input zero?
    if (((eInt &)x & 0x7fffffff) == 0)
    {
        return 0.0f;
    }

    // Mask sign bit in x, set it in r if necessary.
    eF32 r = 1.0f;
    (eInt &)r |= ((eInt &)x & 0x80000000);
    return r;
}

eFORCEINLINE void eUndenormalise(eF32 &sample)
{
#ifndef eUSE_ARM_NEON
    if (((*(eU32 *)&sample)&0x7f800000) == 0)
    {
        sample = 0.0f;
    }
#endif
}

eINLINE eInt eSign(eInt x)
{
    if (x == 0)
    {
        return 0;
    }
    else if (x < 0)
    {
        return -1;
    }

    return 1;
}

eINLINE eU32 eNextPowerOf2(eU32 x)
{
    x--;
    x = (x>>1) | x;
    x = (x>>2) | x;
    x = (x>>4) | x;
    x = (x>>8) | x;
    x = (x>>16) | x;
    x++;

    return x;
}

eINLINE eBool eIsPowerOf2(eU32 x)
{
    return !(x&(x-1));
}

// Templated functions for binary arithmetic.

template<class T> void eSetBit(T &t, eU32 index)
{
    eASSERT(index < sizeof(T)*8);
    t |= (1<<index);
}

template<class T> void eSetBit(T &t, eU32 index, eBool set)
{
    eASSERT(set == 0 || set == 1);

    eASSERT(index <sizeof(T)*8);
    t |= (set<<index);
}

template<class T> void eClearBit(T &t, eU32 index)
{
    eASSERT(index < sizeof(T)*8);
    t &= ~(1<<index);
}

template<class T> eBool eGetBit(T t, eU32 index)
{
    eASSERT(index < sizeof(T)*8);
    return ((t&(1<<index)) != 0);
}

template<class T> void eToggleBit(T &t, eU32 index)
{
    eASSERT(index < sizeof(T)*8);
    t ^= (1<<index);
}

eINLINE eU16 eLoword(eU32 x)
{
    return (eU16)(x&0xffff);
}

eINLINE eU16 eHiword(eU32 x)
{
    return (eU16)((x>>16)&0xffff);
}

eINLINE eU8 eLobyte(eU16 x)
{
    return (eU8)(x&0xff);
}

eINLINE eU8 eHibyte(eU16 x)
{
    return (eU8)((x>>8)&0xff);
}

eINLINE eU32 eMakeDword(eU16 lo, eU16 hi)
{
    return (((eU32)lo)|(((eU32)hi)<<16));
}

eINLINE eU16 eMakeWord(eU8 lo, eU8 hi)
{
    return (((eU8)lo)|(((eU8)hi)<<8));
}

// Templated inline functions.

template<class T> void eSwap(T &a, T &b)
{
    T c = a;
    a = b;
    b = c;
}

template<class T> T eMin(const T &a, const T &b)
{
    return (a < b) ? a : b;
}

template<class T> T eMax(const T &a, const T &b)
{
    return (a > b) ? a : b;
}

template<class T> T eClamp(const T &min, const T &x, const T &max)
{
    if (x < min)
    {
        return min;
    }
    else if (x > max)
    {
        return max;
    }

    return x;
}

template<class T> T eLerp(const T &a, const T &b, eF32 t)
{
    return a+(b-a)*t;
}

eINLINE eF32x2 eSIMDLoad2(eF32 v1, eF32 v2)
{
#ifdef eUSE_ARM_NEON
    const float32_t a[] = { (float32_t)v1, (float32_t)v2 };
    return vld1_f32(a);
#else
#ifdef eUSE_SSE
	return _mm_set_ps(v1, v2, 0.0f, 0.0f);
#else
	return eF32x2(v1, v2);
#endif
#endif
}

eINLINE void eSIMDStore(eF32x2 vec, eF32 *v1, eF32 *v2)
{
#ifdef eUSE_ARM_NEON
    float32_t a[2];
    vst1_f32(a, vec);
	*v1 = (eF32)a[0];
	*v2 = (eF32)a[1];
#else
#ifdef eUSE_SSE
	eF32A store_out[4];
	_mm_store_ps(store_out, vec);     
	*v1 = store_out[3];
	*v2 = store_out[2];
#else
	*v1 = vec.v[0];
	*v2 = vec.v[1];
#endif
#endif
}

eINLINE eF32x2 eSIMDSet2(const eF32 v)
{
#ifdef eUSE_ARM_NEON
    const float32_t a[] = { (float32_t)v, (float32_t)v };
    return vld1_f32(a);
#else
#ifdef eUSE_SSE
	return _mm_set1_ps(v);
#else
	return eF32x2(v);
#endif
#endif
}

eINLINE eF32x2 eSIMDMul(const eF32x2 v1, const eF32x2 v2)
{
#ifdef eUSE_ARM_NEON
	return vmul_f32(v1, v2);
#else
#ifdef eUSE_SSE
	return _mm_mul_ps(v1, v2);
#else
	return eF32x2(v1.v[0] * v2.v[0], v1.v[1] * v2.v[1]);
#endif
#endif
}

eINLINE eF32x2 eSIMDDiv(const eF32x2 v1, const eF32x2 v2)
{
#ifdef eUSE_ARM_NEON
	return vmul_f32(v1, vrecpe_f32(v2));
#else
#ifdef eUSE_SSE
	return _mm_div_ps(v1, v2);
#else
	return eF32x2(v1.v[0] / v2.v[0], v1.v[1] / v2.v[1]);
#endif
#endif
}

eINLINE eF32x2 eSIMDAdd(const eF32x2 v1, const eF32x2 v2)
{
#ifdef eUSE_ARM_NEON
	return vadd_f32(v1, v2);
#else
#ifdef eUSE_SSE
	return _mm_add_ps(v1, v2);
#else
	return eF32x2(v1.v[0] + v2.v[0], v1.v[1] + v2.v[1]);
#endif
#endif
}

eINLINE eF32x2 eSIMDSub(const eF32x2 v1, const eF32x2 v2)
{
#ifdef eUSE_ARM_NEON
	return vsub_f32(v1, v2);
#else
#ifdef eUSE_SSE
	return _mm_sub_ps(v1, v2);
#else
	return eF32x2(v1.v[0] - v2.v[0], v1.v[1] - v2.v[1]);
#endif
#endif
}

eINLINE eF32x2 eSIMDMulAdd(const eF32x2 add, const eF32x2 v1, const eF32x2 v2)
{
#ifdef eUSE_ARM_NEON
	return vmla_f32(add, v1, v2);
#else
#ifdef eUSE_SSE
	return _mm_add_ps(add, _mm_mul_ps(v1, v2));
#else
	return eF32x2(v1.v[0] * v2.v[0] + add.v[0], v1.v[1] * v2.v[1] + add.v[1]);
#endif
#endif
}

eINLINE eF32x2 eSIMDMulSub(const eF32x2 sub, const eF32x2 v1, const eF32x2 v2)
{
#ifdef eUSE_ARM_NEON
	return vmls_f32(sub, v1, v2);
#else
#ifdef eUSE_SSE
	return _mm_sub_ps(sub, _mm_mul_ps(v1, v2));
#else
	return eF32x2(sub.v[0] - v1.v[0] * v2.v[0], sub.v[1] - v1.v[1] * v2.v[1]);
#endif
#endif
}

eINLINE eF32x2 eSIMDMin(const eF32x2 v1, const eF32x2 v2)
{
#ifdef eUSE_ARM_NEON
	return vmin_f32(v1, v2);
#else
#ifdef eUSE_SSE
	return _mm_min_ps(v1, v2);
#else
	return eF32x2(
		v1.v[0] < v2.v[0] ? v1.v[0] : v2.v[0], 
		v1.v[1] < v2.v[1] ? v1.v[1] : v2.v[1]);
#endif
#endif
}

eINLINE eF32x2 eSIMDMax(const eF32x2 v1, const eF32x2 v2)
{
#ifdef eUSE_ARM_NEON
	return vmax_f32(v1, v2);
#else
#ifdef eUSE_SSE
	return _mm_max_ps(v1, v2);
#else
	return eF32x2(
		v1.v[0] > v2.v[0] ? v1.v[0] : v2.v[0], 
		v1.v[1] > v2.v[1] ? v1.v[1] : v2.v[1]);
#endif
#endif
}

eINLINE void eSIMDUndenormalise(eF32x2 &v)
{
#ifdef eUSE_ARM_NEON
	// not neccessary
#else
#ifdef eUSE_SSE
	// not neccessary
#else
	eUndenormalise(v.v[0]);
	eUndenormalise(v.v[1]);
#endif
#endif
}

#endif // RUNTIME_HPP