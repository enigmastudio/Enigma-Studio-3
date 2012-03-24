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

#ifndef TF_IEFFECT_HPP
#define TF_IEFFECT_HPP

class tfIEffect
{
public:

    enum Mode
    {
        FX_NONE = 0,
        FX_DISTORTION,
        FX_DELAY,
        FX_CHORUS,
        FX_FLANGER,
        FX_REVERB,
        FX_FORMANT,
        FX_EQ,
        FX_GAIN,
        FX_FILTER,
        FX_RESERVED8,

        FX_COUNT
    };

    virtual Mode        getMode();
    virtual void        setParams(eF32 *params);
    virtual void        update(eU32 sampleRate);
    virtual eBool       process(eSignal **signal, eU32 len);

    static tfIEffect * create(eF32 value, tfIEffect *old);
};

// -----------------------------------------------------------------------
//    Chorus
// -----------------------------------------------------------------------

class tfChorus : public tfIEffect
{
public:
    tfChorus();
    ~tfChorus();

    Mode        getMode();
    void        setParams(eF32 *params);
    void        update(eU32 sampleRate);
    eBool       process(eSignal **signal, eU32 len);

private:
    eF32        m_gain; 
    eF32        m_rate[2];
    eU32        m_mindelay;
    eU32        m_maxdelay;

    eSignal *    m_buffers[2];
    eSignal *    m_buffersDest[2];
    eU32        m_bufferLen;
    eU32        m_len;
    eU32        m_write[2];
    eF32        m_time;
    eU32        m_sampleRate;
};

// -----------------------------------------------------------------------
//    Delay
// -----------------------------------------------------------------------

class tfDelay : public tfIEffect
{
public:
    tfDelay();
    ~tfDelay();

    Mode        getMode();
    void        setParams(eF32 *params);
    void        update(eU32 sampleRate);
    eBool       process(eSignal **signal, eU32 len);

private:
    eF32        m_fDelay[2];
    eU32        m_delay[2];
    eF32        m_decay;

    eSignal *   m_delayBuffer[2];
    eU32        m_readOffset[2];
    eU32        m_writeOffset[2];
    eU32        m_bufferLen;
};

// -----------------------------------------------------------------------
//    Distortion
// -----------------------------------------------------------------------

class tfDistortion : public tfIEffect
{
public:
    tfDistortion();
    ~tfDistortion();

    Mode        getMode();
    void        setParams(eF32 *params);
    void        update(eU32 sampleRate);
    eBool       process(eSignal **signal, eU32 len);

private:
    eF32    m_paramAmount;
    eF32    m_amount;
    eF32    m_powTable[TF_DISTTABLESIZE];
};

// -----------------------------------------------------------------------
//    Flanger
// -----------------------------------------------------------------------

class tfFlanger : public tfIEffect
{
public:
    tfFlanger();
    ~tfFlanger();

    Mode        getMode();
    void        setParams(eF32 *params);
    void        update(eU32 sampleRate);
    eBool       process(eSignal **signal, eU32 len);

private:
    eInt    m_buffpos;
    eInt    m_bidi;
    eF32    m_buffleft[TF_FLANGERBUFFSIZE*2];
    eF32    m_buffright[TF_FLANGERBUFFSIZE*2];
    eInt    m_depth;
    eInt    m_targetDepth;
    eInt    m_volume;
    eInt    m_targetVolume;

/*#ifdef eUSE_SSE
    eF32x2  m_angle;
    eF32x2  m_angle0_1;
    eF32x2  m_lfocount;
    eF32x2  m_lastBpm;

    eF32x2  m_frequency;
    eF32x2  m_amplitude;
    eF32x2  m_wet;
    eF32x2  m_lfo;
#else*/
    eInt    m_angle;
    eF32    m_angle0;
    eF32    m_angle1;
    eF32    m_lfocount;
    eF32    m_lastBpm;

    eF32    m_frequency;
    eF32    m_amplitude;
    eF32    m_wet;
    eF32    m_lfo;
//#endif

    eInt    m_sampleRate;
};

// -----------------------------------------------------------------------
//    Reverb
// -----------------------------------------------------------------------

const int NUMCOMBS     = 8;
const int NUMALLPASSES = 4;

class tfReverb : public tfIEffect
{
public:
    tfReverb();
    ~tfReverb();

    Mode        getMode();
    void        setParams(eF32 *params);
    void        update(eU32 sampleRate);
    eBool       process(eSignal **signal, eU32 len);

    struct ReverbComb
    {
        eF32    filterstore;
        eF32 *  buffer;
        eInt    bufsize;
        eInt    bufidx;
    };

    struct ReverbAllpass
    {
        eF32 *  buffer;
        eInt    bufsize;
        eInt    bufidx;
    };

private:
    void alloc_comb_buffer(ReverbComb &comb, eU32 size);
    void alloc_allpass_buffer(ReverbAllpass &allpass, eU32 size);

    eF32 m_roomsize;
    eF32 m_damp;
    eF32 m_wet;
    eF32 m_dry;
    eF32 m_width;

    eF32 m_cmbFeedback;
    eF32 m_apsFeedback;

    eF32 m_gain;
    eF32 m_damp1;
    eF32 m_damp2;
    eF32 m_wet1;
    eF32 m_wet2;
    eF32 m_dry0;
    
    ReverbComb     m_Comb[2][NUMCOMBS];
    ReverbAllpass  m_Allpass[2][NUMALLPASSES];
};

// -----------------------------------------------------------------------
//    Gain
// -----------------------------------------------------------------------

class tfGain : public tfIEffect
{
public:
    tfGain();
    ~tfGain();

    Mode        getMode();
    void        setParams(eF32 *params);
    void        update(eU32 sampleRate);
    eBool       process(eSignal **signal, eU32 len);

private:
    eF32    m_amount;
};

// -----------------------------------------------------------------------
//    Filter
// -----------------------------------------------------------------------

class tfFilterFx : public tfIEffect
{
public:
    tfFilterFx();
    ~tfFilterFx();

    Mode        getMode();
    void        setParams(eF32 *params);
    void        update(eU32 sampleRate);
    eBool       process(eSignal **signal, eU32 len);

private:
    tfFilter::Mode  m_filterType;
    tfFilter::State m_state;
    tfFilter        m_filter;

    eF32            m_cutoff;
    eF32            m_resonance;
};

// -----------------------------------------------------------------------
//    Formant
// -----------------------------------------------------------------------

class tfFormantFx : public tfIEffect
{
public:
    tfFormantFx();
    ~tfFormantFx();

    Mode        getMode();
    void        setParams(eF32 *params);
    void        update(eU32 sampleRate);
    eBool       process(eSignal **signal, eU32 len);

private:
    eU32        m_mode;
    eF32        m_wet;
    eF64        m_memoryL[10];
    eF64        m_memoryR[10];
};

// -----------------------------------------------------------------------
//    EQ
// -----------------------------------------------------------------------

class tfEqFx : public tfIEffect
{
public:
    tfEqFx();
    ~tfEqFx();

    Mode        getMode();
    void        setParams(eF32 *params);
    void        update(eU32 sampleRate);
    eBool       process(eSignal **signal, eU32 len);

private:
	struct State
	{
		// Filter #1 (Low band)
		eF32x2        m_f1p0;     // Poles ...
		eF32x2        m_f1p1;     
		eF32x2        m_f1p2;
		eF32x2        m_f1p3;

		// Filter #2 (High band)
		eF32x2        m_f2p0;     // Poles ...
		eF32x2        m_f2p1;
		eF32x2        m_f2p2;
		eF32x2        m_f2p3;

		// Sample history buffer
		eF32x2        m_sdm1;     // Sample data minus 1
		eF32x2        m_sdm2;     //                   2
		eF32x2        m_sdm3;     //                   3
	};

	State *		m_state;

    eF32        m_lf;       // Frequency
    eF32        m_hf;       // Frequency

    // Gain Controls
    eF32        m_lg;       // low  gain
    eF32        m_mg;       // mid  gain
    eF32        m_hg;       // high gain
};


#endif // TF_IEFFECT_HPP