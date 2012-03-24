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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#endif

#include "types.hpp"
#include "runtime.hpp"

// Fix all unresolved external c-lib symbols.
// Also constructor and destructor calling of
// static and global variables is implemented
// here.
#if defined(ePLAYER) && defined(eRELEASE)
    typedef void (__cdecl *ePVFV)();

    #pragma data_seg(".CRT$XCA")
    ePVFV __xc_a[] = {eNULL};
    #pragma data_seg(".CRT$XCZ")
    ePVFV __xc_z[] = {eNULL};
    #pragma data_seg() // Reset data segment.
    #pragma comment(linker, "/merge:.crt=.data")

    static const eU32 eMAX_ATEXITS = 32;
    static ePVFV g_atExitList[eMAX_ATEXITS];

    static void initTerm(ePVFV *pfbegin, ePVFV *pfend)
    {
        eASSERT(pfbegin != eNULL);
        eASSERT(pfend != eNULL);

        while (pfbegin < pfend)
        {
            if (*pfbegin != eNULL)
            {
                (**pfbegin)();
            }

            pfbegin++;
        }
    }

    static void initAtExit()
    {
        eMemSet(g_atExitList, 0, sizeof(g_atExitList));
    }

    static void doAtExit()
    {
        initTerm(g_atExitList, g_atExitList+eMAX_ATEXITS);
    }

    eInt eCDECL atexit(ePVFV func)
    {
        eASSERT(func != eNULL);

        // Get next free entry in atexist list.
        eU32 index = 0;
        while (g_atExitList[index++]);
        eASSERT(index < eMAX_ATEXITS);

        // Put function pointer to destructor there.
        if (index < eMAX_ATEXITS)
        {
            g_atExitList[index] = func;
            return 0;
        }

        return -1;
    }

    eInt eCDECL _purecall()
    {
        eASSERT(eFALSE);
        return 0;
    }

    extern "C"
    {
        eInt _fltused = 0;

        void eCDECL _chkstk(void)
        {
        };
/*
// these cause errors, i.e. (eU32)2.0f == 0
        eInt eCDECL _ftol2(eF32 val)
        {
            return eFtoL(val);
        };

        eInt eCDECL _ftol2_sse(eF32 val)
        {
            return eFtoL(val);
        };
*/
    };


#endif

#ifdef eRELEASE
ePtr __cdecl operator new [] (eU32 size)
{
    return HeapAlloc(GetProcessHeap(), 0, size);
}

void __cdecl operator delete [] (ePtr p)
{
    HeapFree(GetProcessHeap(), 0, p);
}

ePtr __cdecl operator new(eU32 size)
{
    return HeapAlloc(GetProcessHeap(), 0, size);
}

void __cdecl operator delete(ePtr p)
{
    HeapFree(GetProcessHeap(), 0, p);
}
#endif

ePtr	eMemAllocAlignedAndZero(eU32 size, size_t alignment) {
	void *p1 ,*p2; // basic pointer needed for computation.
	/* We need to use malloc provided by C. First we need to allocate memory
	of size bytes + alignment + sizeof(size_t) . We need 'bytes' because
	user requested it. We need to add 'alignment' because malloc can give us
	any address and we need to find multiple of 'alignment', so at maximum multiple
	of alignment will be 'alignment' bytes away from any location. We need
	'sizeof(size_t)' for implementing 'aligned_free', since we are returning modified
	memory pointer, not given by malloc ,to the user, we must free the memory
	allocated by malloc not anything else. So I am storing address given by malloc just above
	pointer returning to user. Thats why I need extra space to store that address.
	Then I am checking for error returned by malloc, if it returns NULL then
	aligned_malloc will fail and return NULL.
	*/
	p1 =(void *) new char[size + alignment + sizeof(size_t)];
	eMemSet(p1, 0, size + alignment + sizeof(size_t));

	/* Next step is to find aligned memory address multiples of alignment.
	By using basic formule I am finding next address after p1 which is
	multiple of alignment.I am storing new address in p2.
	*/
	size_t addr=(size_t)p1 + alignment + sizeof(size_t);
	p2=(void *)(addr - (addr % alignment));

	/* Final step, I am storing the address returned by malloc 'p1' just "size_t"
	bytes above p2, which will be useful while calling aligned_free.
	*/
	*((size_t *)p2-1)=(size_t)p1;

	return p2;
}

void	eFreeAligned(ePtr ptr) {
	/* Find the address stored by aligned_malloc ,"size_t" bytes above the
	current pointer then free it using normal free routine provided by C.
	*/
#ifdef _WIN32
	if(ptr != eNULL)
		delete ((void *)(*((size_t *) ptr-1)));
#endif
}

void	eFreeAlignedArray(ePtr ptr) {
	/* Find the address stored by aligned_malloc ,"size_t" bytes above the
	current pointer then free it using normal free routine provided by C.
	*/
#ifdef _WIN32
	if(ptr != eNULL)
		delete [] ((void *)(*((size_t *) ptr-1)));
#endif
}

// Functions to check if the underlying CPU supports
// the needed SIMD instruction sets (MMX and SSE).

static eBool isSseSupported()
{
    eBool result = eFALSE;
#ifdef _WIN32
    __asm
    {
        mov     eax, 1
        cpuid
        test    edx, 0x02000000
        jz      nosse
        mov     [result], 1
nosse:
    }
#else

#endif
    return result;
}

static eBool isMmxSupported()
{
    eBool result = eFALSE;
#ifdef _WIN32
    __asm
    {
        mov     eax, 1
        cpuid
        test    edx, 0x00800000
        jz      nommx
        mov     [result], 1
nommx:
    }
#else
#endif
    return result;
}

eBool eVerifyInstructionSets()
{
    eBool sse = eTRUE;
    eBool mmx = eTRUE;

#ifdef eUSE_SSE
    sse = isSseSupported();
#endif

#ifdef eUSE_MMX
    mmx = isMmxSupported();
#endif

    if (!sse || !mmx)
    {
        eShowError("SIMD instruction sets are not available on CPU!");
        return eFALSE;
    }

    return eTRUE;
}

void eSetSSEFlushToZeroMode()
{
#ifdef eUSE_SSE
    eU32 mxcsr;
    __asm STMXCSR mxcsr;
    eSetBit<eU32>(mxcsr, 15, eTRUE); // flush to zero bit 15
    // eSetBit<eU32>(mxcsr, 6, eTRUE); // denormals are zero bit 6
    __asm LDMXCSR mxcsr;
#endif
}

#ifdef eUSE_SSE
eBool eSSECmpTrue(eF32x2 value)
{
    eF32A store[4];
    _mm_store_ps(store, value);
    return store[0] != 0.0f;
}
#endif

void eInitGlobalsStatics()
{
#if defined(ePLAYER) && defined(eRELEASE)
    initAtExit();
    initTerm(__xc_a, __xc_z); 
#endif
}

void eFreeGlobalsStatics()
{
#if defined(ePLAYER) && defined(eRELEASE)
    doAtExit();
#endif
}

// Functions are available in release mode too, but
// do nothing, because memory tracker is disabled.

void eMemTrackerStart()
{
#ifdef eDEBUG
    const eInt curFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    _CrtSetDbgFlag(curFlags | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
#endif
}

void eMemTrackerStop()
{
#ifdef eDEBUG
    const eInt oldFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    _CrtSetDbgFlag(oldFlags & ~(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF));
#endif
}

#ifdef eDEBUG
// Returns wether or not the user pressed the
// "try again" button.
eBool eShowAssertion(const eChar *expr, const eChar *file, eU32 line)
{
    eASSERT(expr != eNULL);
    eASSERT(file != eNULL);

    eChar text[1024], program[256];

    GetModuleFileName(GetModuleHandle(eNULL), program, 256);

    eStrCopy(text, "Debug assertion failed!\n\nProgram: ");
    eStrAppend(text, program);
    eStrAppend(text, "\nFile: ");
    eStrAppend(text, file);
    eStrAppend(text, "\nLine: ");
    eStrAppend(text, eIntToStr(line));
    eStrAppend(text, "\n\nExpression: ");
    eStrAppend(text, expr);
    eStrAppend(text, "\n\nPress retry to debug the application, cancel to stop execution\n"
                     "and continue to ignore assertion.");

    const eInt button = MessageBox(eNULL,
                                   text,
                                   "Enigma 3 - Assertion",
                                   MB_SYSTEMMODAL | MB_ICONERROR | MB_CANCELTRYCONTINUE);

    if (button == IDTRYAGAIN)
    {
        return eTRUE;
    }
    else if (button == IDCANCEL)
    {
        // Kill our application.
        ExitProcess(-1);
    }

    return eFALSE;
}
#endif

void eShowError(const eChar *error)
{
    eASSERT(error != eNULL);
#ifdef _WIN32
    MessageBox(eNULL, error, "Enigma 3 - Error", MB_SYSTEMMODAL | MB_ICONERROR);
#else
    printf("ERROR: %s\n", error);
#endif
}

// Bigger, not inlineable functions.

ePtr eMemRealloc(ePtr ptr, eU32 oldLength, eU32 newLength)
{
    // No reallocation necessary. Old array is large enough.
    if (newLength <= oldLength && ptr != eNULL)
    {
        return ptr;
    }

    ePtr newPtr = new eU8[newLength];
    eASSERT(newPtr != eNULL);

    // Source is null, just do a normal allocation.
    if (ptr != eNULL)
    {
        eU8 *bptr = (eU8 *)ptr;
        eMemCopy(newPtr, bptr, oldLength);
        eSAFE_DELETE_ARRAY(bptr);
    }

    return newPtr;
}

void eMemSet(ePtr dst, eU8 value, eU32 count)
{
    eASSERT(dst != eNULL);
#ifdef _WIN32
    __asm
    {
        mov     eax, dword ptr [value]
        mov     ecx, dword ptr [count]
        mov     edi, dword ptr [dst]
        rep     stosb
    }
#else
    memset(dst, value, count);
#endif
}

void eMemCopy(ePtr dst, eConstPtr src, eU32 count)
{
    eASSERT(dst != eNULL);
    eASSERT(src != eNULL);
#ifdef _WIN32
#ifdef eUSE_MMX
    __asm
    {
        mov     edi, dword ptr [dst]
        mov     esi, dword ptr [src]
        mov     ecx, dword ptr [count]

        // Calculate loop iteration count.
        mov     eax, ecx
        shr     ecx, 6
        mov     edx, ecx
        shl     edx, 6
        sub     eax, edx
        cmp     ecx, 0
        je      done

copyloop:
        movq    mm0, [esi]
        movq    mm1, [esi+8]
        movq    mm2, [esi+16]
        movq    mm3, [esi+24]
        movq    mm4, [esi+32]
        movq    mm5, [esi+40]
        movq    mm6, [esi+48]
        movq    mm7, [esi+56]

        movq    [edi], mm0
        movq    [edi+8], mm1
        movq    [edi+16], mm2
        movq    [edi+24], mm3
        movq    [edi+32], mm4
        movq    [edi+40], mm5
        movq    [edi+48], mm6
        movq    [edi+56], mm7

        add     esi, 64
        add     edi, 64
        dec     ecx
        jnz     copyloop

        // Copy missing bytes.
done:   mov     ecx, eax
        rep     movsb
        emms
    }
#else
    eU8 *pd = (eU8 *)dst;
    const eU8 *ps = (eU8 *)src;

    while (count--)
    {
        *pd++ = *ps++;
    }
#endif
#else
    memcpy(dst, src, count);
#endif
}

void eMemMove(ePtr dst, eConstPtr src, eU32 count)
{
    eASSERT(src != eNULL);
    eASSERT(dst != eNULL);

    const eU8 *psrc = (const eU8 *)src;
    eU8 *pdst = (eU8 *)dst;

    if (dst <= src || pdst >= psrc+count)
    {
        // Non-overlapping buffers, so copy from
        // lower addresses to higher addresses.
        while (count--)
        {
            *pdst++ = *psrc++;
        }
    }
    else
    {
        // Overlapping buffers, so copy from
        // higher addresses to lower addresses.
        pdst = pdst+count-1;
        psrc = psrc+count-1;

        while (count--)
        {
            *pdst-- = *psrc--;
        }
    }
}

eBool eMemEqual(eConstPtr mem0, eConstPtr mem1, eU32 count)
{
    eASSERT(mem0 != eNULL);
    eASSERT(mem1 != eNULL);

    const eU8 *ptr0 = (eU8 *)mem0;
    const eU8 *ptr1 = (eU8 *)mem1;

    for (eU32 i=0; i<count; i++)
    {
        if (ptr0[i] != ptr1[i])
        {
            return eFALSE;
        }
    }

    return eTRUE;
}

void eStrClear(eChar *str)
{
    eASSERT(str != eNULL);
    str[0] = '\0';
}

void eStrCopy(eChar *dst, const eChar *src)
{
    eASSERT(dst != eNULL);
    eASSERT(src != eNULL);

    while (*dst++ = *src++);
}

void eStrNCopy(eChar *dst, const eChar *src, eU32 count)
{
    eASSERT(dst != eNULL);
    eASSERT(src != eNULL);

    // Copy string characters.
    while (count && (*dst++ = *src++))
    {
        count--;
    }

    // Pad out with zeros.
    if (count)
    {
        eMemSet(dst, '\0', count-1);
    }
}

eChar * eStrClone(const eChar *str)
{
    eChar *clone = new eChar[eStrLength(str)+1];
    eASSERT(clone != eNULL);
    eStrCopy(clone, str);
    return clone;
}

eU32 eStrLength(const eChar *str)
{
    eASSERT(str != eNULL);

    const eChar *eos = str;
    while (*eos++);
    return (eU32)(eos-str-1);
}

eChar * eStrAppend(eChar *dst, const eChar *src)
{
    eASSERT(dst != eNULL);
    eASSERT(src != eNULL);

    // Find end of source string.
    eChar *pd = dst;

    while (*pd)
    {
        pd++;
    }

    eStrCopy(pd, src);
    return dst;
}

// Splits the a string at token into a left and
// a right string.
void eStrSplit(const eChar *str, eChar token, eChar *left, eChar *right)
{

}

// Compares two strings and returns an integer to
// indicate whether the first is less than the second,
// the two are equal, or whether the first is greater
// than the second. Comparison is done byte by byte on
// an unsigned basis, which is to say that null (0) is
// less than any other character (1-255).
//
// Returns -1 if first string < second string.
// Returns  0 if first string = second string.
// Returns +1 if first string > second string.
eInt eStrCompare(const eChar *str0, const eChar *str1)
{
    eInt result = 0;

    while (!(result = *(eU8 *)str0-*(eU8 *)str1) && *str1)
    {
        str0++;
        str1++;
    }

    if (result < 0)
    {
        result = -1;
    }
    else if (result > 0)
    {
        result = 1;
    }

    return result;
}

eChar * eStrUpper(eChar *str)
{
    eASSERT(str != eNULL);

    const eU32 strLen = eStrLength(str);

    for (eU32 i=0; i<strLen; i++)
    {
        eChar &c = str[i];

        if (c >= 'a' && c <= 'z')
        {
            c -= 32;
        }
    }

    return str;
}

eChar * eIntToStr(eInt value)
{
    // Remember if integer is negative and
    // if it is, make it positive.
    const eBool negative = (value < 0);

    if (negative)
    {
        value = -value;
    }

    // 12 spaces are enough for 32-bit decimal
    // (10 digits + 1 null terminator byte +
    // eventually a sign character).
    static eChar result[12];

    eChar *cp = result+sizeof(result)-1;
    *cp = '\0';

    do
    {
        *(--cp) = value%10+'0';
        value /= 10;
    }
    while (value > 0);

    // Prepend negative sign character.
    if (negative)
    {
        *(--cp) = '-';
    }

    return cp; 
}

eInt eStrToInt(const eChar *str)
{
    eASSERT(str != eNULL);
    eASSERT(eStrLength(str) > 0);

    eChar c;
    eInt value = 0;
	
	const eChar *strIter = str;
    while ((c = *strIter++) != '\0')
        if (c >= '0' && c <= '9') {
            const eU8 digit = c-'0';
            value = value*10+digit;
        }

    // Make integer negative.
    if (str[0] == '-') 
        value = -value;

    return value;
}

// Seed value of the random number generator.
static eU32 g_seed = 1;
static eU32 g_threadId = 0;

void eRandomize(eU32 seed)
{
    // Seed may not be 0.
    g_seed = seed+1;

    // If seed is zero because of an
    // overflow set seed to 1.
    if (g_seed == 0)
    {
        g_seed = 1;
    }
}

eU32 eRandomSeed()
{
	return GetTickCount();
}

// Park-Miller random number generation (so called
// "minimal standard" random generator).
// Random numbers are in range 1,..,m-1.
// Calculates (a*16807 mod 0x7fffffff).
eU32 eRandom()
{
    return eRandom(g_seed);
}

eU32 eRandom(eU32 &seed)
{
	eU32 lo = 16807*(seed&0xffff);
	eU32 hi = 16807*(seed>>16);
	lo += (hi&0x7fff)<<16;
	hi >>= 15;
	lo += hi;
	lo = (lo&0x7FFFFFFF)+(lo>>31);
	seed = lo;
	return seed;
}

// This code was taken from www.xyzw.de, the page
// of Dirk 'chaos/fr' Ohlerich. His comment was:
// "Faster pow based on code by Agner Fog."
static eF64 pow64(eF64 base, eF64 exp)
{
#ifdef _WIN32
    __asm
    {
        fld     qword ptr [exp]
        fld     qword ptr [base]

        ftst
        fstsw   ax
        sahf
        jz      valuezero

        __emit  0xd9 // Fake fyl2x instruction,
        __emit  0xf1 // because compiler crashes.
        fist    dword ptr [base]
        sub     esp, 12
        mov     dword ptr [esp], 0
        mov     dword ptr [esp+4], 0x80000000
        fisub   dword ptr [base]
        mov     eax, dword ptr [base]
        add     eax, 0x3fff
        mov     [esp+8], eax
        jle     underflow
        cmp     eax, 0x8000
        jge     overflow
        f2xm1
        fld1
        fadd
        fld     tbyte ptr [esp]
        add     esp, 12
        fmul
        jmp     finished

underflow:
        fstp    st
        fldz
        add     esp, 12
        jmp     finished

overflow:
        push    0x7f800000
        fstp    st
        fld     dword ptr [esp]
        add     esp, 16
        jmp     finished

valuezero:
        fstp    st(1)

finished:
    }
#else
    return pow(base, exp);
#endif
}

// This low precision floating point power function
// has to use the high precision version above, else
// wrong values are returned.
eF32 ePow(eF32 base, eF32 exp)
{
    return (eF32)pow64((eF64)base, (eF64)exp);
}

eF32 eSinH(eF32 x)
{
#ifdef _WIN32
    eF32 temp;

    __asm
    {
        fld     dword ptr [x]
        fchs
        fldl2e
        fmulp   st(1), st(0)
        fst     st(1)
        frndint
        fsub    st(1), st(0)
        fxch
        f2xm1
        fld1
        fadd
        fscale
        fstp    st(1)
        fstp    dword ptr [temp]
        fld     dword ptr [x]
        fldl2e
        fmulp   st(1), st(0)
        fst     st(1)
        frndint
        fsub    st(1), st(0)
        fxch
        f2xm1
        fld1
        fadd
        fscale
        fstp    st(1)
        fld     dword ptr [temp]
        fsub
        fld1
        fld1
        fadd
        fdiv
    }
#else
    return sinhf(x);
#endif
}

eF32 eTanH(eF32 x)
{
#ifdef _WIN32
    __asm
    {
        fld     dword ptr [x]
        fld     st(0)
        fadd
        fldl2e
        fmulp   st(1), st(0)
        fst     st(1)
        frndint
        fsub    st(1), st(0)
        fxch
        f2xm1
        fld1
        fadd
        fscale
        fstp    st(1)
        fld1
        fadd
        fld1
        fld1
        fadd
        fdivr
        fld1
        fsubr
    }
#else
    return tanhf(x);
#endif
}

// Arcus sine with -1 <= x <= 1.
eF32 eASin(eF32 x)
{
#ifdef _WIN32
    __asm
    {
        fld     dword ptr [x]
        fld     st(0)
        fmul
        fld     st(0)
        fld1
        fsubr
        fdiv
        fsqrt
        fld1
        fpatan
    }
#else
    return asinf(x);
#endif
}

// Arcus cosine with -1 <= x <= 1.
eF32 eACos(eF32 x)
{
#ifdef _WIN32
    __asm
    {
        fld     [x]
        fld     st(0)
        fld     st(0)
        fmul
        fld1
        fsubr
        fsqrt
        fxch
        fpatan
    }
#else
    return acosf(x);
#endif
}

// Returns e^x (exponential function).
eF32 eExp(eF32 x)
{
#ifdef _WIN32
    __asm
    {
        fld     dword ptr [x]
        fldl2e
        fmulp   st(1), st
        fld1
        fld     st(1)
        fprem
        f2xm1
        faddp   st(1), st
        fscale
        fstp    st(1)
    }
#else
    return expf(x);
#endif
}

eInt eFloor(eF32 x)
{
#ifdef _WIN32
    static eInt holder, setter, result;

    __asm
    {
        fld     [x]
        fnstcw  [holder]
        movzx   eax, [holder]
        and     eax, 0xfffff3ff
        or      eax, 0x00000400
        mov     [setter], eax
        fldcw   [setter]
        fistp   [result]
        fldcw   [holder]
    }

    return result;
#else
    return floorf(x);
#endif
}

eInt eCeil(eF32 x)
{
#ifdef _WIN32
    static eInt holder, setter, result;

    __asm
    {
        fld     [x]
        fnstcw  [holder]
        movzx   eax, [holder]
        and     eax, 0xfffff3ff
        or      eax, 0x00000800
        mov     [setter], eax
        fldcw   [setter]
        fistp   [result]
        fldcw   [holder]
    }

    return result; 
#else
    return ceilf(x);
#endif
}
