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

#ifndef TF_FUNCTIONS_HPP
#define TF_FUNCTIONS_HPP

void eSignalMix(eSignal **master, eSignal **in, eU32 length, eF32 volume);
void eSignalToS16(eSignal **sig, eS16 *out, const eF32 gain, eU32 length);
void eSignalToPeak(eSignal **sig, eF32 *peak_left, eF32 *peak_right, eU32 length);
void eDownsampleMix(eSignal **master, eSignal **in, eU32 length, eU32 oversamplingCount);

#ifdef eDEBUG
void eSignalDebugWrite(eSignal **sig, eF32 volume, eChar *filename);
#endif

#ifdef eHWSYNTH
void eSignalDebugWritePeak(eSignal **sig, const eChar *msg);
#endif

#endif