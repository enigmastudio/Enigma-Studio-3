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

#ifndef TF_FILTER_HPP
#define TF_FILTER_HPP

class tfFilter
{
public:

    enum Mode
    {
        FILTER_HIGHPASS = 0,
        FILTER_LOWPASS = 1,
        FILTER_BANDPASS = 2,
        FILTER_NOTCH = 3,

        FILTER_NOISE_LOWPASS = 4,
        FILTER_NOISE_HIGHPASS = 5,

		FILTER_OVERSAMPLING_LOWPASS = 6
    };

    struct State
    {
        void reset()
        {
            a0 = 0.0f;
            a1 = 0.0f;
            a2 = 0.0f;
            b0 = 0.0f;
            b1 = 0.0f;
            b2 = 0.0f;

            // previous inputs/outputs
            in1_r = 0.0f;
            in2_r = 0.0f;
            out1_r = 0.0f;
            out2_r = 0.0f;
            in1_l = 0.0f;
            in2_l = 0.0f;
            out1_l = 0.0f;
            out2_l = 0.0f;

            // moog vcf
            y1_l=y2_l=y3_l=y4_l=oldx_l=oldy1_l=oldy2_l=oldy3_l=0;
            y1_r=y2_r=y3_r=y4_r=oldx_r=oldy1_r=oldy2_r=oldy3_r=0;
        }

        // Filter coefficients
        eF32    a0;
        eF32    a1;
        eF32    a2;
        eF32    b0;
        eF32    b1;
        eF32    b2;

        // previous inputs/outputs
        eF32    in1_r;
        eF32    in2_r;
        eF32    out1_r;
        eF32    out2_r;
        eF32    in1_l;
        eF32    in2_l;
        eF32    out1_l;
        eF32    out2_l;

        // moog vcf
        eBool moog_vcf;
        eF32 k, p, r;
        eF32 oldx_l;
        eF32 oldy1_l, y1_l;
        eF32 oldy2_l, y2_l;
        eF32 oldy3_l, y3_l;
        eF32 y4_l;
        eF32 oldx_r;
        eF32 oldy1_r, y1_r;
        eF32 oldy2_r, y2_r;
        eF32 oldy3_r, y3_r;
        eF32 y4_r;
    };

    void update(tfFilter::State *state, 
        tfModMatrix *modMatrix, 
        tfModMatrix::State *modMatrixState, 
        eF32 *params, 
        tfFilter::Mode mode, 
        eU32 sampleRate);

    void process(State *state, eSignal **signal, eU32 len);
};

#endif // TF_FILTER_HPP