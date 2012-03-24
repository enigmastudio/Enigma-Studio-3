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

#ifndef TF_NOISE_HPP
#define TF_NOISE_HPP

class tfNoise
{
public:

    struct State
    {
        void reset();

        eU32 offset1;
        eU32 offset2;

        eBool filterOn;

        tfFilter::State filterStateHP;
        tfFilter::State filterStateLP;
    };

    tfNoise();

    void update(State *state, eF32 *params, eU32 sampleRate);
    eBool process(State *state,
        tfModMatrix *modMatrix, 
        tfModMatrix::State *modMatrixState, 
        eF32 *params, 
        eSignal **signal, 
        eU32 len,
        eF32 velocity);

private:

    tfFilter m_filter;

    static void generateNoiseTable();

    static eBool m_noiseReady;
    static eF32  m_noiseTable[TF_NOISETABLESIZE];
};

#endif // TF_NOISE_HPP