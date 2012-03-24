/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   This file is part of
 *       _______   ______________  ______     _____
 *      / ____/ | / /  _/ ____/  |/  /   |   |__  /
 *     / __/ /  |/ // // / __/ /|_/ / /| |    /_ <
 *    / /___/ /|  // // /_/ / /  / / ___ |  ___/ /
 *   /_____/_/ |_/___/\____/_/  /_/_/  |_| /____/.
 *
 *   Copyright © 2003-2010 Brain Control, all rights reserved.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef TSHADER_HPP
#define TSHADER_HPP

class eTShader
{
private:
    struct State
    {
        enum
        {
            AUDIO_CHANNELS = 32+1,  // 1 master peak + 32 instrument peaks
            VARS_COUNT     = 32,
            REGS_COUNT     = AUDIO_CHANNELS+2,
        };

        eTShader *              ts;
        eConstPtr               callerId;
        eVector4                vars[VARS_COUNT];
        eVector4                regs[REGS_COUNT];
    };

    typedef eArray<State> StateArray;

public:
    eTShader();
    ~eTShader();

    void                        load(const eByteArray &code);
    void                        free();
    void                        run(eConstPtr calledId, eVector4 &out);
    void                        setTime(eConstPtr calledId, eF32 time);
    void                        setAudio(eConstPtr calledId, eU32 channel, eF32 value);

private:
    State &                     _getStateForCaller(eConstPtr callerId);

    ePtr                        m_machineCode;
    StateArray                  m_states;
};

#endif // TSHADER_HPP