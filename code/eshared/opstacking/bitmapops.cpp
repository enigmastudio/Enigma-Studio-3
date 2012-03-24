/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   This file is part of
 *       _______   ______________  ______     _____
 *      / ____/ | / /  _/ ____/  |/  /   |   |__  /
 *     / __/ /  |/ // // / __/ /|_/ / /| |    /_ <
 *    / /___/ /|  // // /_/ / /  / / ___ |  ___/ /
 *   /_____/_/ |_/___/\____/_/  /_/_/  |_| /____/.
 *
 *   Copyright © 2003-2010 Brain Control, all rights reserved.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <windows.h>

#include "../eshared.hpp"


    static inline __m128 CalcWeights(float x, float y)
    {
        _MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO);

        static const __m128 CONST_1111 = _mm_set1_ps(1);
        static const __m128 CONST_256 = _mm_set1_ps(256);

        __m128 ssx = _mm_set_ss(x);
        __m128 ssy = _mm_set_ss(y);
        __m128 psXY = _mm_unpacklo_ps(ssx, ssy);      // 0 0 y x

        //__m128 psXYfloor = _mm_floor_ps(psXY); // use this line for if you have SSE4
        __m128 psXYfloor = _mm_cvtepi32_ps(_mm_cvtps_epi32(psXY));
        __m128 psXYfrac = _mm_sub_ps(psXY, psXYfloor); // = frac(psXY)

        __m128 psXYfrac1 = _mm_sub_ps(CONST_1111, psXYfrac); // ? ? (1-y) (1-x)
        __m128 w_x = _mm_unpacklo_ps(psXYfrac1, psXYfrac);   // ? ?     x (1-x)
        w_x = _mm_movelh_ps(w_x, w_x);      // x (1-x) x (1-x)
        __m128 w_y = _mm_shuffle_ps(psXYfrac1, psXYfrac, _MM_SHUFFLE(1, 1, 1, 1)); // y y (1-y) (1-y)

        // complete weight vector
        return _mm_mul_ps(w_x, w_y);
    }

    // Returns bilinear filtered color value.
    // Coordinates can be in arbitrary range and
    // are wrapped to fit between [0..width-1] and
    // [0..height-1].
    static eColor _getBilinearFiltered(const eIBitmapOp::Result &res, eF32 u, eF32 v)
    {
        eF32 fau = eAbs(u);
        eF32 fav = eAbs(v);
        int iau = eTrunc(fau);
        int iav = eTrunc(fav);

        int x0 = iau&(res.width-1);
        int y0 = (iav&(res.height-1))*res.width;
        int x1 = (iau+1)&(res.width-1);
        int y1 = ((iav+1)&(res.height-1))*res.width;

        __m128 weight = CalcWeights(fau, fav);

        eColor pixels[4] =
        {
            res.bitmap[y0+x0],
            res.bitmap[y0+x1],
            res.bitmap[y1+x0],
            res.bitmap[y1+x1],
        };

        __m128i p01 = _mm_loadl_epi64((const __m128i *)&pixels[0]);
        __m128i p23 = _mm_loadl_epi64((const __m128i *)&pixels[2]);

        p01 = _mm_unpacklo_epi8(p01, _mm_setzero_si128());
        p23 = _mm_unpacklo_epi8(p23, _mm_setzero_si128());

        static const __m128 CONST_1111 = _mm_set1_ps(1);
        static const __m128 CONST_256 = _mm_set1_ps(256);

        weight = _mm_mul_ps(weight, CONST_256);
        __m128i weighti = _mm_cvtps_epi32(weight); // w4 w3 w2 w1
        weighti = _mm_packs_epi32(weighti, _mm_setzero_si128()); 

        __m128i w12 = _mm_shufflelo_epi16(weighti, _MM_SHUFFLE(1, 1, 0, 0));
        __m128i w34 = _mm_shufflelo_epi16(weighti, _MM_SHUFFLE(3, 3, 2, 2));
        w12 = _mm_unpacklo_epi16(w12, w12); // w2 w2 w2 w2 w1 w1 w1 w1
        w34 = _mm_unpacklo_epi16(w34, w34); // w4 w4 w4 w4 w3 w3 w3 w3

        __m128i L12 = _mm_mullo_epi16(p01, w12);
        __m128i L34 = _mm_mullo_epi16(p23, w34);

        __m128i L1234 = _mm_add_epi16(L12, L34);
        __m128i Lhi = _mm_shuffle_epi32(L1234, _MM_SHUFFLE(3, 2, 3, 2));
        __m128i L = _mm_add_epi16(L1234, Lhi);

        __m128i L8 = _mm_srli_epi16(L, 8); // divide by 256
        L8 = _mm_packus_epi16(L8, _mm_setzero_si128());

        eU32 rr;
        rr = _mm_cvtsi128_si32(L8);
        return eColor(rr);

        // version 1
        /*
        eF32 fau = eAbs(u);
        eF32 fav = eAbs(v);
        int iau = eTrunc(fau);
        int iav = eTrunc(fav);

        const eF32 uf = fau-iau;
        const eF32 vf = fav-iav;
        const eF32 ufvf = uf*vf;

        int x0 = iau%res.width;
        int y0 = (iav%res.height)*res.width;
        int x1 = (iau+1)%res.width;
        int y1 = ((iav+1)%res.height)*res.width;

        return (res.bitmap[y0+x0]*(1.0f-vf-uf+ufvf)+
            res.bitmap[y0+x1]*(uf-ufvf)+
            res.bitmap[y1+x0]*(vf-ufvf)+
            res.bitmap[y1+x1]*ufvf);
        */

        // version 0
        /*
        const eInt iu = eTrunc(u);
        const eInt iv = eTrunc(v);
        const eU32 ut = (eU32)iu;
        const eU32 vt = (eU32)iv;
        const eF32 uf = (u >= 0.0f ? u-iu : 1.0f+u-iu);
        const eF32 vf = (v >= 0.0f ? v-iv : 1.0f+v-iv);
        const eF32 ufvf = uf*vf;

        // Use modulo here, because width and
        // height mustn't be powers of two.
        const eU32 y0 = (vt%res.height)*res.width;
        const eU32 y1 = ((vt+1)%res.height)*res.width;
        const eU32 x0 = ut%res.width;
        const eU32 x1 = (ut+1)%res.width;

        return (res.bitmap[y0+x0]*(1.0f-vf-uf+ufvf)+
                res.bitmap[y0+x1]*(uf-ufvf)+
                res.bitmap[y1+x0]*(vf-ufvf)+
                res.bitmap[y1+x1]*ufvf);
        */
    }




// Fill (bitmap) operator
// ----------------------
// Fill the whole bitmap with just one color.
// Mainly used as a simple starting operator.

#if defined(HAVE_OP_BITMAP_FILL) || defined(eEDITOR)
OP_DEFINE_BITMAP(eFillOp, eFillOp_ID, "Fill", 'f', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_ENUM("Width", "1|2|4|8|16|32|64|128|256|512|1024|2048", DEFSIZE_SEL);
        eOP_PARAM_ADD_ENUM("Height", "1|2|4|8|16|32|64|128|256|512|1024|2048", DEFSIZE_SEL);
        eOP_PARAM_ADD_RGBA("Color", 0.0f, 0.0f, 0.0f, 1.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt widthSel, eInt heightSel, const eFloatColor &color)
    {
        // Reallocate memory for bitmap, if size has changed.
        const eU32 newWidth = 1<<widthSel;
        const eU32 newHeight = 1<<heightSel;

        _reallocate(newWidth, newHeight);

        // Fill bitmap with color.
        eU32 count = m_bmpSize;

        while (count--)
        {
            m_bitmap[count] = color;
        }
    }
OP_END(eFillOp);
#endif

// Color grading (bitmap) operator
// ----------------------
// Creates a 256x16 color grading map that is used as a lookup for color grading FX

#if defined(HAVE_OP_BITMAP_COLOR_GRADING) || defined(eEDITOR)
OP_DEFINE_BITMAP(eColorGradingOp, eColorGradingOp_ID, "Color grading", ' ', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_RGBA("Red", 1.0f, 0.0f, 0.0f, 1.0f);
		eOP_PARAM_ADD_RGBA("Green", 0.0f, 1.0f, 0.0f, 1.0f);
		eOP_PARAM_ADD_RGBA("Blue", 0.0f, 0.0f, 1.0f, 1.0f);
		eOP_PARAM_ADD_FLOAT("Red steepness", 0.1f, 10.0f, 1.0f);
		eOP_PARAM_ADD_FLOAT("Green steepness", 0.1f, 10.0f, 1.0f);
		eOP_PARAM_ADD_FLOAT("Blue steepness", 0.1f, 10.0f, 1.0f);
		eOP_PARAM_ADD_ENUM("Steps", "2|4|8|16", 3);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eFloatColor &colorR, const eFloatColor &colorG, const eFloatColor &colorB, 
		eF32 steepnessR, eF32 steepnessG, eF32 steepnessB, eInt stepsSel)
	{
		const eU32 BM_WIDTH = COLOR_GRADING_SIZE * COLOR_GRADING_SIZE;
		const eU32 BM_HEIGHT = COLOR_GRADING_SIZE;
		eF32 stepLookup[COLOR_GRADING_SIZE];
		eF32 steps;

		if (stepsSel == 0)
			steps = COLOR_GRADING_SIZE / 2;
		else if (stepsSel == 1)
			steps = COLOR_GRADING_SIZE / 4;
		else if (stepsSel == 2)
			steps = COLOR_GRADING_SIZE / 8;
		else
			steps = COLOR_GRADING_SIZE / 16;

		for (eU32 i=0; i<COLOR_GRADING_SIZE; i++)
		{
			eU32 v = eFtoL(i/steps);
			stepLookup[i] = eF32(v) * steps / COLOR_GRADING_SIZE;
		}

        _reallocate(BM_WIDTH, BM_HEIGHT);

		for (eU32 z=0; z<COLOR_GRADING_SIZE; z++)
		{
            const eU32 lookupPosG = eFtoL(ePow(eF32(z) / COLOR_GRADING_SIZE, steepnessG) * COLOR_GRADING_SIZE);
			const eF32 posG = stepLookup[eMin<eU32>(lookupPosG, COLOR_GRADING_SIZE-1)];
			const eColor colG = eColor::BLACK.lerp(colorG, posG);
			const eU32 zoffset = z * COLOR_GRADING_SIZE;

			for (eU32 x=0; x<COLOR_GRADING_SIZE; x++)
			{
                const eU32 lookupPosR = eFtoL(ePow(eF32(x) / COLOR_GRADING_SIZE, steepnessR) * COLOR_GRADING_SIZE);
				const eF32 posR = stepLookup[eMin<eU32>(lookupPosR, COLOR_GRADING_SIZE-1)];
				const eColor colR = eColor::BLACK.lerp(colorR, posR);
				
				for (eU32 y=0; y<COLOR_GRADING_SIZE; y++)
				{
                    const eU32 lookupPosB = eFtoL(ePow(eF32(y) / COLOR_GRADING_SIZE, steepnessB) * COLOR_GRADING_SIZE);
					const eF32 posB = stepLookup[eMin<eU32>(lookupPosB, COLOR_GRADING_SIZE-1)];
					const eColor colB = eColor::BLACK.lerp(colorB, posB);

					m_bitmap[y * BM_WIDTH + zoffset + x] = colR + colG + colB;
				}
			}
		}

    }
OP_END(eColorGradingOp);
#endif

// Perlin noise (bitmap) operator
// ------------------------------
// Creates a tileable perlin noise bitmap.

#if defined(HAVE_OP_BITMAP_PERLIN) || defined(eEDITOR)
OP_DEFINE_BITMAP(ePerlinOp, ePerlinOp_ID, "Perlin", 'p', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_ENUM("Width", "1|2|4|8|16|32|64|128|256|512|1024|2048", DEFSIZE_SEL);
        eOP_PARAM_ADD_ENUM("Height", "1|2|4|8|16|32|64|128|256|512|1024|2048", DEFSIZE_SEL);
        eOP_PARAM_ADD_INT("Octaves", 1, 10, 7);
        eOP_PARAM_ADD_INT("Frequency", 1, 8, 1);
        eOP_PARAM_ADD_FLOAT("Persistence", 0.0f, 10.0f, 1.5f);
        eOP_PARAM_ADD_FLOAT("Amplify", 0.01f, 16.0f, 1.5f);
        eOP_PARAM_ADD_RGBA("Color 0 ", 1.0f, 1.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_RGBA("Color 1", 0.0f, 0.0f, 0.0f, 1.0f);
        eOP_PARAM_ADD_INT("Seed", 0, 65535, 0);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt widthSel, eInt heightSel, eU32 octaves, eU32 freqNum,
            eF32 persis, eF32 amplify, const eFloatColor &color0, const eFloatColor &color1, eU32 seed)
    {
        const eU32 freq = 1<<freqNum;

        // Reallocate memory for bitmap, if
        // size has changed.
        const eU32 newWidth = 1<<widthSel;
        const eU32 newHeight = 1<<heightSel;

        _reallocate(newWidth, newHeight);

        // Initialize perlin noise with seed.
        _initPerlinNoise(seed);

        // Generate perlin noise.
        const eF32 stepx = 1.0f/(eF32)m_bmpDimSize[0];
        const eF32 stepy = 1.0f/(eF32)m_bmpDimSize[1];

        eVector2 v;

        for (eU32 y=0, index=0; y<m_bmpDimSize[1]; y++)
        {
            v.x = 0.0f;

            for (eU32 x=0; x<m_bmpDimSize[0]; x++)
            {
                const eF32 value = _getPerlinNoise(v, octaves, persis, freq, amplify);
                const eF32 finalCol = eClamp(0.0f, (value+1.0f)*0.5f, 1.0f);
                m_bitmap[index++] = eColor(color0).lerp(color1, finalCol);

                v.x += stepx;
            }

            v.y += stepy;
        }
    }

    void _initPerlinNoise(eU32 seed)
    {
        eRandomize(seed);

        for (eU32 i=0; i<LUT_SIZE; i++)
        {
            m_perm[i] = eRandom(0, LUT_SIZE-1);
            m_grad[i] = eRandomF(-1.0f, 1.0f);
        }
    }

    eF32 _getPerlinNoise(const eVector2 &v, eU32 octaves, eF32 persistence, eU32 freq, eF32 amplify) const
    {
        eF32 total = 0.0f;
        eF32 scale = amplify;

        for (eU32 i=0; i<octaves; i++)
        {
            total += _getSmoothedNoise(v.x, v.y, freq)/scale;
            freq <<= 1;
            scale *= persistence;
        }

        return total;
    }

    eF32 _interpolate(eF32 a, eF32 b, eF32 t) const
    {
        eASSERT(t >= 0.0f && t <= 1.0f);

        const eF32 u = 1.0f-t;
        const eF32 r0 = u*u*(0.5f+t);
        const eF32 r1 = t*t*(1.5f-t);

        return 2.0f*(a*r0+b*r1);
    }

    eF32 _getNoise(eInt x, eInt y) const
    {
        return m_grad[(x+m_perm[y&(LUT_SIZE-1)])&(LUT_SIZE-1)];
    }

    eF32 _getSmoothedNoise(eF32 x, eF32 y, eU32 freq) const
    {
        x *= (eF32)freq;
        y *= (eF32)freq;

        freq--;

        const eInt ix = eTrunc(x);
        const eInt iy = eTrunc(y);
        const eF32 fx = x-(eF32)ix;
        const eF32 fy = y-(eF32)iy;

        const eF32 v0 = _getNoise(ix&freq,     iy&freq);
        const eF32 v1 = _getNoise((ix+1)&freq, iy&freq);
        const eF32 v2 = _getNoise(ix&freq,     (iy+1)&freq);
        const eF32 v3 = _getNoise((ix+1)&freq, (iy+1)&freq);

        return _interpolate(_interpolate(v0, v1, fx), _interpolate(v2, v3, fx), fy);
    }

    static const eU32 LUT_SIZE = 256;

    OP_VAR(eU32 m_perm[LUT_SIZE]);
    OP_VAR(eF32 m_grad[LUT_SIZE]);
OP_END(ePerlinOp);
#endif

// Rect (bitmap) operator
// ----------------------
// Draws a colored rectangle on the bitmap.

#if defined(HAVE_OP_BITMAP_RECT) || defined(eEDITOR)
OP_DEFINE_BITMAP(eRectOp, eRectOp_ID, "Rect", ' ', 1, 1, "-1,Bitmap")
    OP_INIT()
    {
        eOP_PARAM_ADD_FXY("Rect start", 0.0f, 1.0f, 0.25f, 0.25f);
        eOP_PARAM_ADD_FXY("Rect end", 0.0f, 1.0f, 0.75f, 0.75f);
        eOP_PARAM_ADD_RGBA("Color", 1.0f, 1.0f, 1.0f, 1.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eVector2 &start, const eVector2 &end, const eFloatColor &color)
    {
        _copyFirstInputBitmap();

        eRect rect(eFtoL(start.x*m_bmpDimSize[0]),
                   eFtoL(start.y*m_bmpDimSize[1]),
                   eFtoL(end.x*m_bmpDimSize[0]),
                   eFtoL(end.y*m_bmpDimSize[1]));

        rect.normalize();

        for (eInt y=rect.top; y<rect.bottom; y++)
        {
            const eU32 index = y*m_bmpDimSize[0]+rect.left;

            for (eInt x=0; x<rect.getWidth(); x++)
            {
                m_bitmap[index+x] = color;
            }
        }
    }
OP_END(eRectOp);
#endif

// Extract (bitmap) operator
// -------------------------
// Copies one color channel to another.

#if defined(HAVE_OP_BITMAP_EXTRACT) || defined(eEDITOR)
OP_DEFINE_BITMAP(eChannelMergeOp, eChannelMergeOp_ID, "Extract channel", ' ', 2, 2, "-1,Bitmap")
    OP_INIT()
    {
	    eOP_PARAM_ADD_ENUM("Copy input", "Left|Right", 0)
    	eOP_PARAM_ADD_ENUM("Channel 0", "---|Other red|Other green|Other blue|Other alpha", 0)
	    eOP_PARAM_ADD_ENUM("Channel 1", "---|Other red|Other green|Other blue|Other alpha", 0)
	    eOP_PARAM_ADD_ENUM("Channel 2", "---|Other red|Other green|Other blue|Other alpha", 0)
	    eOP_PARAM_ADD_ENUM("Channel 3", "---|Other red|Other green|Other blue|Other alpha", 0)
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt copySel, eInt chan0Sel, eInt chan1Sel, eInt chan2Sel, eInt chan3Sel)
    {
        if (getInputCount() > (eU32)copySel)
        {
            const Result &res = ((eIBitmapOp *)getInputOperator(copySel))->getResult();
            eMemCopy(m_bitmap, res.bitmap, res.size*sizeof(eColor));
        }

        const eU32 chan[] =
        {
            chan0Sel, chan1Sel, chan2Sel, chan3Sel
        };

        const eIBitmapOp::Result &res = ((eIBitmapOp *)getInputOperator(1-copySel))->getResult();
        const eColor *opBmpData = res.bitmap;
        eASSERT(opBmpData != eNULL);

        const eU32 srcHeight = res.height;
        const eU32 srcWidth = res.width;
        const eU32 dstWidth = m_bmpDimSize[0];
        const eU32 dstHeight = m_bmpDimSize[1];

        for (eU32 y=0; y<dstHeight; y++)
        {
            const eU32 srcy = (y*dstHeight)/srcHeight;
            const eU32 srcw = srcy*srcWidth;
            const eU32 dstw = y*dstWidth;

            for (eU32 x=0; x<dstWidth; x++)
            {
                const eU32 srcX = (x*dstWidth)/srcWidth;
                const eU32 srcOff = srcw+srcX;
                const eU32 dstOff = dstw+x;

			    for (eU32 c=0; c<4; c++)
                {
				    if(chan[c] != 0)
                    {
					    m_bitmap[dstOff].set(c, res.bitmap[srcOff].get(chan[c]-1));
                    }
                }
            }
        }
    }
OP_END(eChannelMergeOp);
#endif

// Glow (bitmap) operator
// -----------------------
// Generates a glow bitmap, often used as
// base texture for particles.

#if defined(HAVE_OP_BITMAP_GLOW) || defined(eEDITOR)
OP_DEFINE_BITMAP(eGlowOp, eGlowOp_ID, "Glow", 'g', 1, 1, "-1,Bitmap")
    OP_INIT()
    {
       eOP_PARAM_ADD_FXY("Center", -4, 4, 0.5f, 0.5f);
        eOP_PARAM_ADD_FXY("Radius", 0, 4, 0.25f, 0.25f);
        eOP_PARAM_ADD_RGBA("Color", 1.0f, 1.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_INT("Alpha", 0, 255, 255);
        eOP_PARAM_ADD_INT("Gamma", 0, 255, 128);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eVector2 &relCenter, const eVector2 &relRadius,
            const eFloatColor &color, eInt alphaVal, eInt gammaVal)
    {
        const eF32 alpha = (eF32)alphaVal;
        const eF32 gamma = ePow((eF32)gammaVal/255.0f*2.0f, 3.0f);
        const eF32 agadj = alpha/ePow(255.0f, gamma)/255.0f;

        // Calculate absolute bitmap position
        // based on relative parameters.
        const eVector2 center(relCenter.x*m_bmpDimSize[0], relCenter.y*m_bmpDimSize[1]);
        const eVector2 radius(256.0f/(relRadius.x*m_bmpDimSize[0]), 256.0f/(relRadius.y*m_bmpDimSize[1]));

        // Create lookup-table for power function.
        eF32 powLookup[256];

        for (eU32 i=0; i<256; i++)
        {
            powLookup[i] = eClamp(0.0f, ePow((eF32)i, gamma)*agadj, 1.0f);
        }

        // Paint glow onto input bitmap.
        const eColor *inputBmp = ((eIBitmapOp *)getInputOperator(0))->getResult().bitmap;
        eASSERT(inputBmp != eNULL);

        for (eU32 y=0, index=0; y<m_bmpDimSize[1]; y++)
        {
            const eF32 v = (center.y-(eInt)y)*radius.y;
            const eF32 vv = v*v;

            for (eU32 x=0; x<m_bmpDimSize[0]; x++)
            {
                const eF32 u = (center.x-(eInt)x)*radius.x;
                const eF32 uu = u*u;

                const eU32 p = 255-eMin(eFtoL(eSqrt(uu+vv)), 255);
                const eF32 q = powLookup[p];

                m_bitmap[index] = inputBmp[index].lerp(color, q);
                index++;
            }
        }
    }
OP_END(eGlowOp);
#endif

// Pixels (bitmap) operator
// ------------------------
// Puts randomly pixels on bitmap. The color
// for each pixel is randomly interpolated
// between two colors.

#if defined(HAVE_OP_BITMAP_PIXELS) || defined(eEDITOR)
OP_DEFINE_BITMAP(ePixelsOp, ePixelsOp_ID, "Pixels", ' ', 1, 1, "-1,Bitmap")
    OP_INIT()
    {
        eOP_PARAM_ADD_RGBA("Color 0", 0.0f, 0.0f, 0.0f, 1.0f);
        eOP_PARAM_ADD_RGBA("Color 1", 1.0f, 1.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_INT("Count", 0, eU16_MAX, 32);
        eOP_PARAM_ADD_INT("Seed", 0, 65535, 0);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eFloatColor &color0, const eFloatColor &color1, eU32 countVal, eU32 seed)
    {
        _copyFirstInputBitmap();
        eRandomize(seed);

        const eColor c0 = color0;
        const eColor c1 = color1;

        // Calculate number of pixels based on bitmap
        // size (bigger bitmap => more pixels). The
        // scaling factor therefore minds the exponential
        // growing of the bitmap size.
        const eF32 countScale = (eF32)m_bmpDimSize[0]*(eF32)m_bmpDimSize[1]/4096.0f;
        eInt count = eFtoL((eF32)(countVal+1)*countScale);

        while (count--)
        {
            const eU32 x = eRandom(0, m_bmpDimSize[0]);
            const eU32 y = eRandom(0, m_bmpDimSize[1]);

            m_bitmap[y*m_bmpDimSize[0]+x] = c0.lerp(c1, eRandomF());
        }
    }
OP_END(ePixelsOp);
#endif

// Merge (bitmap) operator
// -----------------------
// Merges multiple bitmap operators together.
// The merging mode can be selected.

#if defined(HAVE_OP_BITMAP_MERGE) || defined(eEDITOR)
OP_DEFINE_BITMAP(eBitmapMergeOp, eBitmapMergeOp_ID, "Merge", 'm', 1, 64, "-1,Bitmap")
    OP_INIT()
    {
       eOP_PARAM_ADD_ENUM("Mode", "Add|Sub|Mul|Difference|Average|Minimum|Maximum", 0);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt mode)
    {
        _copyFirstInputBitmap();

        for (eU32 i=1; i<getInputCount(); i++)
        {
            eIBitmapOp *op = (eIBitmapOp *)getInputOperator(i);
            eASSERT(op != eNULL);

            // Fetch bitmap data outside loop, because
            // virtual functions can be VERY slow in
            // debug mode.
            const eColor *opBmpData = op->getResult().bitmap;
            eASSERT(opBmpData != eNULL);
            eU32 size = op->getResult().size;

            switch (mode)
            {
                case 0: // Add
                {
                    while (size--)
                    {
                        m_bitmap[size] += opBmpData[size];
                        m_bitmap[size].setAlpha(opBmpData[size].alpha());
                    }

                    break;
                }

                case 1: // Sub
                {
                    while (size--)
                    {
                        m_bitmap[size] -= opBmpData[size];
                        m_bitmap[size].setAlpha(opBmpData[size].alpha());
                    }

                    break;
                }

                case 2: // Mul
                {
                    while (size--)
                    {
                        m_bitmap[size] *= opBmpData[size];
                        m_bitmap[size].setAlpha(opBmpData[size].alpha());
                    }

                    break;
                }

                case 3: // Difference
                {
                    while (size--)
                    {
                        m_bitmap[size].difference(opBmpData[size]);
                    }

                    break;
                }

                case 4: // Average
                {
                    while (size--)
                    {
                        m_bitmap[size].average(opBmpData[size]);
                    }

                    break;
                }

                case 5: // Minimum
                {
                    while (size--)
                    {
                        m_bitmap[size].minimum(opBmpData[size]);
                    }

                    break;
                }

                case 6: // Maximum
                {
                    while (size--)
                    {
                        m_bitmap[size].maximum(opBmpData[size]);
                    }

                    break;
                }
            }
        }
    }
OP_END(eBitmapMergeOp);
#endif

// Adjust (bitmap) operator
// ------------------------
// Adjusts brightness, contrast and hue and
// saturation (using HSV color space) of bitmap.

#if defined(HAVE_OP_BITMAP_ADJUST) || defined(eEDITOR)
OP_DEFINE_BITMAP(eAdjustOp, eAdjustOp_ID, "Adjust", 'a', 1, 1, "-1,Bitmap")
    OP_INIT()
    {
        eOP_PARAM_ADD_FLOAT("Brightness", 0, 255.0f, 1.0f);
        eOP_PARAM_ADD_FLOAT("Contrast", 0.0f, 128.0f, 1.0f);
        eOP_PARAM_ADD_FLOAT("Hue", 0.0f, 2.0f, 1.0f);
        eOP_PARAM_ADD_FLOAT("Saturation", 0.0f, 16.0f, 1.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 brightness, eF32 contrast, eF32 hueVal, eF32 saturation)
    {
        const eColor *inputBmp = ((eIBitmapOp *)getInputOperator(0))->getResult().bitmap;
        eASSERT(inputBmp != eNULL);

        const eInt hue = eFtoL((hueVal-1.0f)*359.0f);

        // Adjust brightness and contrast.
        const eF32 tcb = (128*contrast-128)*brightness;
        const eF32 cmb = contrast*brightness;

        for (eU32 i=0; i<m_bmpSize; i++)
        {
            const eInt r = eClamp(0, eFtoL(inputBmp[i].red()*cmb-tcb), 255);
            const eInt g = eClamp(0, eFtoL(inputBmp[i].green()*cmb-tcb), 255);
            const eInt b = eClamp(0, eFtoL(inputBmp[i].blue()*cmb-tcb), 255);
            const eInt a = inputBmp[i].alpha();

            m_bitmap[i].set(r, g, b, a);
        }

        // Adjust hue and saturation.
        for (eU32 i=0; i<m_bmpSize; i++)
        {
            eInt h, s, v;

            m_bitmap[i].toHsv(h, s, v);

            h = eClamp(0, h+hue, 359);
            s = eMin(eFtoL((eF32)s*saturation), 255);

            m_bitmap[i].fromHsv(h, s, v);
        }
    }
OP_END(eAdjustOp);
#endif

// Normals (bitmap) operator
// -------------------------
// Generates a normal map of its input bitmap,
// which is used for bump mapping for example.
// The sobel operator is used here in x and y
// direction (normally used for edge detection).

#if defined(HAVE_OP_BITMAP_NORMALS) || defined(eEDITOR)
OP_DEFINE_BITMAP(eNormalsOp, eNormalsOp_ID, "Normals", 'n', 1, 1, "-1,Bitmap")
    OP_INIT()
    {
       eOP_PARAM_ADD_INT("Strength", 0, 255, 32);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt strength)
    {
        const eU32 andSize = m_bmpSize-1;

        const eColor *inputBmp = ((eIBitmapOp *)getInputOperator(0))->getResult().bitmap;
        eASSERT(inputBmp != eNULL);

        eVector3 normal;

        for (eU32 i=0; i<m_bmpSize; i++)
        {
            // Heights are stored in the v<0..8> variables.
            const eU8 v0 = inputBmp[((i-m_bmpDimSize[0]-1)&andSize)].grayScale();
            const eU8 v1 = inputBmp[((i-m_bmpDimSize[0]  )&andSize)].grayScale();
            const eU8 v2 = inputBmp[((i-m_bmpDimSize[0]+1)&andSize)].grayScale();
            const eU8 v3 = inputBmp[((i           -1)&andSize)].grayScale();
            const eU8 v5 = inputBmp[((i           +1)&andSize)].grayScale();
            const eU8 v6 = inputBmp[((i+m_bmpDimSize[0]-1)&andSize)].grayScale();
            const eU8 v7 = inputBmp[((i+m_bmpDimSize[0]  )&andSize)].grayScale();
            const eU8 v8 = inputBmp[((i+m_bmpDimSize[0]+1)&andSize)].grayScale();

            // Calculate normal vector by applying
            // a 2-dimensional sobel filter.
            normal.x = (v0-v2+2.0f*(v3-v5)+v6-v8);
            normal.y = (v0+2.0f*(v1-v7)+v2-v6-v8);
            normal.z = 255.0f-(eF32)strength;
        
            normal.normalize();

            // Convert floating point to byte values.
            // range [-1.0 -> 1.0] to [0 -> 255]
            normal += 1.0f;
            normal *= 127.5f;

            // Set color in bitmap at current position.
            m_bitmap[i].setRed(eFtoL(normal.x));
            m_bitmap[i].setGreen(eFtoL(normal.y));
            m_bitmap[i].setBlue(eFtoL(normal.z));
        }
    }
OP_END(eNormalsOp);
#endif

// Text (bitmap) operator
// ----------------------
// Writes text on its input bitmap.

#if defined(HAVE_OP_BITMAP_TEXT) || defined(eEDITOR)
OP_DEFINE_BITMAP(eTextOp, eTextOp_ID, "Text", 't', 1, 1, "-1,Bitmap")
    OP_INIT()
    {
        eOP_PARAM_ADD_FXY("Position", -4.0f, 4.0f, 0.0f, 0.0f);
        eOP_PARAM_ADD_FLOAT("Size", 0.0f, 16.0f, 0.125f);
        eOP_PARAM_ADD_FLOAT("Stretch", 0.0f, 16.0f, 0.0f);
        eOP_PARAM_ADD_FLOAT("Leading", 0.0f, 16.0f, 0.1f);
        eOP_PARAM_ADD_FLOAT("Kerning", 0.0f, 16.0f, 0.0f);
        eOP_PARAM_ADD_RGBA("Color", 1.0f, 1.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_STRING("Font", "");
        eOP_PARAM_ADD_TEXT("Text", "Brain Control");
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eVector2 &fltPos, eF32 sizeVal, eF32 stretchVal, eF32 leadingVal,
            eF32 kerningVal, const eFloatColor &colorVal, const eChar *fontName, const eChar *text)
    {
        // Do relative to absolute calculations.
        const eInt size = eFtoL(sizeVal*m_bmpDimSize[1]);
        const eU32 stretch = eFtoL(stretchVal*m_bmpDimSize[0]);
        const eU32 leading = eFtoL(leadingVal*m_bmpDimSize[1]);
        const eU32 kerning = eFtoL(kerningVal*m_bmpDimSize[0]);
        const eColor color = colorVal;

        const ePoint position(eFtoL(fltPos.x*m_bmpDimSize[0]), eFtoL(fltPos.y*m_bmpDimSize[1]));

        // Create DCs and bitmap for rendering text on.
        HDC hdc = GetDC(eNULL);
        eASSERT(hdc != eNULL);
        HDC compDc = CreateCompatibleDC(hdc);
        eASSERT(compDc != eNULL);
        HBITMAP bmp = CreateCompatibleBitmap(hdc, m_bmpDimSize[0], m_bmpDimSize[1]);
        eASSERT(bmp != eNULL);
        HBITMAP oldBmp = (HBITMAP)SelectObject(compDc, bmp);

        // Set text color. Switch blue and red channel,
        // because strange windows API wants it this way.
        SetTextColor(compDc, RGB(color.blue(), color.green(), color.red()));
        SetBkMode(compDc, TRANSPARENT);

        // Create antialiased font.
        LOGFONT lf;

        eMemSet(&lf, 0, sizeof(LOGFONT));
        eStrCopy(lf.lfFaceName, fontName);

        lf.lfCharSet = ANSI_CHARSET;
        lf.lfHeight  = -size;
        lf.lfWidth   = stretch;
        lf.lfQuality = ANTIALIASED_QUALITY;
        lf.lfWeight  = FW_NORMAL;

        HFONT font = CreateFontIndirect(&lf);
        eASSERT(font != eNULL);
        HFONT oldFont = (HFONT)SelectObject(compDc, font);

        // Write bitmap data to DIB, so text can
        // be rendered onto an existing bitmap.
        BITMAPINFO bmi;

        eMemSet(&bmi, 0, sizeof(bmi));
        bmi.bmiHeader.biSize        = sizeof(bmi.bmiHeader);
        bmi.bmiHeader.biPlanes      = 1;
        bmi.bmiHeader.biBitCount    = 32;
        bmi.bmiHeader.biWidth       = m_bmpDimSize[0];
        bmi.bmiHeader.biHeight      = -(eInt)m_bmpDimSize[1];
        bmi.bmiHeader.biCompression = BI_RGB;

        const eColor *inputBmp = ((eIBitmapOp *)getInputOperator(0))->getResult().bitmap;
        eASSERT(inputBmp != eNULL);

        eColor *temp = new eColor[m_bmpSize];
        eASSERT(temp != eNULL);

        for (eU32 i=0; i<m_bmpSize; i++)
        {
            eColor &c = temp[i];

            // Copy color, but swap red and blue channel
            // (needed because bitmap DIB swaps, too).
            c = inputBmp[i].toArgb();

            const eU8 r = c.red();
            c.setRed(c.blue());
            c.setBlue(r);
        }

        SetDIBits(compDc, bmp, 0, m_bmpDimSize[1], temp, &bmi, DIB_RGB_COLORS);

        // Render text line by line and character
        // by character, so that leading and kerning
        // can be applied (this isn't possible using
        // the windows API functions).
        RECT r;

        r.bottom = m_bmpDimSize[1];
        r.right = m_bmpDimSize[0];

        for (eU32 i=0, lineNum=0; text[i]!='\0'; i++, lineNum++)
        {
            r.top = position.y+lineNum*leading;
            r.left = position.x;

            while (text[i] != '\n' && text[i] != '\0')
            {
                const eChar chr[2] = {text[i], '\0'};
                SIZE s;

                GetTextExtentPoint32(compDc, chr, 1, &s);
                DrawText(compDc, chr, 1, &r, 0);
                r.left += s.cx+kerning;

                if (text[++i] == '\r')
                {
                    i++;
                    break;
                }
            }
        }

        // Read back bitmap data with text on.
        GetDIBits(compDc, bmp, 0, m_bmpDimSize[1], temp, &bmi, DIB_RGB_COLORS);

        for (eU32 i=0; i<m_bmpSize; i++)
        {
            eColor c = temp[i]; // Uses = operator with RGBA.
            c.setAlpha(m_bitmap[i].alpha()); // Use old alpha channel.
            m_bitmap[i] = c;
        }

        eSAFE_DELETE_ARRAY(temp);

        // Finally free all GDI objects.
        SelectObject(compDc, oldFont);
        SelectObject(compDc, oldBmp);
        DeleteObject(font);
        DeleteObject(bmp);
        DeleteDC(compDc);
        DeleteDC(hdc);
    }
OP_END(eTextOp);
#endif

// Color (bitmap) operator
// -----------------------
// Multi-purpose operator which supports different
// operations on bitmap (add, subtracting and
// multiplying a color, grayscale and invert bitmap).

#if defined(HAVE_OP_BITMAP_COLOR) || defined(eEDITOR)
OP_DEFINE_BITMAP(eColorOp, eColorOp_ID, "Color", 'c', 1, 1, "-1,Bitmap")
    OP_INIT()
    {
        eOP_PARAM_ADD_ENUM("Mode", "Add|Sub|Multiply|Grayscale|Invert", 0);
        eOP_PARAM_ADD_RGBA("Color", 0.0f, 0.0f, 0.0f, 1.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt mode, const eFloatColor &color)
    {
        _copyFirstInputBitmap();

        eU32 size = m_bmpSize;

        switch (mode)
        {
            case 0: // Add
            {
                while (size--)
                {
                    m_bitmap[size] += color;
                }

                break;
            }

            case 1: // Subtract
            {
                while (size--)
                {
                    m_bitmap[size] -= color;
                }

                break;
            }

            case 2: // Multiply
            {
                while (size--)
                {
                    m_bitmap[size] *= color;
                }

                break;
            }

            case 3: // Grayscale (ignores color parameter).
            {
                while (size--)
                {
                    m_bitmap[size].toGrayScale();
                }

                break;
            }

            case 4: // Invert (ignores color parameter). Preserves alpha channel.
            {
                while (size--)
                {
                    eColor col = eColor::WHITE-m_bitmap[size];
                    col.setAlpha(m_bitmap[size].alpha());

                    m_bitmap[size] = col;
                }

                break;
            }
        }
    }
OP_END(eColorOp);
#endif

// Rotozoom (bitmap) operator
// --------------------------
// This operator can rotate, scroll and
// zoom a bitmap.

#if defined(HAVE_OP_BITMAP_ROTOZOOM) || defined(eEDITOR)
OP_DEFINE_BITMAP(eRotoZoomOp, eRotoZoomOp_ID, "Rotozoom", 'r', 1, 1, "-1,Bitmap")
    OP_INIT()
    {
        eOP_PARAM_ADD_FLOAT("Angle", eF32_MIN, eF32_MAX, 0.0f);
        eOP_PARAM_ADD_FXY("Zoom", 0.0f, 16.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_FXY("Scroll", -4.0f, 4.0f, 0.5f, 0.5f);
        eOP_PARAM_ADD_FLAGS("Clamp borders", "X axis|Y axis", 0);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 angleVal, const eVector2 &zoomVal, const eVector2 &scrollVal, eU8 clampBorders)
    {
        const eF32 angle = angleVal*eTWOPI;
        const eBool xClamp = eGetBit(clampBorders, 0);
        const eBool yClamp = eGetBit(clampBorders, 1);

        const eVector2 zoom(ePow(0.05f, zoomVal.x-1.0f), ePow(0.05f, zoomVal.y-1.0f));
        const eVector2 scroll(scrollVal.x*(eF32)m_bmpDimSize[0], scrollVal.y*(eF32)m_bmpDimSize[1]);

        // Pre-calculate some values.
        eF32 s, c;

        eSinCos(angle, s, c);

        const eF32 uRot = -0.5f*(m_bmpDimSize[0]*c-m_bmpDimSize[1]*s);
        const eF32 vRot = -0.5f*(m_bmpDimSize[0]*s+m_bmpDimSize[1]*c);

        // Zoom, rotate and scroll bitmap.
        const Result &inputRes = ((eIBitmapOp *)getInputOperator(0))->getResult();

        for (eU32 y=0, index=0; y<m_bmpDimSize[1]; y++)
        {
            const eF32 xp = vRot+y*c;
            const eF32 yp = uRot-y*s;

            for (eU32 x=0; x<m_bmpDimSize[0]; x++)
            {
                eF32 u = (yp+x*c)*zoom.x+scroll.x;
                eF32 v = (xp+x*s)*zoom.y+scroll.y;

                if (xClamp)
                {
                    u = eClamp(0.0f, u, (eF32)(m_bmpDimSize[0]-1));
                }

                if (yClamp)
                {
                    v = eClamp(0.0f, v, (eF32)(m_bmpDimSize[1]-1));
                }

                m_bitmap[index++] = _getBilinearFiltered(inputRes, u, v);
            }
        }

        /*
        const eF32 angle = angleVal*eTWOPI;
        const eBool xClamp = eGetBit(clampBorders, 0);
        const eBool yClamp = eGetBit(clampBorders, 1);

        const eVector2 zoom(ePow(0.05f, zoomVal.x-1.0f), ePow(0.05f, zoomVal.y-1.0f));
        const eVector2 scroll(scrollVal.x*(eF32)m_bmpDimSize[0], scrollVal.y*(eF32)m_bmpDimSize[1]);

        // Pre-calculate some values.
        eF32 s, c;

        eSinCos(angle, s, c);

        const eF32 uRot = -0.5f*(m_bmpDimSize[0]*c-m_bmpDimSize[1]*s);
        const eF32 vRot = -0.5f*(m_bmpDimSize[0]*s+m_bmpDimSize[1]*c);

        // Zoom, rotate and scroll bitmap.
        const Result &inputRes = ((eIBitmapOp *)getInputOperator(0))->getResult();

        eU32 TILE_SIZE = 8;

        for (eU32 y=0; y<m_bmpDimSize[1]; y+=TILE_SIZE)
        {
            for (eU32 x=0; x<m_bmpDimSize[0]; x+=TILE_SIZE)
            {
                for (eU32 i=y; i<y+TILE_SIZE; i++)
                {
                    const eU32 yDelta = i*m_bmpDimSize[0];
                    const eF32 yp = uRot-i*s;
                    const eF32 xp = vRot+i*c;

                    for (eU32 j=x; j<x+TILE_SIZE; j++)
                    {
                        eF32 u = (yp+j*c)*zoom.x+scroll.x;
                        eF32 v = (xp+j*s)*zoom.y+scroll.y;

                        if (xClamp)
                        {
                            u = eClamp(0.0f, u, (eF32)(m_bmpDimSize[0]-1));
                        }

                        if (yClamp)
                        {
                            v = eClamp(0.0f, v, (eF32)(m_bmpDimSize[1]-1));
                        }

                        m_bitmap[yDelta+j] = _getBilinearFiltered(inputRes, u, v);
                    }
                }

            }
        }
        */
    }
OP_END(eRotoZoomOp);
#endif

// Bump (bitmap) operator
// ----------------------
// The bump operator simulates a lit 3D surface
// in a bitmap, using the phong shading model. 

#if defined(HAVE_OP_BITMAP_BUMP) || defined(eEDITOR)
OP_DEFINE_BITMAP(eBumpOp, eBumpOp_ID, "Bump", 'u', 2, 2, "-1,Bitmap")
    OP_INIT()
    {
        eOP_PARAM_ADD_RGB("Ambient", 1.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_RGB("Diffuse", 1.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_RGB("Specular", 0.5f, 0.5f, 0.5f);
        eOP_PARAM_ADD_FXYZ("Position", -8.0f, 8.0f, 1.0f, 1.0f, 0.5f);
        eOP_PARAM_ADD_FLOAT("Specular amount", 0.0f, 4.0f, 1.0f);
        eOP_PARAM_ADD_FLOAT("Bump amount", 0.0f, 4.0f, 1.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eFloatColor &ambient, const eFloatColor &diffuse,
            const eFloatColor &specular, const eVector3 &pos, eF32 specAmount, eF32 bumpAmount)
    {
        eVector3 position = pos;
        position.x -= 0.5f;
        position.y = -position.y+0.5f;
        position.z = position.z-0.5f;

        // Get bitmap of input operator, which
        // is used as normal map.
        const eColor *normalMap = ((eIBitmapOp *)getInputOperator(1))->getResult().bitmap;
        eASSERT(normalMap != eNULL);

        // Perform bump operation.
        const eColor *inputBmp = ((eIBitmapOp *)getInputOperator(0))->getResult().bitmap;
        eASSERT(inputBmp != eNULL);

        for (eU32 y=0, index=0; y<m_bmpDimSize[1]; y++)
        {
            for (eU32 x=0; x<m_bmpDimSize[0]; x++, index++)
            {
                // Normalize color from normal map.
                eVector3 n(normalMap[index].red(), normalMap[index].green(), normalMap[index].blue());
                n -= 127.0f;
                n.normalize();

                // Compute the angle between normal
                // and light position. Angle is scaled
                // by bump amount.
                const eF32 angle = eMax(0.0f, n*position*bumpAmount);

                // Calculate lighting values based on
                // the phong model:
                // color = ambient+diffuse*angle+angle^2*specular
                const eF32 r = ambient.r+angle*(diffuse.r+angle*specular.r*specAmount);
                const eF32 g = ambient.g+angle*(diffuse.g+angle*specular.g*specAmount);
                const eF32 b = ambient.b+angle*(diffuse.b+angle*specular.b*specAmount);

                // Calculate final average color.
                m_bitmap[index].setRed(eMin(eFtoL(r)*inputBmp[index].red()/255, 255));
                m_bitmap[index].setGreen(eMin(eFtoL(g)*inputBmp[index].green()/255, 255));
                m_bitmap[index].setBlue(eMin(eFtoL(b)*inputBmp[index].blue()/255, 255));
            }
        }
    }
OP_END(eBumpOp);
#endif

// Blur (bitmap) operator
// ----------------------
// Applies n passes of vertical and horizontal
// box blur, which approximates a gaussian
// blur when choosing n=3.

#if defined(HAVE_OP_BITMAP_BLUR) || defined(eEDITOR)
OP_DEFINE_BITMAP(eBlurOp, eBlurOp_ID, "Blur", 'b', 1, 1, "-1,Bitmap")
    OP_INIT()
    {
        eOP_PARAM_ADD_ENUM("Passes", "1|2|3|4", 0);
        eOP_PARAM_ADD_FXY("Amount", 0.0f, 16.0f, 0.01f, 0.01f);
        eOP_PARAM_ADD_FLOAT("Amplify", 0.0f, 16.0f, 1.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt passesVal, const eVector2 &amount, eF32 amplify)
    {
        _copyFirstInputBitmap();

        const eU32 passes = passesVal+1;
        const ePoint blurriness(eFtoL(amount.x*m_bmpDimSize[0]), eFtoL(amount.y*m_bmpDimSize[1]));

        // Create bitmap for temporary results.
        eColor *temp = new eColor[m_bmpSize];
        eASSERT(temp != eNULL);

        // Precalculate some values.
	    eU32 andDim[2];
	    eU32 blurSize[2];
	    eU32 *div[2];
	    eU32 deltaDim[] = {1, m_bmpDimSize[0]};

	    for(eU32 d = 0; d < 2; d++)
        {
		    andDim[d] = m_bmpDimSize[d]-1;
		    blurSize[d] = 2*blurriness[d]+1;
		    eU32 dsize = blurSize[d]*m_bmpDimSize[d];

		    div[d] = new eU32[dsize];
            eASSERT(div[d] != eNULL);

		    for(eU32 i = 0; i < dsize; i++)
            {
	            div[d][i] = eMin(eFtoL((eF32)i/(eF32)blurSize[d]*amplify), 255);
            }
	    };
    
	    // Apply blur for the given number of passes.
	    eColor* source = m_bitmap;
	    eColor* target = temp;

        for (eU32 i=0; i<passes; i++)
        {
		    for (eU32 d=0; d<2; d++)
            {
			    for (eU32 k=0, index=0; k<m_bmpDimSize[1-d]; k++)
                {
				    eU32 t[] = {0, 0, 0, 0};

				    const eU32 delta = k * deltaDim[1-d];

				    for (eInt kd=-blurriness[d]; kd<=blurriness[d]; kd++)
                    {
					    const eColor &c = source[delta+(kd&andDim[d]) * deltaDim[d]];

					    for (eU32 ch=0; ch<4; ch++)
                        {
						    t[ch] += c.get(ch);
                        }
				    }

				    for (eU32 ch=0; ch<4; ch++)
                    {
					    target[delta].set(ch, div[d][t[ch]]);
                    }

				    for (eU32 j=1; j<m_bmpDimSize[d]; j++)
                    {
					    const eColor &csub = source[delta+((j-blurriness[d]-1)&andDim[d]) * deltaDim[d]];
					    const eColor &cadd = source[delta+((j+blurriness[d])&andDim[d]) * deltaDim[d]];

					    for (eU32 ch=0; ch<4; ch++)
                        {
						    t[ch] += (eInt)(cadd.get(ch)-csub.get(ch));
						    target[delta+j * deltaDim[d]].set(ch, div[d][t[ch]]);
					    }
				    }
			    }

			    eSwap(source, target);
		    }
	    }

        // Free memory.
        eSAFE_DELETE_ARRAY(div[0]);
        eSAFE_DELETE_ARRAY(div[1]);
        eSAFE_DELETE_ARRAY(temp);
    }
OP_END(eBlurOp);
#endif

// Distord (bitmap) operator
// -------------------------
// Distorts input bitmap 0 on x-axis using color
// information from red channel and on y-axis
// using color information from green channel.

#if defined(HAVE_OP_BITMAP_DISTORT) || defined(eEDITOR)
OP_DEFINE_BITMAP(eDistortOp, eDistortOp_ID, "Distort", 'd', 2, 2, "-1,Bitmap")
    OP_INIT()
    {
       eOP_PARAM_ADD_FXY("Amount", -4.0f, 4.0f, 0.0f, 0.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eVector2 &amountVal)
    {
        const eVector2 amount(amountVal.x*(eF32)m_bmpDimSize[0]/256.0f,
                              amountVal.y*(eF32)m_bmpDimSize[1]/256.0f);

        const Result &inputRes = ((eIBitmapOp *)getInputOperator(0))->getResult();

        const eColor *map = ((eIBitmapOp *)getInputOperator(1))->getResult().bitmap;
        eASSERT(map != eNULL);

        for (eU32 y=0, index=0; y<m_bmpDimSize[1]; y++)
        {
            for (eU32 x=0; x<m_bmpDimSize[0]; x++)
            {
                const eF32 u = x+amount.x*(eF32)map[index].red();
                const eF32 v = y+amount.y*(eF32)map[index].green();

                m_bitmap[index++] = _getBilinearFiltered(inputRes, u, v);
            }
        }
    }
OP_END(eDistortOp);
#endif

// Cells (bitmap) operator
// -----------------------
// Generates cellular textures, which can be used
// e.g. to create bitmaps that look like stone.

#if defined(HAVE_OP_BITMAP_CELLS) || defined(eEDITOR)
OP_DEFINE_BITMAP(eCellsOp, eCellsOp_ID, "Cells", ' ', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_ENUM("Width", "1|2|4|8|16|32|64|128|256|512|1024|2048", DEFSIZE_SEL);
        eOP_PARAM_ADD_ENUM("Height", "1|2|4|8|16|32|64|128|256|512|1024|2048", DEFSIZE_SEL);
        eOP_PARAM_ADD_INT("Points", 1, 64, 5);
        eOP_PARAM_ADD_FLOAT("Regularity", 0.0f, 1.0f, 0.25f);
        eOP_PARAM_ADD_ENUM("Pattern", "Stone|Cobweb", 0);
        eOP_PARAM_ADD_INT("Seed", 0, 65535, 0);
        eOP_PARAM_ADD_LABEL("Coloring", "Coloring");
        eOP_PARAM_ADD_FLOAT("Amplify", 0.0f, 16.0f, 1.0f);
        eOP_PARAM_ADD_FLOAT("Gamma", 0.0f, 16.0f, 1.0f);
        eOP_PARAM_ADD_RGBA("Color 0", 1.0f, 1.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_RGBA("Color 1", 0.0f, 0.0f, 0.0f, 1.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt widthSel, eInt heightSel, eU32 numPoints, eF32 regularityVal, eInt pattern,
            eU32 seed, eF32 amplify, eF32 gamma, const eFloatColor &color0, const eFloatColor &color1)
    {
        const eF32 regularity = 1.0f-regularityVal;

        // Reallocate memory for bitmap, if
        // size has changed.
        const eU32 newWidth = 1<<getParameter(0).getValue().enumSel;
        const eU32 newHeight = 1<<getParameter(1).getValue().enumSel;

        _reallocate(newWidth, newHeight);

        // Generate control points for cells.
        eRandomize(seed);

        eVector2 *points = new eVector2[numPoints*numPoints];
        eASSERT(points != eNULL);

        for (eU32 y=0, index=0; y<numPoints; y++)
        {
            for (eU32 x=0; x<numPoints; x++)
            {
                eVector2 &p = points[index++];

                p.x = (x+0.5f+eRandomF(-0.5f, 0.5f)*regularity)/(eF32)numPoints;
                p.y = (y+0.5f+eRandomF(-0.5f, 0.5f)*regularity)/(eF32)numPoints;
            }
        }

        // Calculate point with shortest distance to
        // current pixel position. Using this distance,
        // the final pixel color is calculated.
        for (eU32 y=0, index=0; y<m_bmpDimSize[1]; y++)
        {
            const eU32 yo = y*numPoints/m_bmpDimSize[1]+numPoints;
            const eF32 yp = (eF32)y/(eF32)m_bmpDimSize[1];

            for (eU32 x=0; x<m_bmpDimSize[0]; x++)
            {
                const eU32 xo = x*numPoints/m_bmpDimSize[0]+numPoints;
                const eF32 xp = (eF32)x/(eF32)m_bmpDimSize[0];

                eF32 maxDist = eF32_MIN;
                eF32 minDist = eF32_MAX;
                eF32 nextMinDist = eF32_MAX;

                for (eInt i=-1; i<2; i++)
                {
                    const eInt io = ((yo+i)%numPoints)*numPoints;

                    for (eInt j=-1; j<2; j++)
                    {
                        eVector2 cellPos = points[((xo+j)%numPoints)+io];

                        // Make cells tileable.
                        if (j == -1 && x*numPoints < m_bmpDimSize[0])
                        {
                            cellPos.x--;
                        }
                        else if (j == 1 && x*numPoints >= m_bmpDimSize[0]*(numPoints-1))
                        {
                            cellPos.x++;
                        }

                        if (i == 1 && y*numPoints >= m_bmpDimSize[1]*(numPoints-1))
                        {
                            cellPos.y++;
                        }
                        else if (i == -1 && y*numPoints < m_bmpDimSize[1])
                        {
                            cellPos.y--;
                        }

                        // Calculate the two shortest distances.
                        const eF32 dist = (eVector2(xp, yp)-cellPos).length()*amplify;

                        if (dist < minDist)
                        {
                            nextMinDist = minDist;
                            minDist = dist;
                        }
                        else if (dist < nextMinDist)
                        {
                            nextMinDist = dist;
                        }

                        if (dist > maxDist)
                        {
                            maxDist = dist;
                        }
                    }
                }

                // Set pixel intensity based on
                // calculated distances and pattern.
                eF32 intensity;

                switch (pattern)
                {
                    case 0: // Stone
                    {
                        intensity = 1.0f-(nextMinDist-minDist)*(eF32)numPoints;
                        break;
                    }

                    case 1: // Cobweb
                    {
                        intensity = 1.0f-minDist*(eF32)numPoints;
                        break;
                    }
                }

                intensity = eClamp(0.0f, intensity*gamma, 1.0f);

                // Set color value for current pixel.
                m_bitmap[index++] = eColor(color0).lerp(color1, intensity);
            }
        }

        eSAFE_DELETE_ARRAY(points);
    }
OP_END(eCellsOp);
#endif

// Mask (bitmap) operator
// ----------------------
// Uses third input bitmap as "blend map" (mask)
// for combining first and second input bitmaps.

#if defined(HAVE_OP_BITMAP_MASK) || defined(eEDITOR)
OP_DEFINE_BITMAP(eMaskOp, eMaskOp_ID, "Mask", 'k', 3, 3, "-1,Bitmap")
    OP_EXEC(eGraphicsApiDx9 *gfx)
    {
        const eColor *inputBmp0 = ((eIBitmapOp *)getInputOperator(0))->getResult().bitmap;
        eASSERT(inputBmp0 != eNULL);
        const eColor *inputBmp1 = ((eIBitmapOp *)getInputOperator(1))->getResult().bitmap;
        eASSERT(inputBmp1 != eNULL);
        const eColor *maskBmp = ((eIBitmapOp *)getInputOperator(2))->getResult().bitmap;
        eASSERT(maskBmp != eNULL);

        for (eU32 i=0; i<m_bmpSize; i++)
        {
            m_bitmap[i] = inputBmp0[i]*maskBmp[i]+inputBmp1[i]*(eColor::WHITE-maskBmp[i]);
        }
    }
OP_END(eMaskOp);
#endif

// Twirl (bitmap) operator
// -----------------------
// Twirls (rotates) the input bitmap at the given
// center, using the given radius and strength.

#if defined(HAVE_OP_BITMAP_TWIRL) || defined(eEDITOR)
OP_DEFINE_BITMAP(eTwirlOp, eTwirlOp_ID, "Twirl", ' ', 1, 1, "-1,Bitmap")
    OP_INIT()
    {
        eOP_PARAM_ADD_FLOAT("Strength", -16.0f, 16.0f, 0.0f);
        eOP_PARAM_ADD_FXY("Center", 0.0f, 2.0f, 0.5f, 0.5f);
        eOP_PARAM_ADD_FXY("Radius", eALMOST_ZERO, 2.0f, 0.25f, 0.25f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 strength, const eVector2 &centerVal, const eVector2 &radiusVal)
    {
        // Calculate absolute coordinates.
        const eVector2 center(centerVal.x*(eF32)m_bmpDimSize[0], centerVal.y*(eF32)m_bmpDimSize[1]);
        const eVector2 radius(radiusVal.x*(eF32)m_bmpDimSize[0], radiusVal.y*(eF32)m_bmpDimSize[1]);

        const eF32 rr = radius.x*radius.y;

        // Twirl input bitmap.
        const Result &inputRes = ((eIBitmapOp *)getInputOperator(0))->getResult();

        for (eU32 y=0, index=0; y<m_bmpDimSize[1]; y++)
        {
            const eF32 dy = ((eF32)y-center.y)/radius.y;
            const eF32 dy2 = dy*dy;

            for (eU32 x=0; x<m_bmpDimSize[0]; x++, index++)
            {
                const eF32 dx = ((eF32)x-center.x)/radius.x;
                const eF32 dist = eSqrt(dx*dx+dy2);

                if (dist >= 1.0f)
                {
                    m_bitmap[index] = inputRes.bitmap[index];
                }
                else
                {
                    // angle = sin(distance*PI/2)
                    // approximate using:
                    // angle = fastCos(distance*PI/2-PI/2);
                    const eF32 amount = (1.0f-eFastCos(dist*eHALFPI-eHALFPI))*strength;

                    eVector2 pos((eF32)x-center.x, (eF32)y-center.y);
                    pos.rotate(amount);
                    pos += eVector2(center.x, center.y);

                    m_bitmap[index] = _getBilinearFiltered(inputRes, pos.x, pos.y);
                }
            }
        }
    }
OP_END(eTwirlOp);
#endif

// Import (bitmap) operator
// ------------------------
// Imports an image from file.

#if defined(HAVE_OP_BITMAP_IMPORT) || defined(eEDITOR)
OP_DEFINE_BITMAP(eBitmapImportOp, eBitmapImportOp_ID, "Import", 'i', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_FILE("Path", "");
        eOP_PARAM_ADD_ENUM("Mode", "Stretch|Wrap", 0);
        eOP_PARAM_ADD_BOOL("Ignore alpha", eFALSE);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eChar *path, eInt mode, eBool ignoreAlpha)
    {
        eFile f(path);

        if (f.open())
        {
            eByteArray fileData;
            f.readAll(fileData);

            // Try to open image file. If it can't be opened,
            // or it's to big, a black empty bitmap is generated.
            eColor *imgData = eNULL;
            eU32 imgWidth, imgHeight;

            if (gfx->loadImage(fileData, imgData, imgWidth, imgHeight) && imgWidth <= 2048 && imgHeight <= 2048)
            {
                eASSERT(imgData != eNULL);

                // Calculate new size, which has to be
                // a power of 2 (image will be stretched,
                // if size doesn't fit).
                const eU32 newWidth = eNextPowerOf2(imgWidth);
                const eU32 newHeight = eNextPowerOf2(imgHeight);

                _reallocate(newWidth, newHeight);
                eMemSet(m_bitmap, 0, m_bmpSize*sizeof(eColor));

                // Depending on mode image is stretched or wrapped.
                const eF32 stepx = (!mode ? (eF32)imgWidth/(eF32)newWidth : (eF32)newWidth/(eF32)imgWidth);
                const eF32 stepy = (!mode ? (eF32)imgHeight/(eF32)newHeight : (eF32)newHeight/(eF32)imgHeight);

                // Copy image data to bitmap buffer.
                for (eU32 y=0, index=0; y<newHeight; y++)
                {
                    const eF32 v = (eF32)y*stepy;

                    for (eU32 x=0; x<newWidth; x++)
                    {
                        const eF32 u = (eF32)x*stepx;

                        eU32 imgSize = imgWidth*imgHeight;
                        Result res(imgWidth, imgHeight, imgSize, imgData);

                        m_bitmap[index++] = _getBilinearFiltered(res, u, v);
                    }
                }

                if (ignoreAlpha)
                {
                    for (eU32 i=0; i<m_bmpSize; i++)
                    {
                        m_bitmap[i].setAlpha(255);
                    }
                }

                eSAFE_DELETE_ARRAY(imgData);
                return;
            }
        }

        // Loading unsuccessful => black bitmap.
        _reallocate(DEFAULT_SIZE, DEFAULT_SIZE);
        eMemSet(m_bitmap, 0, m_bmpSize*sizeof(eColor));
    }
OP_END(eBitmapImportOp);
#endif

// Sine plasma (bitmap) operator
// -----------------------------
// Creates a sine plasma.

#if defined(HAVE_OP_BITMAP_SINE_PLASMA) || defined(eEDITOR)
OP_DEFINE_BITMAP(eSinePlasmaOp, eSinePlasmaOp_ID, "Sine plasma", 's', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_ENUM("Width", "1|2|4|8|16|32|64|128|256|512|1024|2048", DEFSIZE_SEL);
        eOP_PARAM_ADD_ENUM("Height", "1|2|4|8|16|32|64|128|256|512|1024|2048", DEFSIZE_SEL);
        eOP_PARAM_ADD_IXY("Count", 1, eU16_MAX, 3, 3);
        eOP_PARAM_ADD_IXY("Shift", 0, eU16_MAX, 0, 0);
        eOP_PARAM_ADD_RGBA("Plasma color", 1.0f, 1.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_RGBA("Background color", 0.0f, 0.0f, 0.0f, 1.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt widthSel, eInt heightSel, const eIXY &count,
            const eIXY &shift, const eFloatColor &plasmaCol, const eFloatColor &bgCol)
    {
        const eU32 newWidth = 1<<widthSel;
        const eU32 newHeight = 1<<heightSel;

        _reallocate(newWidth, newHeight);

        const eF32 sinDelta = eTWOPI/((eF32)m_bmpDimSize[0]/count.x);
        const eF32 cosDelta = eTWOPI/((eF32)m_bmpDimSize[1]/count.y);

        for (eU32 y=0, index=0; y<m_bmpDimSize[1]; y++)
        {
            eF32 cosVal = eCos(y*cosDelta+(eF32)shift.y); // In range [-1,1].

            for (eU32 x=0; x<m_bmpDimSize[0]; x++) 
            {
                const eF32 sinVal = eSin(x*sinDelta+(eF32)shift.x); // In range [-1,1].
                const eF32 alpha = (sinVal+cosVal+2.0f)*0.25f; // In range [0,1].

                m_bitmap[index++] = alpha*eColor(plasmaCol)+(1.0f-alpha)*eColor(bgCol);
            }
        }
    }
OP_END(eSinePlasmaOp);
#endif

// Line (bitmap) operator
// ----------------------
// Draws a line.

#if defined(HAVE_OP_BITMAP_LINE) || defined(eEDITOR)
OP_DEFINE_BITMAP(eLineOp, eLineOp_ID, "Line", 'l', 1, 1, "-1,Bitmap")
    OP_INIT()
    {
        eOP_PARAM_ADD_FXY("Start", 0.0f, 1.0f, 0.0f, 0.0f);
        eOP_PARAM_ADD_FXY("End", 0.0f, 1.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_FLOAT("Thickness", 0.1f, eF32_MAX, 1.2f);
        eOP_PARAM_ADD_FLOAT("Decay", 0.0f, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_RGBA("Color", 1.0f, 1.0f, 1.0f, 1.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eVector2 &startVal, const eVector2 &endVal, eF32 thickness, eF32 decay, const eFloatColor &color)
    {
        const eVector2 start(startVal.x*(eF32)m_bmpDimSize[0], startVal.y*(eF32)m_bmpDimSize[1]);
        const eVector2 end(endVal.x*(eF32)m_bmpDimSize[0], endVal.y*(eF32)m_bmpDimSize[1]);

        const eColor *inputBmp = ((eIBitmapOp *)getInputOperator(0))->getResult().bitmap;
        eASSERT(inputBmp != eNULL);

        const eVector2 vecAb = end-start;
        const eF32 lineLen = vecAb.length();
        const eF32 invLineLenQ = 1.0f/(lineLen*lineLen);

        for (eU32 y=0, index=0; y<m_bmpDimSize[1]; y++)
        {
            for (eU32 x=0; x<m_bmpDimSize[0]; x++) 
            {
                const eVector2 point((eF32)x, (eF32)y);
                const eF32 u = ((point.x-start.x)*(end.x-start.x)+(point.y-start.y)*(end.y-start.y))*invLineLenQ;

                eF32 alpha = 1.0f;

                if (u >= 0.0f && u <= 1.0f)
                {
                    const eVector2 intersection = start+u*vecAb;
                    const eF32 distance = (point-intersection).length();
                    const eF32 thicknessDist = distance-thickness;

                    alpha = eClamp(0.0f, thicknessDist/decay, 1.0f);
                }
                /*
                // decay for line ends, not working completely yet
                else
                {
                    eF32 pixels = u * lineLen;
                    if (pixels < 0.0f && pixels > -decay)
                        alpha = eClamp(0.0f, -pixels / decay, 1.0f);
                    else if (pixels > lineLen && pixels - lineLen < decay)
                        alpha = eClamp(0.0f, (pixels-lineLen)/decay, 1.0f);
                }
                */

                const eColor &orgCol = inputBmp[index];
                const eColor finalCol = (1.0f-alpha)*eColor(color)+alpha*eColor(orgCol);

                m_bitmap[index++] = finalCol;
            }
        }
    }
OP_END(eLineOp);
#endif

// Circle (bitmap) operator
// ------------------------
// Draws a circle.

#if defined(HAVE_OP_BITMAP_CIRCLE) || defined(eEDITOR)
OP_DEFINE_BITMAP(eCircleOp, eCircleOp_ID, "Circle", ' ', 1, 1, "-1,Bitmap")
    OP_INIT()
    {
        eOP_PARAM_ADD_FXY("Position", eF32_MIN, eF32_MAX, 0.5f, 0.5f);
        eOP_PARAM_ADD_FLOAT("Radius", 0.0f, eF32_MAX, 0.2f);
        eOP_PARAM_ADD_FLOAT("Thickness", 0.1f, eF32_MAX, 1.2f);
        eOP_PARAM_ADD_FLOAT("Decay", 1.0f, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_RGBA("Color", 1.0f, 1.0f, 1.0f, 1.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eVector2 &posVal, eF32 radiusVal, eF32 thickness, eF32 decay, const eFloatColor &color)
    {
        const eVector2 position(posVal.x*(eF32)m_bmpDimSize[0], posVal.y*(eF32)m_bmpDimSize[1]);
        const eF32 radius = radiusVal*(eF32)m_bmpDimSize[0];

        const eF32 invDecay = 1.0f/decay;
        const eColor *inputBmp = ((eIBitmapOp *)getInputOperator(0))->getResult().bitmap;
        eASSERT(inputBmp != eNULL);

        for (eU32 y=0, index=0; y<m_bmpDimSize[1]; y++) 
        {
            for (eU32 x=0; x<m_bmpDimSize[0]; x++)
            {
                const eVector2 diff = eVector2((eF32)x, (eF32)y)-position;
                const eF32 centerDist = diff.length();
                const eF32 borderDist = eAbs(centerDist-radius);
                const eF32 thicknessDist = borderDist-thickness;
                const eF32 alpha = eClamp(0.0f, thicknessDist*invDecay, 1.0f);

                const eColor &orgCol = inputBmp[index];
                const eColor finalCol = (1.0f-alpha)*eColor(color)+alpha*eColor(orgCol);

                m_bitmap[index++] = finalCol;
            }
        }
    }
OP_END(eCircleOp);
#endif

// Alpha (bitmap) operator
// -----------------------
// Modifies the alpha channel.

#if defined(HAVE_OP_BITMAP_ALPHA) || defined(eEDITOR)
OP_DEFINE_BITMAP(eAlphaOp, eAlphaOp_ID, "Alpha", ' ', 1, 1, "-1,Bitmap")
    OP_INIT()
    {
       eOP_PARAM_ADD_ENUM("Mode", "From brightness|From brightness inverted", 0);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt mode)
    {
        _copyFirstInputBitmap();

        eU8 sub = 0;
        eU8 mul = 1;

        if (mode == 1)
        {
            sub = 255;
            mul = -1;
        }

        for (eU32 i=0; i<m_bmpSize; i++)
        {
            eColor &col = m_bitmap[i];

            const eU8 gray = col.grayScale();
            const eU8 alpha = sub+mul*gray;

            col.setAlpha(alpha);
        }
    }
OP_END(eAlphaOp);
#endif

// Bricks (bitmap) operator
// ------------------------
// Generates a brick pattern.

#if defined(HAVE_OP_BITMAP_BRICKS) || defined(eEDITOR)
OP_DEFINE_BITMAP(eBricksOp, eBricksOp_ID, "Bricks", ' ', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_ENUM("Width", "1|2|4|8|16|32|64|128|256|512|1024|2048", DEFSIZE_SEL);
        eOP_PARAM_ADD_ENUM("Height", "1|2|4|8|16|32|64|128|256|512|1024|2048", DEFSIZE_SEL);
        eOP_PARAM_ADD_INT("Thickness", 1, 16, 1);
        eOP_PARAM_ADD_FLOAT("Contrast", 0.0f, 1.0f, 0.5f);
        eOP_PARAM_ADD_IXY("Counts", 1, 256, 10, 20);
        eOP_PARAM_ADD_RGBA("Color", 0.0f, 0.0f, 0.0f, 1.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt widthSel, eInt heightSel, eU32 thickness,
            eF32 contrastVal, const ePoint &counts, const eFloatColor &color)
    {
        // Allocate bitmap of required size and
        // initialize it to black.
        const eU32 newWidth = 1<<widthSel;
        const eU32 newHeight = 1<<heightSel;

        _reallocate(newWidth, newHeight);

        // Fill bitmap with color.
        eU32 count = m_bmpSize;
        while (count--)
        {
            m_bitmap[count] = color;
        }

        // Precalculate some values.
        const eU32 contrast = eFtoL(contrastVal*128.0f);

        const eF32 stepX = (eF32)m_bmpDimSize[0]/(eF32)counts.x;
        const eF32 stepY = (eF32)m_bmpDimSize[1]/(eF32)counts.y;

        const eU32 bl = 127+contrast;
        const eU32 bd = 127-contrast;
    
        const eColor lightCol(bl, bl, bl, 255);
        const eColor darkCol(bd, bd, bd, 255);

        // Draw the bricks pattern.
        eU32 b = 1;

        for (eF32 y=0; y<(eF32)m_bmpDimSize[1]; y+=stepY)
        {
            // Draw horizontal lines.
            for (eU32 t=0; t<thickness; t++)
            {
                for (eU32 i=0; i<m_bmpDimSize[0]; i++)
                {
                    eU32 yOff = eFtoL(y+t);

                    if (yOff < m_bmpDimSize[1])
                    {
                        m_bitmap[yOff*m_bmpDimSize[0]+i] = lightCol;
                    }

                    // Implicit < 0 check due to unsigned conversion.
                    yOff = eFtoL(y-(eF32)t-1.0f);

                    if (yOff < m_bmpDimSize[1])
                    {
                        m_bitmap[yOff*m_bmpDimSize[0]+i] = darkCol;
                    }
                }
            }

            for (eF32 x=(b%2 ? stepX : 0.0f); x<=(eF32)m_bmpDimSize[0]; x+=2.0f*stepX)
            {
                // Draw vertical lines.
                for (eU32 t=0; t<thickness; t++)
                {
                    for (eInt i=eFtoL(y+t); i<eFtoL(y+stepY-t-1.0f); i++)
                    {
                        if (i < (eInt)m_bmpDimSize[1])
                        {
                            const eU32 yDelta = i*m_bmpDimSize[0];
                            eU32 xOff = eFtoL(x+t);

                            if (xOff < m_bmpDimSize[0])
                            {
                                m_bitmap[yDelta+xOff] = lightCol;
                            }

                            // Implicit < 0 check due to unsigned conversion.
                            xOff = eFtoL(x+(eF32)t-1.0f);

                            if (xOff < m_bmpDimSize[0])
                            {
                                m_bitmap[yDelta+xOff] = darkCol;
                            }
                        }
                    }
                }
            }

            b++;
        }
    }
OP_END(eBricksOp);
#endif

// Blit (bitmap) operator
// ----------------------
// Blits one bitmap into another.

#if defined(HAVE_OP_BITMAP_BLIT) || defined(eEDITOR)
OP_DEFINE_BITMAP(eBlitOp, eBlitOp_ID, "Blit", ' ', 2, 2, "-1,Bitmap")
    OP_INIT()
    {
        eOP_PARAM_ADD_FXY("Offset", eF32_MIN, eF32_MAX, 0.0f, 0.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eVector2 &offset)
    {
        _copyFirstInputBitmap();

        const Result &res = ((eIBitmapOp *)getInputOperator(1))->getResult();

        const eU32 startX = eFtoL(offset.x*m_bmpDimSize[0]);
        const eU32 startY = eFtoL(offset.y*m_bmpDimSize[1]);

        for (eU32 sy=0, dy=startY; sy<res.height; sy++, dy++)
        {
            if (dy < m_bmpDimSize[1])
            {
                const eU32 dyDelta = dy*m_bmpDimSize[0];
                const eU32 syDelta = sy*res.width;

                for (eU32 sx=0, dx=startX; sx<res.width; sx++, dx++)
                {
                    if (dx < m_bmpDimSize[0])
                    {
                        m_bitmap[dyDelta+dx] = res.bitmap[syDelta+sx];
                    }
                }
            }
        }
    }

#ifdef eEDITOR
    // Omit validity check of bitmap-operators, because
    // the blit operator accepts all input sizes.
    virtual eBool checkValidity() const
    {
        return eIOperator::checkValidity();
    }
#endif
OP_END(eBlitOp);
#endif