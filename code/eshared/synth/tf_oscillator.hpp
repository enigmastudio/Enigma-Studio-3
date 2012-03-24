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

#ifndef TF_OSCILLATOR_HPP
#define TF_OSCILLATOR_HPP

class tfSpline
{
public:
    enum Mode
    {
        MODE_STEP,
        MODE_LINEAR,
        MODE_SPLINE,
        MODE_COUNT
    };

public:
    tfSpline();

    void        update(eF32 *params, tfModMatrix *modMatrix, tfModMatrix::State *modMatrixState);
    eF32        calc(eF32 pos);

    eU32        getPointCount() const;
    eVector2    getPoint(eU32 index) const;
	void		setPoint(eU32 index, eVector2 p);

private:
    void        _setValue(eF32 &var, eF32 value, eBool &updated);
    void        _setValue(Mode &var, Mode value, eBool &updated);
    void        _solveLinear();

public:
    eU32        m_pointCount;
    eVector2    m_wps[TF_OSCILLATOR_POINTS];
    Mode        m_modes[TF_OSCILLATOR_POINTS];
    eF32        m_a[TF_OSCILLATOR_POINTS]; // Coefficients of x^3.
    eF32        m_b[TF_OSCILLATOR_POINTS]; // Coefficients of x^2.
    eF32        m_c[TF_OSCILLATOR_POINTS]; // Coefficients of x^1.
    eF32        m_samples[TF_OSCILLATOR_SAMPLES + 1];
};

class tfOscillator
{
public:
	struct SubOscState
    {
        SubOscState()
        {
            reset(eFALSE);    
        }

        void reset(eBool randomizePhase);

        eF32 phase1;
        eF32 phase2;
        eF32 freq1;
        eF32 freq2;
        eF32 average1;
        eF32 average2;
	};

	struct State
	{
		State()
        {
            reset();    
        }

        void reset()
        {
			for(eU32 i=0;i<TF_SUBOSC * TF_MAXUNISONO;i++)
            {
                oscState[i].reset(i > 0);
            }

            lastVolume1 = 0.0f;
            lastVolume2 = 0.0f;
        }

        eF32 lastVolume1;
        eF32 lastVolume2;

		SubOscState oscState[(TF_SUBOSC+1) * TF_MAXUNISONO];
	};

public:
    eBool               process(tfOscillator::State *state, tfModMatrix *modMatrix, 
                                tfModMatrix::State *modMatrixState, eF32 *params, 
                                eSignal **signals, eU32 len, eF32 baseFreq, eF32 velocity, eU32 oversamplingCount);

    void                processSingle(tfOscillator::SubOscState *sostate, eSignal **signals, eF32 volume, eU32 len);

    const tfSpline &	getSpline() const;
    tfSpline &			getSpline();

private:
    tfSpline			m_spline;
};

#endif // TF_OSCILLATOR_HPP