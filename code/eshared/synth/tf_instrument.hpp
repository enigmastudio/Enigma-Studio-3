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

#ifndef TF_INSTRUMENT_HPP
#define TF_INSTRUMENT_HPP

class tfInstrument
{
public:
    tfInstrument();
    ~tfInstrument();

    struct Data
    {
        eF32 params[TF_PARAM_COUNT];
        eChar name[24];
    };

    struct State
    {
        State()
        {            
            noteIsOn = eFALSE;
            playing = eFALSE;
            currentFreq = 0.0f;
        }

        void noteOn(eS32 note, eS32 velocity, eF32 lfoPhase1, eF32 lfoPhase2);
        void noteOff();
        void panic();

#ifndef NO_ADDSYNTH
        tfAddSynth::State   addSynthState;
#endif
#ifdef TF_OVERSAMPLING
		tfFilter::State	    downSampleFilterState;
		tfFilter::State	    downSampleFilterState2;
#endif
		tfOscillator::State oscState;
        tfFilter::State     filterStateHP;
        tfFilter::State     filterStateLP;
        tfFilter::State     filterStateBP;
        tfFilter::State     filterStateNT;
        tfNoise::State      noiseState;
        tfModMatrix::State  modMatrixState;
        eS32                currentNote;
        eS32                currentVelocity;
        eF32                currentSlop;
        eF32                currentFreq;
        eF32                targetFreq;
        eBool               noteIsOn;
        eBool               playing;
        eU32                time;
    };

    eF32    getParam(eU32 index) const;
    void    setParam(eU32 index, eF32 value);
    void    setParamUI(eU32 index, eF32 value);
    void    setIgnoreParamUI(eBool value);
    void    updateAddSynth();
    eF32    process(eSignal **signals, eU32 len);
    void    setSampleRate(eU32 sampleRate);
    void    setModulation(eU32 slot, eF32 value);
    void    noteOn(eS32 note, eS32 velocity, eU32 modslot, eF32 mod);
    void    noteOff(eS32 note);
    void    allNotesOff();
    void    panic();
    eU32    getPolyphony();
    eU32    allocateVoice();
    eU32    getNewestVoice();
    void    prepareMixBuffers(eU32 len);
    void    mix(eSignal **signals, eF32 volume);

    tfOscillator * getOscillator();
    eF32 *         getParams();

private:

    static void _prepareFreqTable();
    static void _prepareMixBufferInternal(eSignal **sig, eU32 len, eU32 actual_len);

    eF32            m_params[TF_PARAM_COUNT];
    eU32            m_sampleRate;
    eF32            m_scaler;
    eBool           m_ignoreParamUI;

    eF32            m_lfo1Phase;
    eF32            m_lfo2Phase;

#ifndef NO_ADDSYNTH
    tfAddSynth      m_addSynth;
#endif

    tfOscillator    m_osc;
    
    tfNoise         m_noise;
    tfFilter        m_filter;
    tfModMatrix     m_modMatrix;
    tfIEffect *     m_effects[TF_EFFECTSLOTS];
    State           m_state[TF_MAXVOICES];

    eSignal *       m_mixBuffers[2];
#ifdef TF_OVERSAMPLING
    eSignal *       m_mixBuffersOverSampling[2];
#endif
    eU32            m_mixBufferLen;

public:
    static eBool    m_freqTableReady;
    static eF32     m_freqTable[TF_NUMFREQS];

};

typedef eArray<tfInstrument> tfInstrumentArray;

#endif // TF_INSTRUMENT_HPP