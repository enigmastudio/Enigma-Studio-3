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

#ifndef TF_ADD_SYNTH_HPP
#define TF_ADD_SYNTH_HPP

#ifndef NO_ADDSYNTH

class tfAddSynth
{
public:

    struct State
    {
        State()
        {
            reset();    
        }

        void reset();

        eF32 phase1;
        eF32 phase2;
        eF32 freq1;
        eF32 freq2;
    };

    tfAddSynth();
    ~tfAddSynth();

    void    update(eF32 *params, eU32 sampleRate);
    eBool   process(State *state, 
                    tfModMatrix *modMatrix, 
                    tfModMatrix::State *modMatrixState, 
                    eF32 *params, 
                    eSignal **signals, 
                    eU32 len, 
                    eF32 baseFreq, 
                    eF32 velocity, 
                    eU32 oversamplingCount);

private:

    static eF32     profileGauss(eF32 fi, eF32 bwi);
    static eF32     profileSingle(eF32 fi, eF32 bwi);
    static eF32     profileDetune(eF32 fi, eF32 bwi);
    static eF32     profileSpread(eF32 fi, eF32 bwi);

    eINLINE void    normalize(eInt N, eF32 *smp);
    void            extendedAlgorithm(eInt N, eInt profile, eInt sampleRate, eF32 f, eF32 bw, eF32 bwscale, eInt number_harmonics, eF32 *A, eF32 *smp);
    void            ifft(eInt N, eF32 *freq_amp, eF32 *freq_phase, eF32 *smp);
    void            fft(eF32 *fftBuffer, eInt fftFrameSize, eInt sign);
    void            gen(eF32 bandwidth, eF32 bwscale, eF32 damp, eF32 numharmonics, eInt profile, eInt sampleRate);
    void            check();

    eF32 *          m_padSynthTable;
    eF32 *          m_padSynthTableTemp;
    eF32            m_bandwidth;
    eF32            m_damp;
    eF32            m_numHarmonics;
    eF32            m_bwScale;
    eU32            m_profile;
    eU32            m_sampleRate;
    eBool           m_padRefresh;
};

#endif

#endif // TF_ADD_SYNTH_HPP