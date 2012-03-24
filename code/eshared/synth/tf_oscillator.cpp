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

void tfOscillator::SubOscState::reset(eBool randomizePhase)
{
	phase1 = 0.0f;
	phase2 = 0.0f;

	if (randomizePhase)
	{
		eU32 seed = eRandomSeed();
		phase1 = eRandomF(seed);
		phase2 = eRandomF(seed);
	}

	freq1 = 0.0f;
	freq2 = 0.0f;
	average1 = 0.0f;
	average2 = 0.0f;
}

tfSpline::tfSpline() :
    m_pointCount(0)
{
    eMemSet(m_modes, 0, sizeof(Mode)*TF_OSCILLATOR_POINTS);
}

void tfSpline::update(eF32 *params, tfModMatrix *modMatrix, tfModMatrix::State *modMatrixState)
{
    eBool updated = eFALSE;
    eU32 points = eFtoL(eRound(params[TF_OSC_POINTCOUNT] * (TF_OSCILLATOR_POINTS-3))) + 1;

    if (points + 2 != m_pointCount)
    {
        m_pointCount = points + 2;
        updated = eTRUE;
    }

    // define first and last point
    m_wps[0].set(0.0f, 0.0f);
    m_wps[points+1].set(1.0f, 0.0f);

    _setValue(m_modes[points+1], (Mode)eFtoL(eRound(params[TF_OSC_FINAL_INTERPOLATION] * (MODE_COUNT-1))), updated);

    eF32 last_x = 0.0f;
    for(eU32 i=0;i<points; i++)
    {
        // calculate x position
        eF32 px = params[TF_OSC_POINT1_OFFSET + i*3];
        if (modMatrix)
            px *= modMatrix->get(modMatrixState, (tfModMatrix::Output)(tfModMatrix::OUTPUT_P1OFFSET + i*2));
        px = eClamp<eF32>(0.01f, px, 0.99f);
        px = (1.0f - last_x) * px + last_x;
        last_x = px;

        // calculate y position
        eF32 py = params[TF_OSC_POINT1_VALUE + i*3];
        if (modMatrix)
            py *= modMatrix->get(modMatrixState, (tfModMatrix::Output)(tfModMatrix::OUTPUT_P1VALUE + i*2));
        py = py * 2.0f - 1.0f;

        // get interpolation mode
        Mode mode = (Mode)eFtoL(eRound(params[TF_OSC_POINT1_INTERPOLATION + i*3] * (MODE_COUNT-1))); 

        // write values
        _setValue(m_wps[i+1].x, px, updated);
        _setValue(m_wps[i+1].y, py, updated);
        _setValue(m_modes[i+1], mode, updated);
    }

    if (updated)
    {
        _solveLinear();
    }
}

eF32 tfSpline::calc(eF32 pos)
{
    eASSERT(pos >= 0.0f && pos <= 1.0f);

    eU32 i = 1;

    // Find active control point.
    while (pos > m_wps[i].x && i <= m_pointCount)
    {
        i++;
    }

    const eU32 k = i-1;

    switch (m_modes[i])
    {
        case MODE_STEP:
        {
            return m_wps[k].y;
        }

        case MODE_LINEAR:
        {
            const eF32 t = (pos-m_wps[k].x)/(m_wps[i].x-m_wps[k].x);
            return eLerp(m_wps[k].y, m_wps[i].y, t);
        }

        case MODE_SPLINE:
        {
            const eF32 t = pos-m_wps[k].x;
            return ((m_a[k]*t+m_b[k])*t+m_c[k])*t+m_wps[k].y;
        }
    }

    return 0.0f;
}

eU32 tfSpline::getPointCount() const
{
    return m_pointCount;
}

eVector2 tfSpline::getPoint(eU32 index) const
{
    eASSERT(index < m_pointCount);
    return m_wps[index];
}

void tfSpline::setPoint(eU32 index, eVector2 p)
{
    eASSERT(index < m_pointCount);
    m_wps[index] = p;
}

void tfSpline::_setValue(eF32 &var, eF32 value, eBool &updated)
{
    if (var != value)
    {
        var = value;
        updated = eTRUE;
    }
}

void tfSpline::_setValue(Mode &var, Mode value, eBool &updated)
{
    if (value == MODE_SPLINE)
    {
        updated = eTRUE;
    }

    var = value;
}

void tfSpline::_solveLinear()
{
    // Calculate differences between control points.
    eF32 dx[TF_OSCILLATOR_POINTS], dy[TF_OSCILLATOR_POINTS];

    for (eU32 i=0; i<m_pointCount; i++)
    {
        dx[i] = (m_wps[i+1].x-m_wps[i].x) + 0.0001f;
        dy[i] = (m_wps[i+1].y-m_wps[i].y) + 0.0001f;
    }

    // Calculate right side of equation system.
    eF32 r[TF_OSCILLATOR_POINTS];

    for (eU32 i=1; i<m_pointCount; i++)
    {
        r[i] = (dy[i]/dx[i]-dy[i-1]/dx[i-1])*3.0f;
    }

    // Calculate main diagonal of equation system.
    eF32 z[TF_OSCILLATOR_POINTS];

    for (eU32 i=1; i<=m_pointCount-1; i++)
    {
        z[i] = (dx[i-1]+dx[i])*2.0f;
    }

    // Process of eliminating variables.
    for (eU32 i=2; i<m_pointCount; i++)
    {
        r[i] -= r[i-1]*dx[i-1]/z[i-1];
        z[i] -= dx[i-1]*dx[i-1]/z[i-1];
    }

    // Solve for m_b[1], m_b[2], ...
    m_b[0] = 0.0f;
    m_b[m_pointCount] = 0.0f;

    for (eInt i=(eInt)m_pointCount-1; i>=1; i--)
    {
        m_b[i] = (r[i]-dx[i]*m_b[i+1])/z[i];
    }

    // Calculate spline coefficients.
    for (eU32 i=0; i<m_pointCount; i++)
    {
        m_c[i] = dy[i]/dx[i]-dx[i]*(m_b[i+1]+2.0f*m_b[i])/3.0f;
        m_a[i] = (m_b[i+1]-m_b[i])/(3.0f*dx[i]);
    }

    // calc samples
    eF32 average = 0.0f;
    for(eU32 i = 0; i <= TF_OSCILLATOR_SAMPLES; i++) 
    {
        eF32 t = (eF32)i / (eF32)TF_OSCILLATOR_SAMPLES;
        m_samples[i] = calc(t);
        average += m_samples[i];
    }

    eF32 offset = average / TF_OSCILLATOR_SAMPLES;
    for(eU32 i = 0; i <= TF_OSCILLATOR_SAMPLES; i++) 
    {
        m_samples[i] -= offset;
    }
}

// Implementation of oscillator class.

eBool tfOscillator::process(tfOscillator::State *state, tfModMatrix *modMatrix, 
                           tfModMatrix::State *modMatrixState, eF32 *params, 
                           eSignal **signals, eU32 len, eF32 baseFreq, eF32 velocity, eU32 oversamplingCount)
{
    eF32 vol = params[TF_OSC_VOLUME] * velocity;

    if (vol > 0.01f || state->lastVolume1 > 0.01f || state->lastVolume2 > 0.01f)
    {
        eF32 freq       = params[TF_OSC_FREQ];
        eF32 drive      = params[TF_OSC_DRIVE];
        eF32 panning    = params[TF_OSC_PAN];
        eU32 subosc     = eFtoL(eRound(params[TF_OSC_SUBOSC] * TF_MAXSUBOSC)) + 1;
        eF32 detune     = params[TF_OSC_DETUNE];
        eU32 unisono    = eFtoL(eRound(params[TF_OSC_UNISONO] * TF_MAXUNISONO)) + 1;
        eF32 spread     = params[TF_OSC_SPREAD];
        eF32 octave     = params[TF_OSC_OCTAVE];
        eBool notefreq  = freq < 0.00001f;

        // Process modulation matrix.
        vol     *= modMatrix->get(modMatrixState, tfModMatrix::OUTPUT_VOLUME);
        detune  *= modMatrix->get(modMatrixState, tfModMatrix::OUTPUT_DETUNE);
        drive   *= modMatrix->get(modMatrixState, tfModMatrix::OUTPUT_DRIVE);
        panning *= modMatrix->get(modMatrixState, tfModMatrix::OUTPUT_PAN);
        spread  *= modMatrix->get(modMatrixState, tfModMatrix::OUTPUT_SPREAD);
        eF32 freqMod = modMatrix->get(modMatrixState, tfModMatrix::OUTPUT_FREQ);
        freq *= freqMod;

        if (vol <= 0.01f && state->lastVolume1 <= 0.01f && state->lastVolume2 <= 0.01f)
            return eFALSE;

        // Scale some of the values.
        detune  = ePow(detune, 3) / 5000.0f;
        drive   = ((drive * drive * drive) * 32.0f) + 1.0f;
        freq    = ePow(freq, 2) / 100.0f;
        spread  = ePow(spread, 4) * 0.0001f;

#ifdef TF_OVERSAMPLING
		detune /= oversamplingCount;
		spread /= oversamplingCount;
		freq   /= oversamplingCount;
#endif

        // Calculate octave multiplicator.
        eU32 ioctave = eFtoL(octave * (TF_MAXOCTAVES-1));
        eF32 octave_mul = TF_OCTAVES[ioctave];

        // Calculate final frequency.
        eF32 freq1 = (baseFreq * octave_mul + detune) * freqMod;
        eF32 freq2 = (baseFreq * octave_mul - detune) * freqMod;

        if (!notefreq)
        {
            freq1 = freq;
            freq2 = freq;
        }

        // Update the spline.
        // ---------------------------------------------------------------------------------
        m_spline.update(params, modMatrix, modMatrixState);

        // calculate panning, volume and volume step values
        // ---------------------------------------------------------------------------------
        eF32 pan1 = panning > 0.5f ? (1.0f - panning) * 2.0f : 1.0f;
		eF32 pan2 = panning < 0.5f ? panning * 2.0f : 1.0f; 
		
		eF32 volumeL = pan1 * vol;
		eF32 volumeR = pan2 * vol;

		eF32 vol1_step = (volumeL - state->lastVolume1) / len;
		eF32 vol1 = state->lastVolume1;
		state->lastVolume1 = volumeL;

		eF32 vol2_step = (volumeR - state->lastVolume2) / len;
		eF32 vol2 = state->lastVolume2;
		state->lastVolume2 = volumeR;

        // run the oscillators
        // ---------------------------------------------------------------------------------
        for (eU32 i=0; i<subosc; i++)
        {
            for (eU32 j=0; j<unisono; j++)
            {
                tfOscillator::SubOscState &sostate = state->oscState[j * TF_SUBOSC + i];

                sostate.freq1 = freq1 + spread * j;
                sostate.freq2 = freq2 - spread * j;
                                
                processSingle(&sostate, signals, 1.0f / (i+1), len);
            }

            freq1 *= 0.5f - detune;
            freq2 *= 0.5f + detune;
        }
        
        // calculate volume, drive and saturation
        // ---------------------------------------------------------------------------------
        eSignal *sig1 = signals[0];
        eSignal *sig2 = signals[1];
        eF32x2 vol_step = eSIMDLoad2(vol1_step, vol2_step);
        eF32x2 drv = eSIMDSet2(drive);
        eF32x2 const_1 = eSIMDSet2(1.0f);
        eF32x2 const_n1 = eSIMDSet2(-1.0f);
        eF32x2 volume = eSIMDLoad2(vol1, vol2);

        eSIMDUndenormalise(vol_step);

        while (len--)
        {
            eF32x2 val = eSIMDLoad2(*sig1, *sig2);

            val = eSIMDMul(
                    eSIMDMax(
                        eSIMDMin(
                            eSIMDMul(val, drv), 
                            const_1
                        ), 
                        const_n1
                    ),
                    volume
                );

            eSIMDStore(val, sig1, sig2);
            sig1++;
            sig2++;

            volume = eSIMDAdd(volume, vol_step);
        }

        return eTRUE;
    }

    return eFALSE;
}

void tfOscillator::processSingle(tfOscillator::SubOscState *state, eSignal **signals, eF32 volume, eU32 len)
{
    eSignal *sig1 = signals[0];
    eSignal *sig2 = signals[1];

    while (len--)
    {
        eASSERT((state->phase1 >= 0.0f) && (state->phase1 <= 1.0f));
        eASSERT((state->phase2 >= 0.0f) && (state->phase2 <= 1.0f));
        eU32 spos1 = eFtoL(state->phase1 * (eF32)TF_OSCILLATOR_SAMPLES);
        eU32 spos2 = eFtoL(state->phase2 * (eF32)TF_OSCILLATOR_SAMPLES);
        eSignal val1 = m_spline.m_samples[spos1];
        eSignal val2 = m_spline.m_samples[spos2];

        *sig1++ += val1 * volume;
        *sig2++ += val2 * volume;

        state->phase1 += state->freq1;
        if (state->phase1 > 1.0f)
            state->phase1 -= 1.0f;

        state->phase2 += state->freq2;
        if (state->phase2 > 1.0f)
            state->phase2 -= 1.0f;
    }
}

const tfSpline & tfOscillator::getSpline() const
{
    return m_spline;
}

tfSpline & tfOscillator::getSpline()
{
    return m_spline;
}