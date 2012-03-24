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

eF32 tfADSR::process(tfADSR::State *state, eU32 len, eF32 a, eF32 d, eF32 s, eF32 r, eF32 slope, eU32 sampleRate)
{
    eF32 attack,decay,sustain,release;
    eF32 scale = 0.00050f * (sampleRate / 44100.0f) * len;

    a = ePow(a, 3);
    d = ePow(d, 3);
    r = ePow(r, 3);

    a = eMax(0.000000001f, a);
    attack = -eLog(a * .94f) * scale;
        
    if (d <= 0)
        decay = -1.0f;
    else
        decay = eLog(d * .94f) * 0.25f * scale;

    sustain = eMax(s, 0.0f);

    r = eMax(r, 0.000000001f);
    release = eLog(r * .94f) * 0.25f * scale;
    
    eF32 volume = state->volume;
    switch (state->state)
    {
        case ADSR_STATE_ATTACK:
            volume += attack;
            if (volume >= 1.0f)
            {
                volume = 1.0f;
                state->state = ADSR_STATE_DECAY;
            }
            break;
        case ADSR_STATE_DECAY:
            {
                eF32 diff = 0.01f + (volume - sustain);
                eF32 range = 1.0f - sustain;
                eF32 pos = diff / range;
                eF32 slope_f = ePow(pos, slope);

                volume += decay * slope_f;

                if (volume <= sustain)
                {
                    volume = sustain;
                    state->state = ADSR_STATE_SUSTAIN;
                }
            }
            break;
        case ADSR_STATE_SUSTAIN:
            if (volume < sustain)
            {
                volume -= decay;
                if (volume > sustain)
                {
                    volume = sustain;
                }
            }
            else if (volume > sustain)
            {
                volume += decay;
                if (volume < sustain)
                {
                    volume = sustain;
                }
            }
            break;
        case ADSR_STATE_RELEASE:
            {
                eF32 slope_f = ePow(volume, slope);

                volume += release * slope_f;

                if (volume <= 0.001f)
                {
                    volume = 0.0f;
                    state->state = ADSR_STATE_FINISHED;
                }
            }
            break;
        case ADSR_STATE_FINISHED:
            break;
    }

    state->volume = volume;

    return volume;
}