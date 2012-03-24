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

#define USE_MOOG_VCF 

void tfFilter::update(tfFilter::State *state, 
    tfModMatrix *modMatrix, 
    tfModMatrix::State *modMatrixState, 
    eF32 *params, 
    tfFilter::Mode mode, 
    eU32 sampleRate)
{
    eF32 f; 
    eF32 q;
    eF32 gain = 1.0f;

    switch (mode)
    {
        case tfFilter::FILTER_LOWPASS:
            {
                f = params[TF_LP_FILTER_CUTOFF];
                q = params[TF_LP_FILTER_RESONANCE];

                if (modMatrix)
                {
                    f *= modMatrix->get(modMatrixState, tfModMatrix::OUTPUT_LP_FILTER_CUTOFF);
                    q *= modMatrix->get(modMatrixState, tfModMatrix::OUTPUT_LP_FILTER_RESONANCE);
                }
            }
            break;

        case tfFilter::FILTER_HIGHPASS:
            {
                f = params[TF_HP_FILTER_CUTOFF];
                q = params[TF_HP_FILTER_RESONANCE];

                if (modMatrix)
                {
                    f *= modMatrix->get(modMatrixState, tfModMatrix::OUTPUT_HP_FILTER_CUTOFF);
                    q *= modMatrix->get(modMatrixState, tfModMatrix::OUTPUT_HP_FILTER_RESONANCE);
                }
            }
            break;

#ifndef NO_BANDPASS_FILTER
        case tfFilter::FILTER_BANDPASS:
            {
                f = params[TF_BP_FILTER_CUTOFF];
                q = params[TF_BP_FILTER_Q];

                if (modMatrix)
                {
                    f *= modMatrix->get(modMatrixState, tfModMatrix::OUTPUT_BP_FILTER_CUTOFF);
                    q *= modMatrix->get(modMatrixState, tfModMatrix::OUTPUT_BP_FILTER_Q);
                }
            }
            break;
#endif

#ifndef NO_NOTCH_FILTER
        case tfFilter::FILTER_NOTCH:
            {
                f = params[TF_NT_FILTER_CUTOFF];
                q = params[TF_NT_FILTER_Q];

                if (modMatrix)
                {
                    f *= modMatrix->get(modMatrixState, tfModMatrix::OUTPUT_NT_FILTER_CUTOFF);
                    q *= modMatrix->get(modMatrixState, tfModMatrix::OUTPUT_NT_FILTER_Q);
                }
            }
            break;
#endif

        case tfFilter::FILTER_NOISE_LOWPASS:
            {
                f = params[TF_NOISE_FREQ];
                f += params[TF_NOISE_BW];
                q = 0.05f;
                mode = tfFilter::FILTER_LOWPASS;
            }
            break;

        case tfFilter::FILTER_NOISE_HIGHPASS:
            {
                f = params[TF_NOISE_FREQ];
                f -= params[TF_NOISE_BW];
                q = 0.05f;
                mode = tfFilter::FILTER_HIGHPASS;
            }
            break;

		case tfFilter::FILTER_OVERSAMPLING_LOWPASS:
            {
                f = 0.8f;
                q = 0.0f;
                mode = tfFilter::FILTER_LOWPASS;
            }
            break;
    }

    f = eClamp<eF32>(0.0f, f, 1.0f);
    
#ifdef USE_MOOG_VCF
    if (mode == tfFilter::FILTER_LOWPASS)
    {
        q = eClamp<eF32>(0.0f, q, 0.95f);
        f = f * f * 20000.0f + 30.0f;

        f = 2.0f * f / sampleRate; //[0 - 1]
        state->k = 3.6f*f - 1.6f*f*f -1.0f; //(Empirical tunning)
        state->p = (state->k+1.0f)*0.5f;
        eF32 scale = ePow(2.718281828459f, ((1.0f-state->p)*1.386249f));
        state->r = q*scale;
        state->moog_vcf = eTRUE;
        
        //printf("%f %f %f %f %f\n", q, f, state->k, state->p, state->r);
    }
    else
    {
#endif
        state->moog_vcf = eFALSE;

        q = 1.0f - eClamp<eF32>(0.0f, q, 0.95f);
        f = f * f * 10000.0f + 30.0f;

        //eF32 A = eSqrt( ePow(10,(gain/20)) );
        eF32 A = 1.059253725f;
        eF32 w0 = 2 * ePI * f / sampleRate;
        eF32 alpha;

        const eF32 cos_w0 = eCos(w0);
        const eF32 sin_w0 = eSin(w0);

        if (mode == tfFilter::FILTER_BANDPASS || mode == tfFilter::FILTER_NOTCH) 
            alpha = sin_w0/2 * eSqrt( (A + 1/A)*(1/q - 1) + 2 );
        else
            alpha = sin_w0 * eSinH( eLog(2.0f)/2 * q * w0/sin_w0 );
        
        switch(mode)
        {
#ifndef USE_MOOG_VCF 
            case tfFilter::FILTER_LOWPASS:
            {
                state->b0 =  (1.0f - cos_w0)/2.0f;
                state->b1 =   1.0f - cos_w0;
                state->b2 =  state->b0;
            }
            break;
#endif

            case tfFilter::FILTER_HIGHPASS:
            {
                state->b0 =  (1.0f + cos_w0)/2.0f;
                state->b1 = -(1.0f + cos_w0);
                state->b2 =  state->b0;
            }
            break;

#ifndef NO_BANDPASS_FILTER
            case tfFilter::FILTER_BANDPASS:
            {
                state->b0 =   sin_w0 / 2.0f;
                state->b1 =   0.0f;
                state->b2 =  -state->b0;
            }
            break;
#endif

#ifndef NO_NOTCH_FILTER
            case tfFilter::FILTER_NOTCH:
            {
                state->b0 =   1.0f;
                state->b1 =  -2.0f * cos_w0;
                state->b2 =   1.0f;
            }
            break;
#endif

#ifdef USE_MOOG_VCF
        }
#endif

        state->a0 =   1.0f + alpha;
        state->a1 =  -2.0f * cos_w0;
        state->a2 =   1.0f - alpha;

        state->b0 /= state->a0;
        state->b1 /= state->a0;
        state->b2 /= state->a0;
        state->a1 /= state->a0;
        state->a2 /= state->a0;
    }
}

void tfFilter::process(tfFilter::State *state, eSignal **signal, eU32 len)
{
    eF32 *signal1 = signal[0];
    eF32 *signal2 = signal[1];

    if (state->moog_vcf)
    {
		eF32x2 p = eSIMDSet2(state->p);
        eF32x2 r = eSIMDSet2(state->r);
        eF32x2 k = eSIMDSet2(state->k);

        eF32x2 x;
        eF32x2 y1 = eSIMDLoad2(state->y1_l, state->y1_r);
        eF32x2 y2 = eSIMDLoad2(state->y2_l, state->y2_r);
        eF32x2 y3 = eSIMDLoad2(state->y3_l, state->y3_r);
        eF32x2 y4 = eSIMDLoad2(state->y4_l, state->y4_r);

        eF32x2 old_x = eSIMDLoad2(state->oldx_l, state->oldx_r);
        eF32x2 old_y1 = eSIMDLoad2(state->oldy1_l, state->oldy1_r);
        eF32x2 old_y2 = eSIMDLoad2(state->oldy2_l, state->oldy2_r);
        eF32x2 old_y3 = eSIMDLoad2(state->oldy3_l, state->oldy3_r);

        eF32x2 const_6 = eSIMDSet2(1.0f / 6.0f);

        while (len--)
        {
            eF32x2 in = eSIMDLoad2((eF32)*signal1, (eF32)*signal2);
			eSIMDUndenormalise(in);
            
			// x = in - r * y4
            x = eSIMDMulSub(in, r, y4);
            
            // state->y1_l=x_l*state->p + state->oldx_l*state->p - state->k*state->y1_l;
            y1 = eSIMDMulSub(eSIMDMulAdd(eSIMDMul(old_x, p), x, p), k, y1);

            //state->y2_l=state->y1_l*state->p+state->oldy1_l*state->p - state->k*state->y2_l;
            y2 = eSIMDMulSub(eSIMDMulAdd(eSIMDMul(old_y1, p), y1, p), k, y2);
            
            //state->y3_l=state->y2_l*state->p+state->oldy2_l*state->p - state->k*state->y3_l;
            y3 = eSIMDMulSub(eSIMDMulAdd(eSIMDMul(old_y2, p), y2, p), k, y3);
            
            //state->y4_l=state->y3_l*state->p+state->oldy3_l*state->p - state->k*state->y4_l;
            y4 = eSIMDMulSub(eSIMDMulAdd(eSIMDMul(old_y3, p), y3, p), k, y4);

            //out_l = state->y4_l - (state->y4_l*state->y4_l*state->y4_l)/6;
            eF32x2 out = eSIMDMulSub(y4, eSIMDMul(eSIMDMul(y4, y4), y4), const_6);
            
            old_x = x;
            old_y1 = y1;
            old_y2 = y2;
            old_y3 = y3;

			eSIMDUndenormalise(out);
			eSIMDStore(out, signal1, signal2);
			signal1++;
			signal2++;
        }
		
		eSIMDStore(x, &state->oldx_l, &state->oldx_r);
		eSIMDStore(y1, &state->oldy1_l, &state->oldy1_r);
		eSIMDStore(y2, &state->oldy2_l, &state->oldy2_r);
		eSIMDStore(y3, &state->oldy3_l, &state->oldy3_r);
		eSIMDStore(y4, &state->y4_l, &state->y4_r);

        state->y1_l = state->oldy1_l;
        state->y1_r = state->oldy1_r;
        state->y2_l = state->oldy2_l;
        state->y2_r = state->oldy2_r;
        state->y3_l = state->oldy3_l;
        state->y3_r = state->oldy3_r;
    }
    else
    {
        eF32x2 b0 = eSIMDSet2(state->b0);
        eF32x2 b1 = eSIMDSet2(state->b1);
        eF32x2 b2 = eSIMDSet2(state->b2);
        eF32x2 a1 = eSIMDSet2(state->a1);
        eF32x2 a2 = eSIMDSet2(state->a2);

        eF32x2 in1 = eSIMDLoad2(state->in1_l, state->in1_r);
        eF32x2 in2 = eSIMDLoad2(state->in2_l, state->in2_r);
        eF32x2 out1 = eSIMDLoad2(state->out1_l, state->out1_r);
        eF32x2 out2 = eSIMDLoad2(state->out2_l, state->out2_r);

        while(len--)
        {
            eF32x2 in = eSIMDLoad2(*signal1, *signal2);
			eSIMDUndenormalise(in);

            /*eF32x2 out = eSIMDSub(   
                            eSIMDSub(      
                                eSIMDAdd(
                                    eSIMDAdd( 
                                        eSIMDMul(b0, in),
                                        eSIMDMul(b1, in1)
                                        ),
                                    eSIMDMul(b2, in2)
                                    ),
                                eSIMDMul(a1, out1)
                                ),
                         eSIMDMul(a2, out2)
                         );*/

			eF32x2 out = eSIMDMulSub(   
                            eSIMDMulSub(      
                                eSIMDMulAdd(
                                    eSIMDMulAdd( 
                                        eSIMDMul(b0, in),
                                        b1, in1
                                        ),
                                    b2, in2
                                    ),
                                a1, out1
                                ),
							a2, out2
                         );

			eSIMDUndenormalise(out);
			eSIMDStore(out, signal1, signal2);

			signal1++;
			signal2++;

            in2 = in1;
            in1 = in;
            out2 = out1;
            out1 = out;
        }

		eSIMDStore(in1, &state->in1_l, &state->in1_r);
		eSIMDStore(in2, &state->in2_l, &state->in2_r);
		eSIMDStore(out1, &state->out1_l, &state->out1_r);
		eSIMDStore(out2, &state->out2_l, &state->out2_r);
    }
}


