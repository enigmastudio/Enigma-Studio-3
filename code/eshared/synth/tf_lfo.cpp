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

eBool tfLFO::m_noiseReady = eFALSE;
eF32 tfLFO::m_noiseTable[];

tfLFO::tfLFO()
{
    generateNoiseTable();
}

void tfLFO::setFrequency(tfLFO::State *state, eF32 f, eU32 sampleRate, eU32 blocksize)
{
    state->freq = (f * f) / sampleRate * blocksize * 50.0f;
}

eF32 tfLFO::process(tfLFO::State *state, eF32 depth, eF32 shape)
{
    eF32 result = 1.0f;

    depth *= depth;

    if (shape < 0.2f)  // sine
    {
        result = (eSin(state->phase) + 1.0f / 2.0f);
    }
    else if (shape < 0.4f) // sawtooth down
    {
        result = (eMod(state->phase, (ePI*2)) / ePI);
    }
    else if (shape < 0.6f) // sawtooth up
    {
		result = 1.0f - (eMod(state->phase, (ePI*2)) / ePI);
    }
	else if (shape < 0.8f) // pulse
	{
		result = (state->phase < ePI) ? 1.0f : 0.0f;
	}
	else // noise
	{
		result = m_noiseTable[eFtoL(state->phase / (ePI*2) * TF_LFONOISETABLESIZE)];
	}

	result = (result * depth) + (1.0f - depth);

    state->phase += state->freq;
    if (state->phase > (ePI*2))
        state->phase -= ePI*2;

    return result;
}

void tfLFO::generateNoiseTable()
{
    if (m_noiseReady)
        return;

    eRandomize(1);

    for (eU32 i=0;i<TF_LFONOISETABLESIZE;i++)
    {
        m_noiseTable[i] = eRandomF(0.0f, 1.0f);
    }

    m_noiseReady = eTRUE;
}