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

tfIEffect::Mode tfIEffect::getMode()
{
    return FX_NONE;
}

void tfIEffect::setParams(eF32 *params)
{
}

void tfIEffect::update(eU32 sampleRate)
{
}

eBool tfIEffect::process(eSignal **signal, eU32 len)
{
    return eFALSE;
}

tfIEffect * tfIEffect::create(eF32 value, tfIEffect *old)
{
    tfIEffect::Mode mode = (tfIEffect::Mode)eFtoL(eRound(value * (FX_COUNT-1)));

    if (old == eNULL || old->getMode() != mode)
    {
        tfIEffect *fx = eNULL;

        switch(mode)
        {
#ifndef NO_FX_DISTORTION
        case tfIEffect::FX_DISTORTION:
            fx = new tfDistortion();
            break;
#endif
#ifndef NO_FX_DELAY
        case tfIEffect::FX_DELAY:
            fx = new tfDelay();
            break;
#endif
#ifndef NO_FX_CHORUS
        case tfIEffect::FX_CHORUS:
            fx = new tfChorus();
            break;
#endif
#ifndef NO_FX_FLANGER
        case tfIEffect::FX_FLANGER:
            fx = new tfFlanger();
            break;
#endif
#ifndef NO_FX_REVERB
        case tfIEffect::FX_REVERB:
            fx = new tfReverb();
            break;
#endif
#ifndef NO_FX_FORMANT
        case tfIEffect::FX_FORMANT:
            fx = new tfFormantFx();
            break;
#endif
#ifndef NO_FX_EQ
        case tfIEffect::FX_EQ:
            fx = new tfEqFx();
            break;
#endif
        default:
            {
                eSAFE_DELETE(old);
                return eNULL;
            }
        }

        return fx;
    }

    return old;
}


// -----------------------------------------------------------------------
//    Chorus
// -----------------------------------------------------------------------

tfChorus::tfChorus()
{
	m_buffers[0]        = eNULL;
    m_buffers[1]        = eNULL;
    m_buffersDest[0]    = eNULL;
    m_buffersDest[1]    = eNULL;
    m_write[0]          = 0;
    m_write[1]          = 0;
    m_bufferLen         = 0;
    m_len               = 0;
    m_time              = 0;
    m_sampleRate        = 0;
}

tfChorus::~tfChorus()
{
	eSAFE_DELETE_ARRAY(m_buffers[0]);
    eSAFE_DELETE_ARRAY(m_buffers[1]);
}

tfIEffect::Mode tfChorus::getMode()
{
    return tfIEffect::FX_CHORUS;
}

void tfChorus::setParams(eF32 *params)
{
    eF32 rate = params[TF_CHORUS_RATE];
    eF32 depth = params[TF_CHORUS_DEPTH];
    m_gain = params[TF_CHORUS_GAIN];

    depth *= depth;

    m_rate[0] = rate;
    m_rate[1] = rate;
    m_mindelay = 1;
    m_maxdelay = 1 + eFtoL(depth * 50.0f);
}

void tfChorus::update(eU32 sampleRate)
{
    eU32 newBufferLen = sampleRate/10;
    m_sampleRate = sampleRate;
    m_len = eFtoL(m_maxdelay * (eF32)sampleRate / 1000.0f);
        
    if (newBufferLen != m_bufferLen)
    {
        if (m_write[0] > newBufferLen)
            m_write[0] = 0;

        if (m_write[1] > newBufferLen)
            m_write[1] = 0;
        
        if (newBufferLen > m_bufferLen)
        {
            m_bufferLen = newBufferLen;

            m_buffers[0] = new eF32[m_bufferLen];
            m_buffers[1] = new eF32[m_bufferLen];

            eMemSet(m_buffers[0], 0, m_bufferLen * sizeof(eF32));
            eMemSet(m_buffers[1], 0, m_bufferLen * sizeof(eF32));

            m_buffersDest[0] = m_buffers[0];
            m_buffersDest[1] = m_buffers[1];
        }
    }
}

eBool tfChorus::process(eSignal **signal, eU32 len)
{
    eF32 time;
    eF32 width = (eF32)(m_maxdelay - m_mindelay);
	
    for (int j=0; j<2; j++)
    {
        eF32 f             = 1.0f / m_sampleRate * ePI * m_rate[j];
        eSignal *source    = signal[j];
        eU32 write         = m_write[j];
        eF32 *buffer_src   = m_buffers[j];
        eF32 *buffer_dest  = &m_buffersDest[j][write];
        eF32 *buffer_start = m_buffersDest[j];

        time = m_time;

        eU32 len2 = len;
        while (len2--)
        {
            // Process LFO for Chorus
            time++;
            eF32 millisecs = (eF32)(m_mindelay + (((eSin(time * f) + 1.0f) / 2.0f) * width));
            eASSERT(millisecs >= 0.0f);

            eU32 delay_samples = eFtoL(millisecs * m_sampleRate / 1000.0f);
            eASSERT(delay_samples <= m_bufferLen);
               
            // calculate buffer read position
            eInt read = write - delay_samples;
            if (read < 0) { read += (eInt)m_bufferLen; }
            eASSERT(read >= 0);
            
            // get the sample from the buffer
            *buffer_dest++ = *source * m_gain; 
            *source++ += buffer_src[read];

            write++;

            if (write >= m_bufferLen)
            {
                write = 0;
                buffer_dest = buffer_start;
            }               
        }

        m_write[j] = write;
    }

    m_time = time;

    return eTRUE;
}

// -----------------------------------------------------------------------
//    Delay
// -----------------------------------------------------------------------

tfDelay::tfDelay()
{
	m_delayBuffer[0] = eNULL;
    m_delayBuffer[1] = eNULL;
    m_readOffset[0] = 0;
    m_readOffset[1] = 0;
    m_writeOffset[0] = 1;
    m_writeOffset[1] = 1;
    m_bufferLen = 0;
}

tfDelay::~tfDelay()
{
	eSAFE_DELETE_ARRAY(m_delayBuffer[0]);
    eSAFE_DELETE_ARRAY(m_delayBuffer[1]);
}

tfIEffect::Mode tfDelay::getMode()
{
    return tfIEffect::FX_DELAY;
}

void tfDelay::setParams(eF32 *params)
{
    m_fDelay[0] = params[TF_DELAY_LEFT];
    m_fDelay[1] = params[TF_DELAY_RIGHT];
    m_decay     = params[TF_DELAY_DECAY];
}

void tfDelay::update(eU32 sampleRate)
{
    eU32 len = sampleRate; // 1 seconds delay max
    const eF32x2 const_zero = eSIMDSet2(0.0f);
    const eF32x2 const_len = eSIMDSet2((eF32)len);

    eF32x2 mdelay = eSIMDLoad2(m_fDelay[0], m_fDelay[1]);

    mdelay = eSIMDMax(
                eSIMDMin(
                    eSIMDMul(
                        mdelay, 
                        const_len
                        ), 
                    const_len
                    ), 
                const_zero
                );

    eF32 store_delay[2];
    eSIMDStore(mdelay, &store_delay[0], &store_delay[1]);

    m_delay[0] = eFtoL(store_delay[0]);
    m_delay[1] = eFtoL(store_delay[1]);

    if (m_bufferLen != len)
    {
        eSAFE_DELETE_ARRAY(m_delayBuffer[0]);
		eSAFE_DELETE_ARRAY(m_delayBuffer[1]);
        m_bufferLen = len;

        m_delayBuffer[0] = new eSignal[m_bufferLen];
        m_delayBuffer[1] = new eSignal[m_bufferLen];

        const eU32 memsetsize = sizeof(eSignal) * m_bufferLen;

        eMemSet(m_delayBuffer[0], 0, memsetsize);
        eMemSet(m_delayBuffer[1], 0, memsetsize);
    }
}

eBool tfDelay::process(eSignal **signals, eU32 len)
{
    for (eU32 j=0;j<2;j++)
    {
        eU32 *read_pos          = &m_readOffset[j];
        eU32 *write_pos         = &m_writeOffset[j];
        eSignal *signal         = signals[j];

        eSignal *buffer_src     = &m_delayBuffer[j][*read_pos];         
        eSignal *buffer_dest    = &m_delayBuffer[j][*write_pos];
        eU32 buffer_len         = m_delay[j]; 
            
        eU32 len2 = len;
        while (len2--)
        {
            // fetch one sample
            eF32 sample = *signal;

            // write it to buffer
            *buffer_dest += sample;
            *buffer_dest *= m_decay;
            eUndenormalise((*buffer_dest));
            buffer_dest++;

            // read from buffer and add to sample
            sample += *buffer_src++;

            // increase write position and verify
            (*write_pos)++;
            if ((*write_pos) >= buffer_len)
            {
                (*write_pos) = 0;
                buffer_dest = m_delayBuffer[j];
            }

            // increase read position and verify
            (*read_pos)++;
            if ((*read_pos) >= buffer_len)
            {
                (*read_pos) = 0;
                buffer_src = m_delayBuffer[j];  
            }
                
            // write to output
            *signal++ = sample;
        }
    }

    return eTRUE;
}

// -----------------------------------------------------------------------
//    Distortion
// -----------------------------------------------------------------------

tfDistortion::tfDistortion()
{
	m_amount = 0.0f;
}

tfDistortion::~tfDistortion()
{

}

tfIEffect::Mode tfDistortion::getMode()
{
    return tfIEffect::FX_DISTORTION;
}

void tfDistortion::setParams(eF32 *params)
{
    m_paramAmount = params[TF_DISTORT_AMOUNT];
}

void tfDistortion::update(eU32 sampleRate)
{
    eF32 amount = 1.0f - m_paramAmount;

    if (amount != m_amount)
    {
        m_amount = amount;

        for (eU32 base = 0; base<32768; base++)
        {
            m_powTable[base] = ePow(base/32768.f, amount);
        }
    }
}

eBool tfDistortion::process(eSignal **signal, eU32 len)
{
    for(eU32 i=0;i<2;i++)
    {
        eSignal *in = signal[i];
        eU32 len2 = len;

        while(len2--)
        {
            eF32 val = *in;
            eF32 sign = eSign(val);
            eF32 abs  = (eF32)eAbs(val);
            if (abs > 1.0f) abs = 1.0;
            eU32 offs = eFtoL(abs*32767);
            *in++ = sign * m_powTable[offs];
        }
    }

    return eTRUE;
}

// -----------------------------------------------------------------------
//    Flanger
// -----------------------------------------------------------------------

const eF32 DELAYMIN = 44100.0f *0.1f / 1000.0f;    // 0.1 ms delay min
const eF32 DELAYMAX = 44100.0f *12.1f / 1000.0f;    // 12.1 ms delay max

tfFlanger::tfFlanger()
{
	m_angle = 0;
    m_angle0 = 0.0f;
    m_angle1 = 0.0f;
    m_lfocount = 0.0f;
    m_lastBpm = 0.0f;

    m_buffpos = 0;
    m_bidi = 0;
    m_depth = 0;
    m_targetDepth = 0;
    m_volume = 0;
    m_targetVolume = 0;

    eMemSet(m_buffleft, 0, sizeof(eF32) * TF_FLANGERBUFFSIZE*2);
    eMemSet(m_buffright, 0, sizeof(eF32) * TF_FLANGERBUFFSIZE*2);
}

tfFlanger::~tfFlanger()
{

}

tfIEffect::Mode tfFlanger::getMode()
{
    return tfIEffect::FX_FLANGER;
}

void tfFlanger::setParams(eF32 *params)
{
    m_frequency  = params[TF_FLANGER_FREQUENCY];
    m_amplitude  = params[TF_FLANGER_AMPLITUDE];
    m_wet        = params[TF_FLANGER_WET];
    m_lfo        = params[TF_FLANGER_LFO];
}

void tfFlanger::update(eU32 sampleRate)
{
    m_sampleRate = sampleRate;
}

eBool tfFlanger::process(eSignal **signals, eU32 len)
{
    eSignal *pcmleft  = signals[0];
    eSignal *pcmright = signals[1];
    /*
#ifdef eUSE_SSE

    eF32x2 inc = eSIMDSet2(0.1f*(eF32)len);
    eF32x2 const_0 = eSIMDSet2(0.0f);
    eF32x2 const_p1 = eSIMDSet2(1.0f);
    eF32x2 const_m1 = eSIMDSet2(-1.0f);
    eF32x2 const_120 = eSIMDSet2(120.0f);
    eF32x2 const_4096 = eSIMDSet2(4096.0f);
    eF32x2 delay_min = eSIMDSet2(DELAYMIN);
    eF32x2 delay_diff = eSIMDSet2((DELAYMAX - DELAYMIN) / 8192.0f);
    eF32x2 samplerate_scaled = eSIMDSet2(m_sampleRate * 4.0f * 60.0f / ePI);

    while(len--)
    {
        if (m_bidi==0)
        {
            // m_lfocount += m_lfo * inc;
            m_lfocount = eSIMDAdd(m_lfocount, eSIMDMul(m_lfo, inc));

            // if (m_lfocount > m_frequency)
            if (eSSECmpTrue(_mm_cmpgt_ss(m_lfocount, m_frequency)))
            {
                m_lfocount = eSIMDSet2(1.0f);
                m_bidi = 1;
            }
        }
        else
        {
            // m_lfocount -= m_lfo * inc;
            m_lfocount = eSIMDSub(m_lfocount, eSIMDMul(m_lfo, inc));

            // if (m_lfocount < 0.0f)
            if (eSSECmpTrue(_mm_cmplt_ss(m_lfocount, const_0)))
            {
                m_lfocount = eSIMDSet2(0.01f);
                m_bidi = 0;
            }
        }

        eF32x2 frequency = m_lfocount;

        // if (frequency==0.0) 
        if (eSSECmpTrue(_mm_cmpeq_ss(frequency, const_0))) 
            frequency = eSIMDSet2(1.0f);

        // if(m_lastBpm != frequency) 
        if (eSSECmpTrue(_mm_cmpneq_ss(m_lastBpm, frequency))) 
        {
            //m_angle0 += (eF32)((eF64)m_angle * frequency * 120.0f / samplerate_scaled);    
            //m_angle1 += (eF32)((eF64)m_angle * (1.0f - frequency) * 120.0f / samplerate_scaled);
            m_angle0_1 = _mm_div_ps(eSIMDMul(eSIMDMul(m_angle, frequency), const_120), samplerate_scaled);
            
            m_lastBpm = frequency;
            m_angle = eSIMDSet2(0.0f);
        }

        // eInt deltaleft = eFtoL(DELAYMIN + ((DELAYMAX - DELAYMIN) / 8192.0f) * m_amplitude * 4096.0f * (1.0f - eCos(m_angle0 + m_angle * frequency / (m_sampleRate * 4 * 60 / ePI))));
        // eInt deltaright = eFtoL(DELAYMIN + ((DELAYMAX - DELAYMIN) / 8192.0f) * m_amplitude * 4096.0f * (1.0f - eCos(m_angle1 + m_angle * frequency / (m_sampleRate * 4 * 60 / ePI))));
        eF32x2 cos_input = eSIMDAdd(m_angle0_1, _mm_div_ps(eSIMDMul(m_angle, frequency), samplerate_scaled));

        eF32A store_cos_input[4];
        _mm_store_ps(store_cos_input, cos_input);

        eF32x2 cos_output = _mm_set_ps(eCos(store_cos_input[3]),
                                        eCos(store_cos_input[2]),
                                        eCos(store_cos_input[1]),
                                        eCos(store_cos_input[0]));

        eF32x2 deltas = eSIMDAdd(delay_min, eSIMDMul(eSIMDMul(eSIMDMul(delay_diff, m_amplitude), const_4096), eSIMDSub(const_p1, cos_output)));

        eF32A store_deltas[4];
        _mm_store_ps(store_deltas, deltas);

        m_angle = eSIMDAdd(m_angle, const_p1);

        eInt ppleft = m_buffpos - eFtoL(store_deltas[3]);
        eInt ppright = m_buffpos - eFtoL(store_deltas[2]);
        
        if(ppleft<0) 
            ppleft += TF_FLANGERBUFFSIZE;

        if(ppright<0) 
            ppright += TF_FLANGERBUFFSIZE;

        // eF32 l = *pcmleft - m_wet * m_buffleft[ppleft];
        // eF32 r = *pcmright - m_wet * m_buffright[ppright];
        eF32x2 buffervals = _mm_set_ps(m_buffleft[ppleft], m_buffright[ppright], 0.0f, 0.0f);
        eF32x2 signal = _mm_set_ps(*pcmleft, *pcmright, 0.0f, 0.0f);

        signal = _mm_max_ps(
                    _mm_min_ps(
                        eSIMDSub(
                            signal, 
                            eSIMDMul(
                                m_wet, 
                                buffervals)
                            ), 
                        const_p1
                   ), 
                   const_m1
                 );

        eF32A store_signal[4];
        _mm_store_ps(store_signal, signal);

        *pcmleft = store_signal[3];
        *pcmright = store_signal[2];

        m_buffleft[m_buffpos] = store_signal[3];
        m_buffright[m_buffpos] = store_signal[2];

        pcmleft++;
        pcmright++;
        m_buffpos++;

        if (m_buffpos >= TF_FLANGERBUFFSIZE) 
            m_buffpos = 0;
    }
#else
*/
    eF32 inc = 0.1f*(eF32)len;

    while(len--)
    {
        if (m_bidi==0)
        {
            m_lfocount += m_lfo * inc;

            if (m_lfocount > m_frequency)
            {
                m_lfocount = 1.0f;
                m_bidi = 1;
            }
        }
        else
        {
            m_lfocount -= m_lfo * inc;

            if (m_lfocount < 0.0f)
            {
                m_lfocount = 0.01f;
                m_bidi = 0;
            }
        }

        eF32 frequency = m_lfocount;

        if (frequency==0.0) 
            frequency=1.0f;

        if(m_lastBpm != frequency) 
        {
            m_angle0 += (eF32)((eF64)m_angle * frequency * 120.0f / (m_sampleRate * 4.0f * 60.0f / ePI));    
            m_angle1 += (eF32)((eF64)m_angle * (1.0f - frequency) * 120.0f / (m_sampleRate * 4.0f * 60.0f / ePI));
            m_lastBpm = frequency;
            m_angle = 0;
        }

        eInt deltaleft = eFtoL(DELAYMIN + ((DELAYMAX - DELAYMIN) / 8192.0f) * m_amplitude * 4096.0f * (1.0f - eCos(m_angle0 + m_angle * frequency / (m_sampleRate * 4 * 60 / ePI))));
        eInt deltaright = eFtoL(DELAYMIN + ((DELAYMAX - DELAYMIN) / 8192.0f) * m_amplitude * 4096.0f * (1.0f - eCos(m_angle1 + m_angle * frequency / (m_sampleRate * 4 * 60 / ePI))));

        m_angle++;

        eInt ppleft = m_buffpos - deltaleft;
        eInt ppright = m_buffpos - deltaright;
        
        if(ppleft<0) 
            ppleft += TF_FLANGERBUFFSIZE;

        if(ppright<0) 
            ppright += TF_FLANGERBUFFSIZE;

        eF32 l = *pcmleft - m_wet * m_buffleft[ppleft];

        if (l<-1.0f) 
            l=-1.0f; 
        else if (l>1.0f) 
            l=1.0f;

        eF32 r = *pcmright - m_wet * m_buffright[ppright];

        if (r<-1.0f)
            r=-1.0f; 
        else if (r>1.0f) 
            r=1.0f;

        eUndenormalise(l);
        eUndenormalise(r);

        *pcmleft = l;
        *pcmright = r;

        m_buffleft[m_buffpos] = l;
        m_buffright[m_buffpos] = r;

        pcmleft++;
        pcmright++;
        m_buffpos++;

        if (m_buffpos >= TF_FLANGERBUFFSIZE) 
            m_buffpos = 0;
    }
//#endif

    return eTRUE;
}


// -----------------------------------------------------------------------
//    Reverb
// -----------------------------------------------------------------------

const eF32 MUTED        = 0.0f;
const eF32 FIXEDGAIN    = 0.015f;
const eF32 SCALEWET     = 3.0f;
const eF32 SCALEDRY     = 2.0f;
const eF32 SCALEDAMP    = 0.4f;
const eF32 SCALEROOM    = 0.28f;
const eF32 OFFSETROOM   = 0.7f;
const eF32 INITIALROOM  = 0.5f;
const eF32 INITIALDAMP  = 0.5f;
const eF32 INITIALWET   = 1.0f / SCALEWET;
const eF32 INITIALDRY   = 0.0f;
const eF32 INITIALWIDTH = 1.0f;
const eF32 INITIALMODE  = 0.0f;
const eF32 FREEZEMODE   = 0.5f;
const eInt STEREOSPREAD = 23;

const eInt COMBTUNINGS[]    = { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 };
const eInt ALLPASSTUNINGS[] = { 556, 441, 341, 225 };

tfReverb::tfReverb()
{
	for (int i=0; i<NUMCOMBS; i++)
    {
        alloc_comb_buffer(m_Comb[0][i], COMBTUNINGS[i]);
        alloc_comb_buffer(m_Comb[1][i], COMBTUNINGS[i] + STEREOSPREAD);
    }

    for (int i=0; i<NUMALLPASSES; i++)
    {
        alloc_allpass_buffer(m_Allpass[0][i], ALLPASSTUNINGS[i]);
        alloc_allpass_buffer(m_Allpass[1][i], ALLPASSTUNINGS[i] + STEREOSPREAD);
    }

    m_apsFeedback = 0.5f;
}

tfReverb::~tfReverb()
{
	for (int i=0; i<NUMCOMBS; i++)
    {
        eSAFE_DELETE_ARRAY(m_Comb[0][i].buffer);
        eSAFE_DELETE_ARRAY(m_Comb[1][i].buffer);
    }

    for (int i=0; i<NUMALLPASSES; i++)
    {
        eSAFE_DELETE_ARRAY(m_Allpass[0][i].buffer);
        eSAFE_DELETE_ARRAY(m_Allpass[1][i].buffer);
    }
}

tfIEffect::Mode tfReverb::getMode()
{
    return tfIEffect::FX_REVERB;
}

void tfReverb::alloc_comb_buffer(tfReverb::ReverbComb &comb, eU32 size)
{
    comb.bufidx = 0;
    comb.filterstore = 0.0f;
    comb.buffer = new eF32[size + 3];
    comb.bufsize = size;

    eMemSet(comb.buffer, 0, sizeof(eF32) * size + 3);
}

void tfReverb::alloc_allpass_buffer(tfReverb::ReverbAllpass &allpass, eU32 size)
{
    allpass.bufidx = 0;
    allpass.buffer = new eF32[size + 3];
    allpass.bufsize = size;

    eMemSet(allpass.buffer, 0, sizeof(eF32) * size + 3);
}

void tfReverb::setParams(eF32 *params)
{
    m_roomsize  = params[TF_REVERB_ROOMSIZE] * SCALEROOM + OFFSETROOM;
    m_damp      = params[TF_REVERB_DAMP] * SCALEDAMP;
    m_wet       = params[TF_REVERB_WET] * SCALEWET;
    m_dry       = (1.0f - params[TF_REVERB_WET]) * SCALEDRY;
    m_width     = params[TF_REVERB_WIDTH];
}

void tfReverb::update(eU32 sampleRate)
{
    m_wet1          = m_wet * (m_width / 2.0f + 0.5f);
    m_wet2          = m_wet * ((1.0f - m_width) / 2.0f);
    m_dry0          = m_dry;
    m_cmbFeedback   = m_roomsize;
    m_damp1         = m_damp;
    m_gain          = FIXEDGAIN;
    m_damp2         = 1.0f - m_damp1;
}

eBool tfReverb::process(eSignal **signals, eU32 len)
{
    eSignal *inputL = signals[0];
    eSignal *inputR = signals[1];

    //eSignalDebugWrite(signals, 1.0f, "d:\\dev\\reverb_pre.raw");

	eF32x2 damp2 = eSIMDSet2(m_damp2);
	eF32x2 damp1 = eSIMDSet2(m_damp1);
	eF32x2 wet1 = eSIMDSet2(m_wet1);
	eF32x2 wet2 = eSIMDSet2(m_wet2);
	eF32x2 dry0 = eSIMDSet2(m_dry0);
	eF32x2 cmbFeedback = eSIMDSet2(m_cmbFeedback);
	eF32x2 apsFeedback = eSIMDSet2(m_apsFeedback);

	while (len--)
    {
        eF32 saml = *inputL;
        eF32 samr = *inputR;

        eF32 input = (saml + samr) * m_gain;

		eF32x2 in = eSIMDLoad2(saml, samr);
		eF32x2 out = eSIMDSet2(0.0f);
		eF32x2 minput = eSIMDSet2(input);

        // Accumulate comb filters in parallel
        for (eU32 i = 0;i < NUMCOMBS;i++)
        {
			ReverbComb *comb1 = &m_Comb[0][i];
			ReverbComb *comb2 = &m_Comb[1][i];

			eF32x2 output = eSIMDLoad2(comb1->buffer[comb1->bufidx], comb2->buffer[comb2->bufidx]);
			eF32x2 filterstore = eSIMDLoad2(comb1->filterstore, comb2->filterstore);

			filterstore = eSIMDAdd(eSIMDMul(output, damp2), eSIMDMul(filterstore, damp1));

			eF32x2 buffer = eSIMDAdd(minput, eSIMDMul(filterstore, cmbFeedback));

			eSIMDStore(buffer, &comb1->buffer[comb1->bufidx], &comb2->buffer[comb2->bufidx]);
			eSIMDStore(filterstore, &comb1->filterstore, &comb2->filterstore);
			
			out = eSIMDAdd(out, output);

			comb1->bufidx++;
			comb2->bufidx++;

			if (comb1->bufidx >= comb1->bufsize)
				comb1->bufidx = 0;

			if (comb2->bufidx >= comb2->bufsize)
				comb2->bufidx = 0;
        }

        // Feed through allpasses in series
        for (eU32 i = 0;i < NUMALLPASSES;i++)
        {
			ReverbAllpass *apass1 = &m_Allpass[0][i];
			ReverbAllpass *apass2 = &m_Allpass[1][i];

			eF32x2 bufout = eSIMDLoad2(apass1->buffer[apass1->bufidx], apass2->buffer[apass2->bufidx]);
			eF32x2 buffer = eSIMDAdd(out, eSIMDMul(bufout, apsFeedback));

			out = eSIMDSub(bufout, out);

			eSIMDStore(buffer, &apass1->buffer[apass1->bufidx], &apass2->buffer[apass2->bufidx]);

			apass1->bufidx++;
			apass2->bufidx++;

			if (apass1->bufidx >= apass1->bufsize)
				apass1->bufidx = 0;

			if (apass2->bufidx >= apass2->bufsize)
				apass2->bufidx = 0;
        }

        // Calculate output
		out = eSIMDAdd(
				eSIMDAdd(
					eSIMDMul(out, wet1),
					eSIMDMul(out, wet2)),
				eSIMDMul(in, dry0));

		eSIMDStore(out, inputL, inputR);

        inputL++;
        inputR++;
    }

    //eSignalDebugWrite(signals, 1.0f, "d:\\dev\\reverb_post.raw");

    return eTRUE;
}

// -----------------------------------------------------------------------
//    Formant
// -----------------------------------------------------------------------

tfFormantFx::tfFormantFx()
{
	m_mode = 0;
    m_wet = 1.0f;
    eMemSet(m_memoryL, 0, sizeof(eF64) * 10);
	eMemSet(m_memoryR, 0, sizeof(eF64) * 10);
}

tfFormantFx::~tfFormantFx()
{

}

tfIEffect::Mode tfFormantFx::getMode()
{
    return tfIEffect::FX_FORMANT;
}

void tfFormantFx::setParams(eF32 *params)
{
    m_mode      = eFtoL(params[TF_FORMANT_MODE] * 4.0f);
    m_wet       = params[TF_FORMANT_WET];
}

void tfFormantFx::update(eU32 sampleRate)
{
    
}

eBool tfFormantFx::process(eSignal **signal, eU32 len)
{
    const eF64 coeff[5][11]= {
		{ 3.11044e-06,
			8.943665402,    -36.83889529,    92.01697887,    -154.337906,    181.6233289,
			-151.8651235,   89.09614114,    -35.10298511,    8.388101016,    -0.923313471  ///A
		},
		{4.36215e-06,
		8.90438318,    -36.55179099,    91.05750846,    -152.422234,    179.1170248,  ///E
		-149.6496211,87.78352223,    -34.60687431,    8.282228154,    -0.914150747
		},
		{ 3.33819e-06,
		8.893102966,    -36.49532826,    90.96543286,    -152.4545478,    179.4835618,
		-150.315433,    88.43409371,    -34.98612086,    8.407803364,    -0.932568035  ///I
		},
		{1.13572e-06,
		8.994734087,    -37.2084849,    93.22900521,    -156.6929844,    184.596544,   ///O
		-154.3755513,    90.49663749,    -35.58964535,    8.478996281,    -0.929252233
		},
		{4.09431e-07,
		8.997322763,    -37.20218544,    93.11385476,    -156.2530937,    183.7080141,  ///U
		-153.2631681,    89.59539726,    -35.12454591,    8.338655623,    -0.910251753
		}
	}; 

	eF32 wet_inv = 1.0f - m_wet;
		
	eSignal *in1  = signal[0];
	eSignal *in2  = signal[1];

	const eF64 *co_vow = coeff[m_mode];
	eF64 res = 0.0f;

	while (len--)
	{
		// LEFT CHANNEL

		res = ( co_vow[0]  * (eF64)*in1 +
				co_vow[1]  * m_memoryL[0] +  
				co_vow[2]  * m_memoryL[1] +
				co_vow[3]  * m_memoryL[2] +
				co_vow[4]  * m_memoryL[3] +
				co_vow[5]  * m_memoryL[4] +
				co_vow[6]  * m_memoryL[5] +
				co_vow[7]  * m_memoryL[6] +
				co_vow[8]  * m_memoryL[7] +
				co_vow[9]  * m_memoryL[8] +
				co_vow[10] * m_memoryL[9] );

		m_memoryL[9]= m_memoryL[8];
		m_memoryL[8]= m_memoryL[7];
		m_memoryL[7]= m_memoryL[6];
		m_memoryL[6]= m_memoryL[5];
		m_memoryL[5]= m_memoryL[4];
		m_memoryL[4]= m_memoryL[3];
		m_memoryL[3]= m_memoryL[2];
		m_memoryL[2]= m_memoryL[1];                    
		m_memoryL[1]= m_memoryL[0];
		m_memoryL[0]= res;

		*in1 *= wet_inv;
		*in1++ += (eF32)res * m_wet;

		// RIGHT CHANNEL

		res = ( co_vow[0]  * (eF64)*in2 +
				co_vow[1]  * m_memoryR[0] +  
				co_vow[2]  * m_memoryR[1] +
				co_vow[3]  * m_memoryR[2] +
				co_vow[4]  * m_memoryR[3] +
				co_vow[5]  * m_memoryR[4] +
				co_vow[6]  * m_memoryR[5] +
				co_vow[7]  * m_memoryR[6] +
				co_vow[8]  * m_memoryR[7] +
				co_vow[9]  * m_memoryR[8] +
				co_vow[10] * m_memoryR[9] );

		m_memoryR[9]= m_memoryR[8];
		m_memoryR[8]= m_memoryR[7];
		m_memoryR[7]= m_memoryR[6];
		m_memoryR[6]= m_memoryR[5];
		m_memoryR[5]= m_memoryR[4];
		m_memoryR[4]= m_memoryR[3];
		m_memoryR[3]= m_memoryR[2];
		m_memoryR[2]= m_memoryR[1];                    
		m_memoryR[1]= m_memoryR[0];
		m_memoryR[0]= res;
			
		*in2 *= wet_inv;
		*in2++ += (eF32)res * m_wet;
	}

    return eTRUE;
}

// -----------------------------------------------------------------------
//    EQ
// -----------------------------------------------------------------------

tfEqFx::tfEqFx()
{
	m_state = (tfEqFx::State*)eMemAllocAlignedAndZero(sizeof(tfEqFx::State), 16);
}

tfEqFx::~tfEqFx()
{
	eFreeAligned(m_state);
}

tfIEffect::Mode tfEqFx::getMode()
{
    return tfIEffect::FX_EQ;
}

void tfEqFx::setParams(eF32 *params)
{
    m_lg = params[TF_EQ_LOW];
    m_mg = params[TF_EQ_MID];
    m_hg = params[TF_EQ_HIGH];

    if (m_lg <= 0.5f) 
        m_lg *= 2.0f; 
    else 
    {
        m_lg = (m_lg - 0.5f) * 2.0f;
        m_lg *= m_lg;
        m_lg *= 10.0f;
        m_lg += 1.0f;
    }

    if (m_mg <= 0.5f) 
        m_mg *= 2.0f; 
    else 
    {
        m_mg = (m_mg - 0.5f) * 2.0f;
        m_mg *= m_mg;
        m_mg *= 10.0f;
        m_mg += 1.0f;
    }

    if (m_hg <= 0.5f) 
        m_hg *= 2.0f; 
    else 
    {
        m_hg = (m_hg - 0.5f) * 2.0f;
        m_hg *= m_hg;
        m_hg *= 10.0f;
        m_hg += 1.0f;
    }

    // Calculate filter cutoff frequencies
    m_lf = 2.0f * eSin(ePI * (880.0f / 44100.0f)); 
    m_hf = 2.0f * eSin(ePI * (5000.0f / 44100.0f));
}

void tfEqFx::update(eU32 sampleRate)
{
    
}

eBool tfEqFx::process(eSignal **signal, eU32 len)
{
	eSignal *in1 = signal[0];
	eSignal *in2 = signal[1];
	
	eF32x2 lf = eSIMDSet2(m_lf);
	eF32x2 hf = eSIMDSet2(m_hf);
	eF32x2 lg = eSIMDSet2(m_lg);
	eF32x2 mg = eSIMDSet2(m_mg);
	eF32x2 hg = eSIMDSet2(m_hg);
	
	while(len--)
	{
		eF32x2 val = eSIMDLoad2(*in1, *in2);
		
		// Locals
		eF32x2  l,m,h;      // Low / Mid / High - Sample Values

		// Filter #1 (lowpass)
		//m_f1p0  += (lf * (val   - m_f1p0));
		m_state->m_f1p0 = eSIMDMulAdd(m_state->m_f1p0, lf, eSIMDSub(val, m_state->m_f1p0));
		//m_f1p1  += (lf * (m_f1p0 - m_f1p1));
		m_state->m_f1p1 = eSIMDMulAdd(m_state->m_f1p1, lf, eSIMDSub(m_state->m_f1p0, m_state->m_f1p1));
		//m_f1p2  += (lf * (m_f1p1 - m_f1p2));
		m_state->m_f1p2 = eSIMDMulAdd(m_state->m_f1p2, lf, eSIMDSub(m_state->m_f1p1, m_state->m_f1p2));
		//m_f1p3  += (lf * (m_f1p2 - m_f1p3));
		m_state->m_f1p3 = eSIMDMulAdd(m_state->m_f1p3, lf, eSIMDSub(m_state->m_f1p2, m_state->m_f1p3));
		
		l = m_state->m_f1p3;

		// Filter #2 (highpass)
		//m_f2p0  += (hf * (val   - m_f2p0));
		m_state->m_f2p0 = eSIMDMulAdd(m_state->m_f2p0, hf, eSIMDSub(val, m_state->m_f2p0));
		//m_f2p1  += (hf * (m_f2p0 - m_f2p1));
		m_state->m_f2p1 = eSIMDMulAdd(m_state->m_f2p1, hf, eSIMDSub(m_state->m_f2p0, m_state->m_f2p1));
		//m_f2p2  += (hf * (m_f2p1 - m_f2p2));
		m_state->m_f2p2 = eSIMDMulAdd(m_state->m_f2p2, hf, eSIMDSub(m_state->m_f2p1, m_state->m_f2p2));
		//m_f2p3  += (hf * (m_f2p2 - m_f2p3));
		m_state->m_f2p3 = eSIMDMulAdd(m_state->m_f2p3, hf, eSIMDSub(m_state->m_f2p2, m_state->m_f2p3));
		
		h = eSIMDSub(m_state->m_sdm3, m_state->m_f2p3);

		// Calculate midrange (signal - (low + high))
		m = eSIMDSub(m_state->m_sdm3, eSIMDAdd(h, l));

		// Scale, Combine and store
		l = eSIMDMul(l, lg);
		m = eSIMDMul(m, mg);
		h = eSIMDMul(h, hg);

		// Shuffle history buffer 
		m_state->m_sdm3 = m_state->m_sdm2;
		m_state->m_sdm2 = m_state->m_sdm1;
		m_state->m_sdm1 = val;                

		eF32x2 out = eSIMDAdd(eSIMDAdd(l, m), h);
		
		eSIMDStore(out, in1, in2);
		in1++;
		in2++;
	}

    return eTRUE;
}