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

tfModMatrix::State::State()
{
    for(eU32 i=0;i<TF_MAX_MODULATIONS;i++)
        modulation[i] = 1.0f;
}

void tfModMatrix::State::noteOn(eF32 lfophase1, eF32 lfophase2)
{
    adsrState1.noteOn();
    adsrState2.noteOn();

    lfoState1.reset(lfophase1);
    lfoState2.reset(lfophase2);
}

void tfModMatrix::State::noteOff()
{
    adsrState1.noteOff();
    adsrState2.noteOff();
}

void tfModMatrix::State::panic()
{
    adsrState1.reset();
    adsrState2.reset();
}

eBool tfModMatrix::isActive(tfModMatrix::State *state)
{
    for (eU32 i=0; i<TF_MODMATRIXENTRIES; i++)
    {
        switch(state->entries[i].src)
        {
            case INPUT_ADSR1:
            {
                if (!state->adsrState1.isEnd())
                {
                    return eTRUE;
                }

                break;
            }

            case INPUT_ADSR2:
            {
                if (!state->adsrState2.isEnd())
                {
                    return eTRUE;
                }

                break;
            }
        }
    }

    return eFALSE;
}

eBool tfModMatrix::process(tfModMatrix::State *state, eF32 *params, eU32 len, eU32 sampleRate)
{
    eBool playing = eFALSE;
    
    eBool adsr1_done = eFALSE;
    eBool adsr2_done = eFALSE;
    eBool lfo1_done = eFALSE;
    eBool lfo2_done = eFALSE;

    for(eU32 i=0;i<TF_MODMATRIXENTRIES;i++)
    {
        eF32 mod = (params[TF_MM1_MOD + i*3] - 0.5f) * TF_MM_MODRANGE;
        mod *= mod;

        state->entries[i].src   = (Input)eFtoL(eRound(params[TF_MM1_SOURCE + i*3] * (INPUT_COUNT-1)));
        state->entries[i].dst   = (Output)eFtoL(eRound(params[TF_MM1_TARGET + i*3] * (OUTPUT_COUNT-1)));
        state->entries[i].mod   = mod; 
        state->entries[i].result= 1.0f; 

        switch(state->entries[i].src)
        {
            case INPUT_LFO1:
            {
                if (!lfo1_done)
                {
                    eF32 lfo1_freq = params[TF_LFO1_RATE];
                    eF32 lfo1_depth = params[TF_LFO1_DEPTH];
                    eF32 lfo1_shape = params[TF_LFO1_SHAPE];
                    eBool lfo1_sync = params[TF_LFO1_SYNC] > 0.5f;

                    m_lfo.setFrequency(&state->lfoState1, lfo1_freq, sampleRate, len);
                    state->values[INPUT_LFO1] = m_lfo.process(&state->lfoState1, lfo1_depth, lfo1_shape);

                    lfo1_done = eTRUE;
                }

                state->entries[i].result = state->entries[i].mod * state->values[INPUT_LFO1];

                break;
            }

            case INPUT_LFO2:
            {
                if (!lfo2_done)
                {
                    eF32 lfo2_freq = params[TF_LFO2_RATE];
                    eF32 lfo2_depth = params[TF_LFO2_DEPTH];
                    eF32 lfo2_shape = params[TF_LFO2_SHAPE];
                    eBool lfo2_sync = params[TF_LFO2_SYNC] > 0.5f;

                    m_lfo.setFrequency(&state->lfoState2, lfo2_freq, sampleRate, len);
                    state->values[INPUT_LFO2] = m_lfo.process(&state->lfoState2, lfo2_depth, lfo2_shape);

                    lfo2_done = eTRUE;
                }

                state->entries[i].result = state->entries[i].mod * state->values[INPUT_LFO2];

                break;
            }

            case INPUT_ADSR1:
            {
                if (!adsr1_done)
                {
                    eF32 adsr1_a = params[TF_ADSR1_ATTACK];
                    eF32 adsr1_d = params[TF_ADSR1_DECAY];
                    eF32 adsr1_s = params[TF_ADSR1_SUSTAIN];
                    eF32 adsr1_r = params[TF_ADSR1_RELEASE];
                    eF32 adsr1_sl = params[TF_ADSR1_SLOPE];

                    eF32 mmo_decay = get(state, OUTPUT_ADSR1_DECAY);
                    adsr1_d *= mmo_decay;

                    playing = !state->adsrState1.isEnd();
                    state->values[INPUT_ADSR1] = m_adsr.process(&state->adsrState1, len, adsr1_a, adsr1_d, adsr1_s, adsr1_r, adsr1_sl, sampleRate);

                    adsr1_done = eTRUE;
                }

                state->entries[i].result = state->entries[i].mod * state->values[INPUT_ADSR1];

                break;
            }

            case INPUT_ADSR2:
            {
                if (!adsr2_done)
                {
                    eF32 adsr2_a = params[TF_ADSR2_ATTACK];
                    eF32 adsr2_d = params[TF_ADSR2_DECAY];
                    eF32 adsr2_s = params[TF_ADSR2_SUSTAIN];
                    eF32 adsr2_r = params[TF_ADSR2_RELEASE];
                    eF32 adsr2_sl = params[TF_ADSR2_SLOPE];

                    eF32 mmo_decay = get(state, OUTPUT_ADSR2_DECAY);
                    adsr2_d *= mmo_decay;

                    playing = !state->adsrState2.isEnd();
                    state->values[INPUT_ADSR2] = m_adsr.process(&state->adsrState2, len, adsr2_a, adsr2_d, adsr2_s, adsr2_r, adsr2_sl, sampleRate);

                    adsr2_done = eTRUE;
                }

                state->entries[i].result = state->entries[i].mod * state->values[INPUT_ADSR2];
                break;
            }

#ifndef NO_MM_MODULATORS
            case INPUT_MOD1:
            {
                state->values[INPUT_MOD1] = state->modulation[0];
                state->entries[i].result = state->entries[i].mod * state->values[INPUT_MOD1];
                break;
            }

            case INPUT_MOD2:
            {
                state->values[INPUT_MOD2] = state->modulation[1];
                state->entries[i].result = state->entries[i].mod * state->values[INPUT_MOD2];
                break;
            }

            case INPUT_MOD3:
            {
                state->values[INPUT_MOD3] = state->modulation[2];
                state->entries[i].result = state->entries[i].mod * state->values[INPUT_MOD3];
                break;
            }

            case INPUT_MOD4:
            {
                state->values[INPUT_MOD4] = state->modulation[3];
                state->entries[i].result = state->entries[i].mod * state->values[INPUT_MOD4];
                break;
            }
#endif
        }
    }

    return playing;
}

eF32 tfModMatrix::get(tfModMatrix::State *state, Output output)
{
    eF32 value = 1.0f;

    for(eU32 i=0; i<TF_MODMATRIXENTRIES; i++)
    {
        if (state->entries[i].dst == output)
        {
            value *= state->entries[i].result;
        }
    }

    return value;
}