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

#ifndef TYPES_HPP
#define TYPES_HPP

#ifndef _WIN32
#include <stdint.h>
#endif

#ifdef eUSE_ARM_NEON
#include <arm_neon.h>
#endif

#ifdef _WIN32
#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#endif

// Token paste macros for concatenating strings
// for pre-processor usage.
#define eTOKENPASTE_DEF(x, y)   x##y
#define eTOKENPASTE(x, y)       eTOKENPASTE_DEF(x, y)

// Visual C++ specific stuff as macros.
#ifdef _WIN32
#define eFASTCALL           __fastcall
#define eFORCEINLINE        __forceinline
#define eINLINE             __inline
#define eNORETURN           __declspec(noreturn)
#define eALIGN16            __declspec(align(16))
#define eNAKED              __declspec(naked)
#define eCALLBACK           __stdcall
#define eCDECL              __cdecl
#else
#define eFASTCALL           
#define eFORCEINLINE        inline
#define eINLINE             inline
#define eNORETURN           
#define eALIGN16            
#define eNAKED              
#define eCALLBACK           
#endif

// Self defined types.
typedef unsigned char       eU8;
typedef signed char         eS8;
typedef unsigned short      eU16;
typedef short               eS16;
typedef unsigned int        eU32;
typedef int                 eS32;
typedef float               eF32;
#ifdef _WIN32
typedef unsigned __int64    eU64;
typedef signed __int64      eS64;
typedef eALIGN16 float      eF32A;
#else
typedef uint64_t            eU64;
typedef int64_t             eS64;
typedef eF32                eF32A;
#endif
typedef eALIGN16 float      eF32A4[4];
typedef double              eF64;
typedef int                 eInt;
typedef char                eChar;
typedef signed char         eBool;
typedef void *              ePtr;
typedef const void *        eConstPtr;
typedef eU32                eID;

#ifdef eUSE_ARM_NEON
typedef float32x2_t			eF32x2;	
typedef float32x4_t			eF32x4;	
#else
#ifdef eUSE_SSE
typedef __m128	     		eF32x2;				
typedef __m128	     		eF32x4;	
#else
typedef struct eF32x2
{
	eF32x2(eF32 v1, eF32 v2)
	{
		v[0] = v1;
		v[1] = v2;
	}

	eF32x2(eF32 v)
	{
		v[0] = v[1] = v;
	}

	eF32 v[2];
} eF32x2;

typedef struct eF32x4
{
	eF32x4(eF32 v1, eF32 v2, eF32 v3, eF32 v4)
	{
		v[0] = v1;
		v[1] = v2;
	}

	eF32x4(eF32 v)
	{
		v[0] = v[1] = v[2] = v[3] = v;
	}

	eF32 v[4];
} eF32x4;
#endif
#endif

// Numerical limits of above types. (DONT CHANGE 'EM INTO CONSTS !!! ITSA SIZE THANG !!!)
#define eU32_MAX        (~0)
#define eS32_MIN        (-2147483647-1)
#define eS32_MAX        (2147483647)
#define eU16_MAX        (65535)
#define eS16_MIN        (-32768)
#define eS16_MAX        (32767)
#define eU8_MAX         (255)
#define eS8_MIN         (-128)
#define eS8_MAX         (127)

#define eF32_MAX        (1E+37f)
#define eF32_MIN        (-eF32_MAX)

// Some constants as macros.
#define eTRUE               (eBool)(!0)
#define eFALSE              0
#define eNULL               0
#define eNOID               0

#endif // TYPES_HPP
