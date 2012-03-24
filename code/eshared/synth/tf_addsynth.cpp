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

#ifndef NO_ADDSYNTH

static eU32 g_noiseCounter = 0;
static eF32 *g_rnd = eNULL;
    
static void genRand()
{
    if (g_rnd == eNULL)
    {
        g_rnd = new eF32[2048*32];
        eASSERT(g_rnd != eNULL);
        
		eU32 seed = eRandomSeed();
        for (eU32 i=0; i<2048*32; i++)
        {
            g_rnd[i] = eRandomF(0.0f, eTWOPI, seed);
        }
    }
}

eFORCEINLINE static eF32 noise()
{
    eF32 n = g_rnd[g_noiseCounter++];

    if (g_noiseCounter >= 2048*32)
    {
        g_noiseCounter = 0;
    }
    
    return n;
}

void tfAddSynth::State::reset()
{
	eU32 seed = eRandomSeed();
	phase1 = eRandomF(seed);
	phase2 = eRandomF(seed);
	freq1 = 0.0f;
	freq2 = 0.0f;
}

tfAddSynth::tfAddSynth() :
    m_padSynthTable(eNULL),
    m_padSynthTableTemp(eNULL),
    m_bandwidth(-1.0f),
    m_damp(-1.0f),
    m_numHarmonics(-1.0f),
    m_bwScale(-1.0f),
    m_profile(0),
    m_sampleRate(0),
    m_padRefresh(eFALSE)
{
}

tfAddSynth::~tfAddSynth()
{
    eSAFE_DELETE_ARRAY(m_padSynthTable);
    eSAFE_DELETE_ARRAY(m_padSynthTableTemp);
}

void tfAddSynth::update(eF32 *params, eU32 sampleRate)
{
    eF32 volume = params[TF_ADD_VOLUME];

    if (volume > 0.0f)
    {
        eF32 bandwidth = params[TF_ADD_BANDWIDTH];
        eF32 damp = params[TF_ADD_DAMP];
        eF32 scale = params[TF_ADD_SCALE];
        eF32 harmonics = params[TF_ADD_HARMONICS];
        eU32 profile = eFtoL(eRound(params[TF_ADD_PROFILE] * 3.0f));

        harmonics /= 4.0f;
        scale *= scale;

        gen(bandwidth, scale, damp, harmonics, profile, sampleRate);
        check();
    }
}

eBool tfAddSynth::process(State *state, 
                    tfModMatrix *modMatrix, 
                    tfModMatrix::State *modMatrixState, 
                    eF32 *params, 
                    eSignal **signals, 
                    eU32 len, 
                    eF32 baseFreq, 
                    eF32 velocity, 
                    eU32 oversamplingCount)
{
    eF32 vol = params[TF_ADD_VOLUME];

    if (vol > 0.0f)
    {
        eF32 freq       = params[TF_OSC_FREQ];
        eF32 drive      = params[TF_ADD_DRIVE];
        eF32 detune     = params[TF_OSC_DETUNE];
        eF32 octave     = params[TF_ADD_OCTAVE];
        eBool notefreq  = freq < 0.00001f;

        // process modulation matrix
        // -------------------------------------------------
        vol     *= modMatrix->get(modMatrixState, tfModMatrix::OUTPUT_VOLUME);
        detune  *= modMatrix->get(modMatrixState, tfModMatrix::OUTPUT_DETUNE);
        drive   *= modMatrix->get(modMatrixState, tfModMatrix::OUTPUT_DRIVE);
        freq    *= modMatrix->get(modMatrixState, tfModMatrix::OUTPUT_FREQ);

        if (vol < 0.01f)
            return eFALSE;
        
        // scale some of the values
        // -------------------------------------------------
        detune  = ePow(detune, 3.0f) / 5000.0f;
        drive   *= 32.0f;
        drive    += 1.0f;
        freq    = ePow(freq, 2.0f) / 100.0f;

#ifdef TF_OVERSAMPLING
		detune /= oversamplingCount;
		freq   /= oversamplingCount;
#endif
        
        // calculate octave multiplicator
        // -------------------------------------------------
        eU32 ioctave = eFtoL(octave * (TF_MAXOCTAVES-1));
        eF32 octave_mul = TF_OCTAVES[ioctave];

        // calculate final frequency
        // -------------------------------------------------
        state->freq1 = baseFreq * octave_mul + detune;
        state->freq2 = baseFreq * octave_mul - detune;

        if (!notefreq)
        {
            state->freq1 = freq;
            state->freq2 = freq;
        }

        state->freq1 /= 260.0f;
        state->freq2 /= 260.0f;

        // calculate signal
        // -------------------------------------------------

        eSignal *sig1 = signals[0];
        eSignal *sig2 = signals[1];

        eF32 offset_f = TF_ADDSYNTHTABLESIZE / 2.0f;

		eF32x2 mdrive = eSIMDSet2(drive);
		eF32x2 mvol = eSIMDSet2(vol);
		eF32x2 mmin = eSIMDSet2(-1.0f);
		eF32x2 mmax = eSIMDSet2(1.0f);

        while(len--)
        {
            eU32 off1 = eFtoL(state->phase1 * offset_f);
            eU32 off2 = eFtoL(state->phase2 * offset_f);

			eF32 val1 = m_padSynthTable[off1];
			eF32 val2 = m_padSynthTable[off2];

			eF32x2 mval = eSIMDMul(
							eSIMDMax(
								eSIMDMin(
									eSIMDMul(
										eSIMDLoad2(val1, val2), 
										mdrive),
									mmax),
								mmin),
							mvol);
			
			eF32A store_out[4];
            _mm_store_ps(store_out, mval);
            
            *sig1++ += store_out[3];
            *sig2++ += store_out[2];

            state->phase1 += state->freq1;
			while (state->phase1 > 1.0f) { state->phase1 -= 1.0f; }

			state->phase2 += state->freq2;
			while (state->phase2 > 1.0f) { state->phase2 -= 1.0f; }
        }

        return eTRUE;
    }

    return eFALSE;
}

eF32 tfAddSynth::profileGauss(eF32 fi, eF32 bwi)
{
    eF32 x=fi/bwi;
    x*=x;
    //this avoids computing the e^(-x^2) where it's results are very close to zero
    if (x>14.71280603) return 0.0;
    return eExp(-x)/bwi;
}

eF32 tfAddSynth::profileSingle(eF32 fi, eF32 bwi)
{
    eF32 x=fi/bwi;
    if (x>0.1f || x < -0.1f) return 0.0f;
    return 1.0f/bwi;
}

eF32 tfAddSynth::profileDetune(eF32 fi, eF32 bwi)
{
    eF32 x=fi/bwi;
    if (x<-0.5f || (x > -0.4f && x < 0.4f) || x > 0.5f) return 0.0f;
    return 1.0f/bwi;
}

eF32 tfAddSynth::profileSpread(eF32 fi, eF32 bwi)
{
    eF32 x=fi/bwi;
    if (x>5.0f || x < -5.0f) return 0.0f;
    return 1.0f/bwi;
}

eINLINE void tfAddSynth::normalize(eInt N, eF32 *smp)
{
    eF32 max=0.0;
    for (eInt i=0;i<N/2;i++) 
    {
        eF32 abs_smp = eAbs(smp[i]);

        if (abs_smp > max) 
            max=abs_smp;
    }
    if (max<1e-5f) max=1e-5f;
        
    max = 1.0f/(max*1.4142f);
    for (eInt i=0;i<N;i++) 
    {
        smp[i]*=max;
    }
}

void tfAddSynth::extendedAlgorithm(eInt N, eInt profile, eInt sampleRate, eF32 f, eF32 bw, eF32 bwscale, eInt number_harmonics, eF32 *A, eF32 *smp)
{
    eInt i,nh;
    eInt NH=N>>1;
    eF32 *freq_amp     = new eF32[NH];
    eF32 *freq_phase   = new eF32[NH];
    eMemSet(freq_amp,0,sizeof(eF32)*NH);
    //eF32 recdsr = 1.0f/(2.0f*sampleRate);
    eF32 recsr = 1.0f/sampleRate;
    eF32 recdsr = recsr/2.0f;
    eF32 Nrec = 1.0f/(eF32)N;
    eF32 precalcbw = (ePow(2.0f,bw / 1200.0f)-1.0f)*f;
    bwscale *= 19.0f;
    bwscale += 1.0f;

    eF32 (*profile_func)(eF32 fi, eF32 bwi);

    switch (profile)
    {
    default:
    case 0: profile_func = profileSingle; break;
    case 1: profile_func = profileDetune; break;
    case 2: profile_func = profileSpread; break;
    case 3: profile_func = profileGauss; break;
    } 

    for (nh=1;nh<number_harmonics;nh++)
    {
        eF32 bw_Hz; //bandwidth of the current harmonic measured in Hz
        eF32 bwi;
        eF32 fi;

        //bw_Hz=precalcbw*nh;
        bw_Hz = precalcbw*ePow((eF32)nh, bwscale);

        bwi = bw_Hz *recdsr;
        fi  = f * nh * recsr;
        //fi  = f * nh * recsr;

        for (i=0;i<NH;i++)
        {
            eF32 hprofile;

            hprofile = profile_func((i*Nrec)-fi,bwi);
            freq_amp[i] += hprofile*A[nh];
        }
    }

    //Add random phases
    for (i=0;i<NH;i++)
    {
        freq_phase[i] = noise();
    }

    ifft(N,freq_amp,freq_phase,smp);
    normalize(N,smp);

    eSAFE_DELETE_ARRAY(freq_amp);
    eSAFE_DELETE_ARRAY(freq_phase);
}

void tfAddSynth::ifft(eInt N, eF32 *freq_amp, eF32 *freq_phase, eF32 *smp)
{
    eF32 *freqs = new eF32[N];
    eInt NH = N>>1;

    for (eInt i=0;i<NH;i++)
    {
        freqs[i*2]   = freq_amp[i] * eCos(freq_phase[i]);
        freqs[i*2+1] = freq_amp[i] * eSin(freq_phase[i]);
    }

    fft(freqs, NH, 1);

    for (eInt j=0;j<NH;j++)
        smp[j] = freqs[j*2];
}

void tfAddSynth::fft(eF32 *fftBuffer, eInt fftFrameSize, eInt sign)
{
    eF32 wr, wi, arg, *p1, *p2, temp;
    eF32 tr, ti, ur, ui, *p1r, *p1i, *p2r, *p2i;
    long i, bitm, j, le, le2, k;

    for (i = 2; i < 2*fftFrameSize-2; i += 2) 
    {
        for (bitm = 2, j = 0; bitm < 2*fftFrameSize; bitm <<= 1) 
        {
            if (i & bitm) j++;
            j <<= 1;
        }

        if (i < j) 
        {
            p1 = fftBuffer+i; p2 = fftBuffer+j;
            temp = *p1; *(p1++) = *p2;
            *(p2++) = temp; temp = *p1;
            *p1 = *p2; *p2 = temp;
        }
    }
    for (k = 0, le = 2; k < eFtoL(eLog((eF32)fftFrameSize)/eLog(2.0f)); k++) 
    {
        le <<= 1;
        le2 = le>>1;
        ur = 1.0;
        ui = 0.0;
        arg = ePI / (le2>>1);
        wr = (eF32)eCos(arg);
        wi = (eF32)(sign*eSin(arg));

        for (j = 0; j < le2; j += 2) 
        {
            p1r = fftBuffer+j; p1i = p1r+1;
            p2r = p1r+le2; p2i = p2r+1;

            for (i = j; i < 2*fftFrameSize; i += le) 
            {
                tr = *p2r * ur - *p2i * ui;
                ti = *p2r * ui + *p2i * ur;
                *p2r = *p1r - tr; *p2i = *p1i - ti;
                *p1r += tr; *p1i += ti;
                p1r += le; p1i += le;
                p2r += le; p2i += le;
            }

            tr = ur*wr - ui*wi;
            ui = ur*wi + ui*wr;
            ur = tr;
        }
    }
}

void tfAddSynth::gen(eF32 bandwidth, eF32 bwscale, eF32 damp, eF32 numharmonics, eInt profile, eInt sampleRate)
{
    if (!m_padSynthTable || 
        m_bandwidth != bandwidth || 
        m_damp != damp ||
        m_numHarmonics != numharmonics ||
        m_bwScale != bwscale ||
        m_profile != profile ||
        m_sampleRate != m_sampleRate)
    {
        m_bandwidth = bandwidth;
        m_damp = damp;
        m_numHarmonics = numharmonics;
        m_bwScale = bwscale;
        m_profile = profile;
        m_sampleRate = sampleRate;

        if (!m_padSynthTable)
        {
            m_padSynthTable = new eF32[TF_ADDSYNTHTABLESIZE];
            eMemSet(m_padSynthTable, 0, sizeof(eF32) * TF_ADDSYNTHTABLESIZE);
        }

        m_padRefresh = true;
    }
}

void tfAddSynth::check()
{
    if (m_padRefresh)
    {
        m_padRefresh = false;

        if (!m_padSynthTableTemp)
            m_padSynthTableTemp = new eF32[TF_ADDSYNTHTABLESIZE];

        eF32 basefreq = tfInstrument::m_freqTable[29];
        eInt max_harmonics = eFtoL(m_sampleRate/basefreq);

        eInt number_harmonics = eFtoL(m_numHarmonics*255);
        number_harmonics = 16 + eMin(number_harmonics, max_harmonics);

        eF32 bw = m_bandwidth * 256;

        eF32 *A = new eF32[number_harmonics];

        A[0]=0.0;

        for (eInt i=1;i<number_harmonics;i+=2) 
        {
            eF32 div = 1.0f + i * (1.0f-m_damp);

            A[i]=1.0f/div;
        }
        for (eInt i=2;i<number_harmonics;i+=2) 
        {
            eF32 div = 1.0f + i * (1.0f-m_damp);
            A[i]=2.0f/div;
        }

        genRand();
        extendedAlgorithm(TF_ADDSYNTHTABLESIZE,
                        m_profile,
                        m_sampleRate,
                        basefreq,
                        bw,
                        m_bwScale,
                        number_harmonics,
                        A,
                        m_padSynthTableTemp);

        eSAFE_DELETE_ARRAY(A);

        eMemCopy(m_padSynthTable, m_padSynthTableTemp, sizeof(eF32) * TF_ADDSYNTHTABLESIZE);
    }
}

#endif