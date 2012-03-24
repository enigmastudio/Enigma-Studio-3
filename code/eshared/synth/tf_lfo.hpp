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

#ifndef TF_LFO_HPP
#define TF_LFO_HPP

class tfLFO
{
public:
    struct State 
    {
        State() :
            phase(0.0f),
            freq(0.0f)
        {
        }

        void reset(eF32 p)
        {
            phase = p;
        }

        eF32 freq;
        eF32 phase;
    };

public:
    tfLFO();

    void    setFrequency(State *state, eF32 f, eU32 sampleRate, eU32 blocksize);
    eF32    process(State *state, eF32 depth, eF32 shape);

private:
    static void generateNoiseTable();

    static eBool m_noiseReady;
    static eF32  m_noiseTable[TF_NOISETABLESIZE+1];
};

#endif // TF_LFO_HPP