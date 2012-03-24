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

#ifndef TF_ADSR_HPP
#define TF_ADSR_HPP

class tfADSR
{
public:
    enum ADSRState 
    {
        ADSR_STATE_ATTACK = 0,
        ADSR_STATE_DECAY,
        ADSR_STATE_SUSTAIN,
        ADSR_STATE_RELEASE,
        ADSR_STATE_FINISHED,
    };

public:
    struct State 
    {
        State()
        {
            reset();
        }

        void reset()
        {
            state = ADSR_STATE_FINISHED;
            volume = 0.0;    
        }

        eBool isEnd()
        {
            return state == ADSR_STATE_FINISHED;
        }

        void noteOn()
        {
            state = ADSR_STATE_ATTACK;
        }

        void noteOff()
        {
            state = ADSR_STATE_RELEASE;
        }

        eF32        volume;
        ADSRState   state;
    };

public:
    eF32    process(State *state, eU32 len, eF32 a, eF32 d, eF32 s, eF32 r, eF32 slope, eU32 sampleRate);
};

#endif // TF_ADSR_HPP