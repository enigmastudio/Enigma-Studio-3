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

static eFORCEINLINE eF32 gen_whitenoise()
{
    const static eInt q = 15;
    const static eF32 c1 = (1 << q) - 1;
    const static eF32 c2 = (eF32)(eFtoL(c1 / 3)) + 1;
    const static eF32 c3 = 1.f / c1;

    eF32 random = eRandomF(0.0f, 1.0f);
    return (2.f * ((random * c2) + (random * c2) + (random * c2)) - 3.f * (c2 - 1.f)) * c3;
}

eBool tfNoise::m_noiseReady = eFALSE;
eF32 tfNoise::m_noiseTable[];

void tfNoise::State::reset()
{
	eU32 seed = eRandomSeed();
	offset1 = eRandom(0, TF_NOISETABLESIZE/2, seed);
	offset2 = eRandom(0, TF_NOISETABLESIZE/2, seed);

	filterOn = eFALSE;

	filterStateHP.reset();
	filterStateLP.reset();
}

tfNoise::tfNoise()
{
    generateNoiseTable();
}

void tfNoise::update(tfNoise::State *state, eF32 *params, eU32 sampleRate)
{
    eF32 bw = params[TF_NOISE_BW];

    if (bw < 0.99f)
    {
        state->filterOn = eTRUE;

        m_filter.update(&state->filterStateHP, eNULL, eNULL, params, tfFilter::FILTER_NOISE_HIGHPASS, sampleRate);
        m_filter.update(&state->filterStateLP, eNULL, eNULL, params, tfFilter::FILTER_NOISE_LOWPASS, sampleRate);
    }
    else
    {
        state->filterOn = eFALSE;
    }
}

eBool tfNoise::process(State *state, 
    tfModMatrix *modMatrix, 
    tfModMatrix::State *modMatrixState, 
    eF32 *params, 
    eSignal **signal, 
    eU32 len,
    eF32 velocity)
{
    eF32 noise = params[TF_NOISE_AMOUNT];

    noise *= velocity;
    noise *= modMatrix->get(modMatrixState, tfModMatrix::OUTPUT_NOISE_AMOUNT);

    if (noise > 0.01f)
    {
        eSignal *signal1 = signal[0];
        eSignal *signal2 = signal[1];

        eU32 len2 = len;
        while(len2--)
        {
            *signal1++ += m_noiseTable[state->offset1++] * noise;
            *signal2++ += m_noiseTable[state->offset2++] * noise;

            if (state->offset1 >= TF_NOISETABLESIZE)
                state->offset1 = 0;

            if (state->offset2 >= TF_NOISETABLESIZE)
                state->offset2 = 0;
        }

        if (state->filterOn)
        {
            m_filter.process(&state->filterStateLP, signal, len);
            m_filter.process(&state->filterStateHP, signal, len);
        }

        return eTRUE;
    }

    return eFALSE;
}

void tfNoise::generateNoiseTable()
{
    if (m_noiseReady)
        return;

    eRandomize(1);

    for (eU32 i=0;i<TF_NOISETABLESIZE;i++)
    {
        m_noiseTable[i] = gen_whitenoise();
    }

    m_noiseReady = eTRUE;
}