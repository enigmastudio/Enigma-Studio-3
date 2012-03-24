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

eBool tfInstrument::m_freqTableReady = eFALSE;
eF32  tfInstrument::m_freqTable[TF_NUMFREQS];

void tfInstrument::State::noteOn(eS32 note, eS32 velocity, eF32 lfoPhase1, eF32 lfoPhase2)
{
	eU32 seed = eRandomSeed();

	currentNote = note;
	currentVelocity = velocity;
	currentSlop = eRandomF(-1.0f, 1.0f, seed);
	noteIsOn = eTRUE;
	time = 0;

	oscState.reset();

#ifndef NO_ADDSYNTH
	addSynthState.reset();
#endif
	noiseState.reset();
	filterStateHP.reset();
	filterStateLP.reset();
	filterStateBP.reset();
	filterStateNT.reset();
	modMatrixState.noteOn(lfoPhase1, lfoPhase2);

#ifdef TF_OVERSAMPLING
	downSampleFilterState.reset();
	downSampleFilterState2.reset();
#endif
}

void tfInstrument::State::noteOff()
{
	noteIsOn = eFALSE;
	modMatrixState.noteOff();
}

void tfInstrument::State::panic()
{
	noteIsOn = eFALSE;
	playing = eFALSE;
	modMatrixState.panic();
}

tfInstrument::tfInstrument()
{
    setSampleRate(44100);

#ifndef ePLAYER
    for (eU32 i=0; i<TF_PARAM_COUNT; i++)
    {
        m_params[i] = TF_DEFAULTPROG[i];
    }
#else
    eMemSet(m_params, 0, sizeof(eF32) * TF_PARAM_COUNT);
#endif

    for (eU32 i=0; i<TF_EFFECTSLOTS; i++)
    {
        m_effects[i] = eNULL;
    }

    m_mixBufferLen = 0;
    m_mixBuffers[0] = eNULL;
    m_mixBuffers[1] = eNULL;
    m_lfo1Phase = 0.0f;
    m_lfo2Phase = 0.0f;
    m_ignoreParamUI = eFALSE;

#ifdef TF_OVERSAMPLING
    m_mixBuffersOverSampling[0] = eNULL;
    m_mixBuffersOverSampling[1] = eNULL;
#endif

    _prepareFreqTable();
}

tfInstrument::~tfInstrument()
{
    eSAFE_DELETE_ARRAY(m_mixBuffers[0]);
    eSAFE_DELETE_ARRAY(m_mixBuffers[1]);

#ifdef TF_OVERSAMPLING
    eSAFE_DELETE_ARRAY(m_mixBuffersOverSampling[0]);
    eSAFE_DELETE_ARRAY(m_mixBuffersOverSampling[1]);
#endif

    for (eU32 i=0; i<TF_EFFECTSLOTS; i++)
    {
        eSAFE_DELETE(m_effects[i]);
    }
}

void tfInstrument::_prepareFreqTable()
{
    if (!m_freqTableReady)
    {
        // make frequency (Hz) table
        eF64 k = 1.059463094359;    // 12th root of 2
        eF64 a = 6.875;    // a
        a *= k;    // b
        a *= k;    // bb
        a *= k;    // c, frequency of midi note 0

        for (eU32 i = 0; i < TF_NUMFREQS; i++)    // 128 midi notes
        {
            m_freqTable[i] = (eF32)a;
            a *= k;
        }

        m_freqTableReady = eTRUE;
    }
}

eF32 tfInstrument::getParam(eU32 index) const
{
    eASSERT(index >= 0 && index < TF_PARAM_COUNT);
    return m_params[index];
}

void tfInstrument::setParam(eU32 index, eF32 value)
{
    eASSERT(index >= 0 && index < TF_PARAM_COUNT);
    m_params[index] = value;
}

void tfInstrument::setIgnoreParamUI(eBool ignore)
{
    m_ignoreParamUI = ignore;
}

void tfInstrument::setParamUI(eU32 index, eF32 value)
{
    eASSERT(index >= 0 && index < TF_PARAM_COUNT);
    if (!m_ignoreParamUI)
    {
        eF32 diff = eAbs(m_params[index] - value);
        if (diff >= 0.005f) 
        {
            m_params[index] = value;
        }   
    }
}

void tfInstrument::setSampleRate(eU32 sampleRate)
{
    m_sampleRate = sampleRate;
    m_scaler = 1.0f/(eF32)sampleRate;
}

void tfInstrument::noteOn(eS32 note, eS32 velocity, eU32 modslot, eF32 mod)
{
    eF32 lfoPhase1 = 0.0f;
    eF32 lfoPhase2 = 0.0f;

    eU32 voice = allocateVoice();

    if (m_params[TF_LFO1_SYNC] < 0.5f)
        lfoPhase1 = m_lfo1Phase;

    if (m_params[TF_LFO2_SYNC] < 0.5f)
        lfoPhase2 = m_lfo2Phase;

    if (modslot >= 1 && modslot <= TF_MAX_MODULATIONS)
        m_state[voice].modMatrixState.modulation[modslot-1] = mod;

    m_state[voice].noteOn(note, velocity, lfoPhase1, lfoPhase2);
}

void tfInstrument::noteOff(eS32 note)
{
    for(eU32 i=0;i<TF_MAXVOICES;i++)
    {
        if (m_state[i].currentNote == note && m_state[i].noteIsOn)
        {
            m_state[i].noteOff();
        }
    }
}

void tfInstrument::allNotesOff()
{
    for(eU32 i=0;i<TF_MAXVOICES;i++)
    {
        if (m_state[i].noteIsOn)
            m_state[i].noteOff(); 
    }
}

void tfInstrument::panic()
{
    for(eU32 i=0;i<TF_MAXVOICES;i++)
    {
        if (m_state[i].noteIsOn)
            m_state[i].panic(); 
    }
}

eU32 tfInstrument::getPolyphony()
{
    eU32 count = 0;

    for(eU32 i=0;i<TF_MAXVOICES;i++)
    {
        if (m_state[i].playing)
            count++;
    }

    return count;
}

eU32 tfInstrument::allocateVoice()
{
    eU32 poly = eFtoL(m_params[TF_OSC_POLYPHONY] * (TF_MAXVOICES-1) + 1);

    eU32 time = 0;
    eS32 chosen = -1;

    for(eU32 i=0;i<poly;i++)
    {
        if (!m_state[i].playing && !m_state[i].noteIsOn)
        {
            return i;
        }
        else
        {
            if (chosen == -1 || m_state[i].time > time)
            {
                chosen = i;
                time = m_state[i].time;
            }
        }
    }

    return (eU32)chosen;
}

void tfInstrument::_prepareMixBufferInternal(eSignal **sig, eU32 len, eU32 actual_len)
{
    if (len != actual_len)
    {
        eSAFE_DELETE_ARRAY(*sig);
        *sig = new eSignal[len];
    }

    eMemSet(*sig, 0, sizeof(eSignal) * len);
}

void tfInstrument::prepareMixBuffers(eU32 len)
{
    _prepareMixBufferInternal(&m_mixBuffers[0], len, m_mixBufferLen);
    _prepareMixBufferInternal(&m_mixBuffers[1], len, m_mixBufferLen);
    
#ifdef TF_OVERSAMPLING
    _prepareMixBufferInternal(&m_mixBuffersOverSampling[0], len*TF_MAX_OVERSAMPLING, m_mixBufferLen*TF_MAX_OVERSAMPLING);
    _prepareMixBufferInternal(&m_mixBuffersOverSampling[1], len*TF_MAX_OVERSAMPLING, m_mixBufferLen*TF_MAX_OVERSAMPLING);
#endif

    m_mixBufferLen = len;
}

void tfInstrument::mix(eSignal **signals, eF32 volume)
{
    eSignalMix(signals, m_mixBuffers, m_mixBufferLen, volume);
}

void tfInstrument::updateAddSynth()
{
#ifndef NO_ADDSYNTH
    m_addSynth.update(m_params, m_sampleRate);
#endif
}

eF32 tfInstrument::process(eSignal **signals, eU32 len)
{
    eSetSSEFlushToZeroMode();

    for(eU32 k=0;k<TF_MAXVOICES;k++)
    {
        State *state = &m_state[k];
       
        if (state->noteIsOn || state->playing)
        {
            state->time++;

            eCLEAR_UNDENORMALIZED();

            // process mod matrix
            eBool has_mm_active = m_modMatrix.process(&state->modMatrixState, m_params, len, m_sampleRate);
            m_lfo1Phase = state->modMatrixState.lfoState1.phase;
            m_lfo2Phase = state->modMatrixState.lfoState2.phase;

            eASSERT_UNDENORMALIZED();
            
            // clear the mixing buffers
            prepareMixBuffers(len);
		
            eF32 velocity = (eF32)state->currentVelocity / 128.0f;

			if (!has_mm_active && !state->noteIsOn)
				velocity = 0.0f;
                
            //    Run Noise generator
            m_noise.update(&state->noiseState, m_params, m_sampleRate);
            eBool has_noise = m_noise.process(&state->noiseState, 
                &m_modMatrix, 
                &state->modMatrixState, 
                m_params, 
                m_mixBuffers, 
                len,
                velocity);
                
#ifdef eHWSYNTH
	    eSignalDebugWritePeak(m_mixBuffers, "Noise       ");
#endif	    


            eASSERT_UNDENORMALIZED();

            //    Run Oscillator
            eF32 baseFreq = m_freqTable[state->currentNote & 0x7f];
            eF32 slop = ePow(m_params[TF_OSC_SLOP], 3);
            baseFreq += state->currentSlop * slop * 8.0f;
            
            //  calculate amount of oversampling
#ifdef TF_OVERSAMPLING
            eU32 oversamplingCount = 1;
            eF32 minSamplingRate = baseFreq * 2 * TF_DESIRED_OVERSAMPLING;
            
            while (m_sampleRate * oversamplingCount < minSamplingRate
                && oversamplingCount < TF_MAX_OVERSAMPLING)
            {
                oversamplingCount++;
            }

            baseFreq *= m_scaler / oversamplingCount;
#else
            baseFreq *= m_scaler;
#endif
            
            eF32 glide = m_params[TF_OSC_GLIDE];
            if (glide > 0.0f && state->currentFreq > 0.0f)
            {
                eF32 freqDiff = baseFreq - state->currentFreq;
                freqDiff /= glide * 10.0f + 1.0f;
                state->currentFreq += freqDiff;
            }
            else
                state->currentFreq = baseFreq;
            
#ifdef TF_OVERSAMPLING
			eBool has_osc = m_osc.process(&state->oscState, 
                &m_modMatrix, 
                &state->modMatrixState, 
                m_params, 
                m_mixBuffersOverSampling, 
                len*oversamplingCount, 
                state->currentFreq, 
                velocity,
                oversamplingCount);
#else
            eBool has_osc = m_osc.process(&state->oscState, 
                &m_modMatrix, 
                &state->modMatrixState, 
                m_params, 
                m_mixBuffers, 
                len, 
                state->currentFreq, 
                velocity,
                1);
#endif

#ifdef eHWSYNTH
	    eSignalDebugWritePeak(m_mixBuffers, "Oscillators ");
#endif	    

            eASSERT_UNDENORMALIZED();

#ifndef NO_ADDSYNTH
            //    Run additive synth
#ifdef TF_OVERSAMPLING
			m_addSynth.update(m_params, m_sampleRate);
            eBool has_addsyn = m_addSynth.process(&state->addSynthState, 
                &m_modMatrix, 
                &state->modMatrixState, 
                m_params, 
                m_mixBuffersOverSampling, 
                len*oversamplingCount, 
                state->currentFreq, 
                velocity,
                oversamplingCount);
#else
            m_addSynth.update(m_params, m_sampleRate);
            eBool has_addsyn = m_addSynth.process(&state->addSynthState, 
                &m_modMatrix, 
                &state->modMatrixState, 
                m_params, 
                m_mixBuffers, 
                len, 
                state->currentFreq, 
                velocity,
                1);
#endif
#endif
            eASSERT_UNDENORMALIZED();
            
#ifdef eHWSYNTH
	    eSignalDebugWritePeak(m_mixBuffers, "Additive    ");
#endif	    


#ifdef TF_OVERSAMPLING
			m_filter.update(&state->downSampleFilterState, eNULL, eNULL, eNULL, tfFilter::FILTER_OVERSAMPLING_LOWPASS, m_sampleRate*oversamplingCount);
			m_filter.update(&state->downSampleFilterState2, eNULL, eNULL, eNULL, tfFilter::FILTER_OVERSAMPLING_LOWPASS, m_sampleRate*oversamplingCount);
			m_filter.process(&state->downSampleFilterState, m_mixBuffersOverSampling, m_mixBufferLen*oversamplingCount);
			m_filter.process(&state->downSampleFilterState2, m_mixBuffersOverSampling, m_mixBufferLen*oversamplingCount);
			eDownsampleMix(m_mixBuffers, m_mixBuffersOverSampling, m_mixBufferLen, oversamplingCount);
#endif

#ifndef NO_ADDSYNTH
            state->playing = has_noise || has_osc || has_addsyn;
#else
            state->playing = has_noise || has_osc;
#endif

            //    Run Filters
            if (m_params[TF_LP_FILTER_ON] > 0.5f)
            {
                m_filter.update(&state->filterStateLP, 
                    &m_modMatrix, 
                    &state->modMatrixState, 
                    m_params, 
                    tfFilter::FILTER_LOWPASS, 
                    m_sampleRate);

                m_filter.process(&state->filterStateLP, m_mixBuffers, len);
                
#ifdef eHWSYNTH
		eSignalDebugWritePeak(m_mixBuffers, "LP Filter   ");
#endif	    
            }

            eASSERT_UNDENORMALIZED();

            if (m_params[TF_HP_FILTER_ON] > 0.5f)
            {
                m_filter.update(&state->filterStateHP, 
                    &m_modMatrix, 
                    &state->modMatrixState, 
                    m_params, 
                    tfFilter::FILTER_HIGHPASS, 
                    m_sampleRate);

                m_filter.process(&state->filterStateHP, m_mixBuffers, len);

#ifdef eHWSYNTH
		eSignalDebugWritePeak(m_mixBuffers, "HP Filter   ");
#endif	    
            }

            eASSERT_UNDENORMALIZED();

#ifndef NO_BANDPASS_FILTER
            if (m_params[TF_BP_FILTER_ON] > 0.5f)
            {
                m_filter.update(&state->filterStateBP, 
                    &m_modMatrix, 
                    &state->modMatrixState, 
                    m_params, 
                    tfFilter::FILTER_BANDPASS, 
                    m_sampleRate);

                m_filter.process(&state->filterStateBP, m_mixBuffers, len);
                
#ifdef eHWSYNTH
		eSignalDebugWritePeak(m_mixBuffers, "BP Filter   ");
#endif	    
            }
#endif
            eASSERT_UNDENORMALIZED();

#ifndef NO_NOTCH_FILTER
            if (m_params[TF_NT_FILTER_ON] > 0.5f)
            {
                m_filter.update(&state->filterStateNT, 
                    &m_modMatrix, 
                    &state->modMatrixState, 
                    m_params, 
                    tfFilter::FILTER_NOTCH, 
                    m_sampleRate);

                m_filter.process(&state->filterStateNT, m_mixBuffers, len);
                
#ifdef eHWSYNTH
		eSignalDebugWritePeak(m_mixBuffers, "NT Filter   ");
#endif	    
            }
#endif
            eASSERT_UNDENORMALIZED();

            //  Mix to final signal.
            mix(signals, m_params[TF_GAIN_AMOUNT]);
            
#ifdef eHWSYNTH
		eSignalDebugWritePeak(signals, "Mixed       ");
#endif	    
				
            eASSERT_UNDENORMALIZED();
        }    
    }

    //    Run effects
    for(eU32 i=0;i<TF_EFFECTSLOTS;i++)
    {
        tfIEffect *fx = m_effects[i];

        m_effects[i] = fx = tfIEffect::create(m_params[TF_EFFECT_1 + i], fx);

        if (fx != eNULL)
        {
            fx->setParams(m_params);
            fx->update(m_sampleRate);
            eBool has_output = fx->process(signals, len);

            eASSERT_UNDENORMALIZED();
            
#ifdef eHWSYNTH
	    eSignalDebugWritePeak(signals, "Effects     ");
#endif	    
        }
    }

#ifndef eHWSYNTH
	eF32 peak1 = 0.0f;
	eF32 peak2 = 0.0f;
	eSignalToPeak(signals, &peak1, &peak2, len);
	return (peak1 + peak2) / 2.0f;
#else
	return 0.0f;
#endif
}

tfOscillator * tfInstrument::getOscillator()
{
    return &m_osc;
}

eF32 * tfInstrument::getParams()
{
    return m_params;
}