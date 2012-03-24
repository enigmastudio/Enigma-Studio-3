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

#ifndef TF_MODMATRIX_HPP
#define TF_MODMATRIX_HPP

class tfModMatrix
{
public:
    enum Input
    {
        INPUT_NONE,
        INPUT_LFO1,
        INPUT_LFO2,
        INPUT_ADSR1,
        INPUT_ADSR2,
        INPUT_MOD1,
        INPUT_MOD2,
        INPUT_MOD3,
        INPUT_MOD4,
        INPUT_WHEEL,

        INPUT_COUNT
    };

    enum Output
    {
        OUTPUT_NONE,
        OUTPUT_P1OFFSET,
        OUTPUT_P1VALUE,
        OUTPUT_P2OFFSET,
        OUTPUT_P2VALUE,
        OUTPUT_P3OFFSET,
        OUTPUT_P3VALUE,
        OUTPUT_P4OFFSET,
        OUTPUT_P4VALUE,
        OUTPUT_VOLUME,
        OUTPUT_FREQ,
        OUTPUT_PAN,
        OUTPUT_DETUNE,
        OUTPUT_SPREAD,
        OUTPUT_DRIVE,
        OUTPUT_NOISE_AMOUNT,
        OUTPUT_LP_FILTER_CUTOFF,
        OUTPUT_LP_FILTER_RESONANCE,
        OUTPUT_HP_FILTER_CUTOFF,
        OUTPUT_HP_FILTER_RESONANCE,
        OUTPUT_BP_FILTER_CUTOFF,
        OUTPUT_BP_FILTER_Q,
        OUTPUT_NT_FILTER_CUTOFF,
        OUTPUT_NT_FILTER_Q,
        OUTPUT_ADSR1_DECAY,
        OUTPUT_ADSR2_DECAY,

        OUTPUT_COUNT
    };

public:
    struct Entry
    {
        Input           src;
        Output          dst;
        eF32            mod;
        eF32            result;
    };

    struct State
    {
        tfADSR::State   adsrState1;
        tfADSR::State   adsrState2;
        tfLFO::State    lfoState1;
        tfLFO::State    lfoState2;
        eF32            values[INPUT_COUNT];
        Entry           entries[TF_MODMATRIXENTRIES];
        eF32            modulation[TF_MAX_MODULATIONS];

        State();

        void            noteOn(eF32 lfoPhase1, eF32 lfoPhase2);
        void            noteOff();
        void            panic();
    };


public:
    eBool               isActive(tfModMatrix::State *state);
    eBool               process(State *state, eF32 *params, eU32 len, eU32 sampleRate);
    eF32                get(tfModMatrix::State *state, Output output);

private:
    tfADSR              m_adsr;
    tfLFO               m_lfo;
};

#endif // TF_MODMATRIX_HPP