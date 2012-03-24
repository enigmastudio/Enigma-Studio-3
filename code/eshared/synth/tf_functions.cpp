/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   This file is part of
 *
 *    ------  /   /  /\   /  /---  -----  /  ----  /   /
 *       /   /   /  /  \ /  /---   -/-   /   \--  /---/   version 3
 *      /    \---  /    /  /---    /    /  ----/ /   /.
 *
 *       t i n y   m u s i c   s y n t h e s i z e r
 *
 *   Copyright © 2003-2010 Brain Control, all rights reserved.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "../system/system.hpp"
#include "../math/math.hpp"
#include "tunefish3.hpp"

void eSignalMix(eSignal **master, eSignal **in, eU32 length, eF32 volume)
{
    eSignal *signal1 = master[0];
    eSignal *signal2 = master[1];
    eSignal *mix1 = in[0];
    eSignal *mix2 = in[1];

    if (volume <= 0.5f)
    {
        volume *= 2.0f;
        volume *= volume;
    }
    else
    {
        volume -= 0.5f;
        volume *= 20.0f;
        volume += 1.0f;
    }

	eF32x2 const_vol = eSIMDSet2(volume);

    while(length--)
	{
        eF32x2 val = eSIMDAdd(
						eSIMDMul(
							eSIMDLoad2(*mix1++, *mix2++), 
							const_vol
						), 
						eSIMDLoad2(*signal1, *signal2)
					);
					
        eSIMDStore(val, signal1, signal2);
		
        signal1++;
		signal2++;
    }
}

void eSignalToS16(eSignal **sig, eS16 *out, const eF32 gain, eU32 length)
{
    eS16 *dest = out;
	eSignal *srcLeft = sig[0];
	eSignal *srcRight = sig[1];

	eF32x2 const_gain = eSIMDSet2(gain);
    eF32x2 const_min = eSIMDSet2(-32768.0f);
    eF32x2 const_max = eSIMDSet2(32767.0f);
    
    while(length--)
	{
        eF32x2 val = eSIMDLoad2(*srcLeft++, *srcRight++);
        val = eSIMDMin(eSIMDMax(eSIMDMul(val, const_gain), const_min), const_max);
		eF32 store_val[2];
		eSIMDStore(val, &store_val[0], &store_val[1]);
        *dest++ = eFtoL(store_val[0]);
		*dest++ = eFtoL(store_val[1]);
    }
}

void eSignalToPeak(eSignal **sig, eF32 *peak_left, eF32 *peak_right, eU32 length)
{
	eSignal *srcLeft = sig[0];
	eSignal *srcRight = sig[1];

    eF32x2 peak = eSIMDSet2(0.0f);
    eF32x2 div = eSIMDSet2((eF32)length);

    while(length--)
	{
        eF32 left = *srcLeft++;
        eF32 right = *srcRight++;
        eU32 left_i = *((eU32*)&left);
        eU32 right_i = *((eU32*)&right);
        left_i &= 0x7fffffff;
        right_i &= 0x7fffffff;
        left = *((eF32*)&left_i);
        right = *((eF32*)&right_i);

        peak = eSIMDAdd(peak, eSIMDLoad2(left, right));
    }

    peak = eSIMDDiv(peak, div);

	eSIMDStore(peak, peak_left, peak_right);
}

void eDownsampleMix(eSignal **master, eSignal **in, eU32 length, eU32 oversamplingCount)
{
	eSignal *signal1 = master[0];
    eSignal *signal2 = master[1];
    eSignal *mix1 = in[0];
    eSignal *mix2 = in[1];

    while(length--)
    {
		eU32 i = oversamplingCount;
		eF32 v1 = 0.0f;
		eF32 v2 = 0.0f;

		while(i--)
		{
			v1 += *mix1++;
			v2 += *mix2++;
		}

		*signal1++ += v1 / oversamplingCount;
		*signal2++ += v2 / oversamplingCount;
    }
}

#ifdef eDEBUG
#include <stdio.h>
void eSignalDebugWrite(eSignal **sig, eF32 volume, eChar *filename)
{
    eS16 sig_int[TF_BLOCKSIZE*2];
    eSignalToS16(sig, sig_int, 20000.0f*volume, TF_BLOCKSIZE);
    FILE *out = fopen(filename, "ab");
    fwrite(sig_int, TF_BLOCKSIZE*2*sizeof(eS16), 1, out);
    fclose(out);
}
#endif

#ifdef eHWSYNTH
#include <stdio.h>
void eSignalDebugWritePeak(eSignal **sig, const eChar *msg)
{
/*
    eF32 peak = 0.0f;
    eU32 len = TF_BLOCKSIZE;
    eSignal *s1 = sig[0];
    eSignal *s2 = sig[1];
    
    while(len--)
    {
	eF32 v1 = *s1++;
	eF32 v2 = *s2++;
	
	v1 = eAbs(v1);
	v2 = eAbs(v2);
	
	peak += v1;
	peak += v2;
    }
    
    peak /= TF_BLOCKSIZE*2;
    
    printf("%s - %f\n", msg, peak);
    */
}
#endif



