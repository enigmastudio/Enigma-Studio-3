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

#include "../eshared.hpp"

// Camera (effect) operator
// ------------------------
// Converts the model input-operator to an effect
// and adds a camera to new effect.

#if defined(HAVE_OP_EFFECT_CAMERA) || defined(eEDITOR)
OP_DEFINE_EFFECT(eFxCameraOp, eFxCameraOp_ID, "Camera", 'c', 1, 1, "0,Misc : Scene")
    OP_INIT()
    {
        eU32 flags = 0;
        eSetBit(flags, 0);
        eSetBit(flags, 1);
        eSetBit(flags, 2);

        eOP_PARAM_ADD_FLOAT("Field of view", eALMOST_ZERO, 180.0f, 45.0f);
        eOP_PARAM_ADD_ENUM("Aspect ratio", "4:3|16:9|16:10", 0);
        eOP_PARAM_ADD_FLOAT("Near plane", eF32_MIN, eF32_MAX, 0.01f);
        eOP_PARAM_ADD_FLOAT("Far plane", eF32_MIN, eF32_MAX, 1000.0f);
        eOP_PARAM_ADD_FXYZ("Position", eF32_MIN, eF32_MAX, 0.0f, 0.0f, 10.0f);
        eOP_PARAM_ADD_FXYZ("Look at", eF32_MIN, eF32_MAX, 0.0f, 0.0f, 0.0f);
        eOP_PARAM_ADD_FXYZ("Up vector", eF32_MIN, eF32_MAX, 0.0f, 1.0f, 0.0f);
    }

    OP_DEINIT()
    {
        eSAFE_DELETE(m_effect);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 fov, eInt aspectSel, eF32 zNear, eF32 zFar,
            const eVector3 &pos, const eVector3 &lookAt, const eVector3 &upVec)
    {
        eScene &scene = ((eISceneOp *)getInputOperator(0))->getResult().scene;

        const eF32 aspectRatios[] =
        {
            4.0f/3.0f,
            16.0f/9.0f,
            16.0f/10.0f
        };

        const eF32 aspect = aspectRatios[aspectSel];

        eCamera cam(fov, aspect, zNear, zFar);
        eMatrix4x4 mtx;
        mtx.lookAt(pos, lookAt, upVec);
        cam.setViewMatrix(mtx);

        eSAFE_DELETE(m_effect);
        m_effect = new eInputEffect(scene, cam);
        eASSERT(m_effect != eNULL);
    }
OP_END(eFxCameraOp);
#endif

// Blur (effect) operator
// ----------------------
// Blurs a render target.

#if defined(HAVE_OP_EFFECT_BLUR) || defined(eEDITOR)
OP_DEFINE_EFFECT(eFxBlurOp, eFxBlurOp_ID, "Blur", 'b', 1, 1, "-1,Effect")
    OP_INIT()
    {
        eOP_PARAM_ADD_INT("Iterations", 0, eU8_MAX, 1);
        eOP_PARAM_ADD_IXY("Distance", 1, 64, 1, 1);
        eOP_PARAM_ADD_ENUM("Direction", "Horizontal|Vertical|Both", 2);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eU32 iterations, const ePoint &dist, eInt dir)
    {
        // Add horizontal blur effect.
        if (dir == 0 || dir == 2)
        {
            m_fxBlurH = eBlurEffect(eBlurEffect::DIR_HORZ, dist.x);
            m_fxBlurH.setIterations(iterations);

            _appendEffect(&m_fxBlurH);
        }
    
        // Add vertical blur effect.
        if (dir == 1 || dir == 2)
        {
            m_fxBlurV = eBlurEffect(eBlurEffect::DIR_VERT, dist.y);
            m_fxBlurV.setIterations(iterations);

            _appendEffect(&m_fxBlurV);
        }
    }

    OP_VAR(eBlurEffect m_fxBlurH);
    OP_VAR(eBlurEffect m_fxBlurV);
OP_END(eFxBlurOp);
#endif

// Merge (effect) operator
// -----------------------
// Merges two or more input render-targets.

#if defined(HAVE_OP_EFFECT_MERGE) || defined(eEDITOR)
OP_DEFINE_EFFECT(eFxMergeOp, eFxMergeOp_ID, "Merge", 'm', 2, 64, "-1,Effect")
    OP_INIT()
    {
        eOP_PARAM_ADD_ENUM("Blending", "Additive|Subtractive|Multiplicative|Brighter|Darker", 0);
        eOP_PARAM_ADD_FXY("Blend ratios", 0.0f, 1.0f, 1.0f, 1.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eMergeEffect::BlendMode blendMode, const eVector2 &blendRatios)
    {
        m_fxMerge = eMergeEffect(blendMode, blendRatios);
        _appendEffect(&m_fxMerge);

        for (eU32 i=1; i<getInputCount(); i++)
        {
            const eIEffectOp::Result &res = ((eIEffectOp *)getInputOperator(i))->getResult();
            m_fxMerge.addInput(res.effect);
        }
    }

    OP_VAR(eMergeEffect m_fxMerge);
OP_END(eFxMergeOp);
#endif

// Adjust (effect) operator
// ------------------------
// Adjusts brightness/contrast/color of a render target.

#if defined(HAVE_OP_EFFECT_ADJUST) || defined(eEDITOR)
OP_DEFINE_EFFECT(eFxAdjustOp, eFxAdjustOp_ID, "Adjust", 'a', 1, 1, "-1,Effect")
    OP_INIT()
    {
        eOP_PARAM_ADD_INT("Iterations", 0, eU8_MAX, 1);
        eOP_PARAM_ADD_FLOAT("Brightness", 0.0f, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_FLOAT("Contrast", 0.0f, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_RGB("Color", 1.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_RGB("Subtract", 0.0f, 0.0f, 0.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eU32 iterations, eF32 brightness, eF32 contrast,
            const eFloatColor &adjCol, const eFloatColor &subCol)
    {
        m_fxAdjust = eAdjustEffect(brightness, contrast, adjCol, subCol);
        _appendEffect(&m_fxAdjust);
    }

    OP_VAR(eAdjustEffect m_fxAdjust);
OP_END(eFxAdjustOp);
#endif

// Radial blur (effect) operator
// -----------------------------
// Applies a radial blur on a render target.

#if defined(HAVE_OP_EFFECT_RADIAL_BLUR) || defined(eEDITOR)
OP_DEFINE_EFFECT(eFxRadialBlurOp, eFxRadialBlurOp_ID, "Radial blur", 'i', 1, 1, "-1,Effect")
    OP_INIT()
    {
        eOP_PARAM_ADD_FXY("Origin", 0.0f, 1.0f, 0.5f, 0.5f);
        eOP_PARAM_ADD_FLOAT("Distance", 0.0f, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_FLOAT("Strength", 0.0f, eF32_MAX, 2.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eVector2 &origin, eF32 dist, eF32 strength)
    {
        m_fxRadial = eRadialBlurEffect(origin, dist, strength);
        _appendEffect(&m_fxRadial);
    }

    OP_VAR(eRadialBlurEffect m_fxRadial);
OP_END(eFxRadialBlurOp);
#endif

// Ripple (effect) operator
// ------------------------
// Ripples a render target.

#if defined(HAVE_OP_EFFECT_RIPPLE) || defined(eEDITOR)
OP_DEFINE_EFFECT(eFxRippleOp, eFxRippleOp_ID, "Ripple", 'r', 1, 1, "-1,Effect")
    OP_INIT()
    {
        eOP_PARAM_ADD_INT("Iterations", 0, eU8_MAX, 1);
        eOP_PARAM_ADD_FLOAT("Amplitude", 0.0f, eF32_MAX, 0.5f);
        eOP_PARAM_ADD_FLOAT("Length", 0.01f, eF32_MAX, 5.0f);
        eOP_PARAM_ADD_FLOAT("Speed", 0.0f, eF32_MAX, 5.0f);
        eOP_PARAM_ADD_FLOAT("Time", 0.0f, eF32_MAX, 0.0f);
        eOP_PARAM_ADD_FXY("Offset", 0.0f, 1.0f, 0.5f, 0.5f);
        eOP_PARAM_ADD_ENUM("Mode", "Standard|Concentric", 0);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eU32 iterations, eF32 ampli, eF32 length, eF32 speed,
            eF32 time, const eVector2 &offset, eRippleEffect::Mode mode)
    {
        ampli /= 10.0f;

        m_fxRipple = eRippleEffect(ampli, length, speed, time, offset, mode);
        m_fxRipple.setIterations(iterations);

        _appendEffect(&m_fxRipple);
    }

    OP_VAR(eRippleEffect m_fxRipple);
OP_END(eFxRippleOp);
#endif

// Fog (effect) operator
// ---------------------
// Adds fog (linear, exponential, exponential squared)
// to a render target.

#if defined(HAVE_OP_EFFECT_FOG) || defined(eEDITOR)
OP_DEFINE_EFFECT(eFxFogOp, eFxFogOp_ID, "Fog", 'f', 1, 1, "-1,Effect")
    OP_INIT()
    {
        eOP_PARAM_ADD_ENUM("Type", "Linear|Exponential|Exponential squared", 0);
        eOP_PARAM_ADD_FLOAT("Start", 0.01f, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_FLOAT("End", 0.01f, eF32_MAX, 10.0f);
        eOP_PARAM_ADD_FLOAT("Density", 0.01f, eF32_MAX, 0.1f);
        eOP_PARAM_ADD_RGB("Color", 1.0f, 1.0f, 1.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt type, eF32 start, eF32 end, eF32 density, const eFloatColor &color)
    {
        m_fxFog = eFogEffect((eFogEffect::Type)type, start, end, density, color);
        _appendEffect(&m_fxFog);
    }

    OP_VAR(eFogEffect m_fxFog);
OP_END(eFxFogOp);
#endif

// Save (effect) operator
// ----------------------
// Copies render target into another render target,
// which can the be used for texturing purposes
// (aka render-to-texture).

#if defined(HAVE_OP_EFFECT_SAVE) || defined(eEDITOR)
OP_DEFINE_EFFECT(eFxSaveOp, eFxSaveOp_ID, "Save", 's', 1, 1, "-1,Effect")
    OP_INIT()
    {
        eOP_PARAM_ADD_LINK("Render target", "R2T");
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eIRenderToTextureOp *r2tOp)
    {
        if (r2tOp)
        {
            const eIRenderToTextureOp::Result &res = r2tOp->getResult();

            m_fxSave = eSaveEffect(res.renderTarget, res.depthTarget);
            _appendEffect(&m_fxSave);
        }
    }

    OP_VAR(eSaveEffect m_fxSave);
OP_END(eFxSaveOp);
#endif

// Depth of field (effect) operator
// --------------------------------
// Adds a depth of field effect to a render target.
// Requires an additional normal and blurred target
// as input operators.

#if defined(HAVE_OP_EFFECT_DOF) || defined(eEDITOR)
OP_DEFINE_EFFECT(eFxDofOp, eFxDofOp_ID, "DOF", 'o', 2, 2, "-1,Effect")
    OP_INIT()
    {
        eOP_PARAM_ADD_FLOAT("Focus depth", 0.01f, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_FLOAT("Focus range", 0.01f, eF32_MAX, 10.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 focusDepth, eF32 focusRange)
    {
        m_fxDof = eDofEffect(focusDepth, focusRange);
        _appendEffect(&m_fxDof);
        m_fxDof.addInput(((eIEffectOp *)getInputOperator(1))->getResult().effect);
    }

    OP_VAR(eDofEffect m_fxDof);
OP_END(eFxDofOp);
#endif

// Downsample (effect) operator
// ----------------------------
// Downsamples a render target.

#if defined(HAVE_OP_EFFECT_DOWNSAMPLE) || defined(eEDITOR)
OP_DEFINE_EFFECT(eFxDownsampleOp, eFxDownsampleOp_ID, "Downsample", 'd', 1, 1, "-1,Effect")
    OP_INIT()
    {
        eOP_PARAM_ADD_INT("Iterations", 0, eU8_MAX, 1);
        eOP_PARAM_ADD_FXY("Amount", 0.01f, 1.0f, 0.5f, 0.5f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eU32 iterations, const eVector2 &amount) 
    {
        m_fxDown = eDownsampleEffect(amount);
        m_fxDown.setIterations(iterations);

        _appendEffect(&m_fxDown);
    }

    OP_VAR(eDownsampleEffect m_fxDown);
OP_END(eFxDownsampleOp);
#endif

// SSAO (effect) operator
// ----------------------
// Adds screen space ambient occlusion to a render target.
// Requires an additional noise lookup texture linked.

#if defined(HAVE_OP_EFFECT_SSAO) || defined(eEDITOR)
OP_DEFINE_EFFECT(eFxSsaoOp, eFxSsaoOp_ID, "SSAO", 's', 1, 1, "-1,Effect")
    OP_INIT()
    {
        m_noiseMap = eNULL;

        eOP_PARAM_ADD_FLOAT("Scale", 0.01f, eF32_MAX, 0.5f);
        eOP_PARAM_ADD_FLOAT("Intensity", 0.01f, eF32_MAX, 2.5f);
        eOP_PARAM_ADD_FLOAT("Bias", 0.01f, eF32_MAX, 0.25f);
        eOP_PARAM_ADD_FLOAT("Radius", 0.01f, eF32_MAX, 0.5f);
        eOP_PARAM_ADD_LINK("Noise map", "Bitmap");
    }

    OP_DEINIT()
    {
        eSAFE_DELETE(m_noiseMap);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 scale, eF32 intensity, eF32 bias, eF32 radius, eIBitmapOp *noiseMapOp) 
    {
        if (getParameter(4).getChanged() && noiseMapOp)
        {
            const eIBitmapOp::Result &res = noiseMapOp->getResult();

            eSAFE_DELETE(m_noiseMap);
            m_noiseMap = gfx->createTexture2d(res.width, res.height, eFALSE, eFALSE, eFALSE, eFORMAT_ARGB8);
            ePtr data = m_noiseMap->lock();
            eMemCopy(data, res.bitmap, res.size*sizeof(eColor));
            m_noiseMap->unlock();
        }

        // Add effect if there's a valid noise map given.
        if (m_noiseMap)
        {
            m_fxSsao = eSsaoEffect(scale, intensity, bias, radius, m_noiseMap);
            m_fxSsao.setNoiseMap(m_noiseMap);

            _appendEffect(&m_fxSsao);
        }
    }

    OP_VAR(eSsaoEffect   m_fxSsao);
    OP_VAR(eITexture2d * m_noiseMap);
OP_END(eFxSsaoOp);
#endif

// FXAA (effect) operator
// ---------------------
// Adds screen space ambient occlusion to a render target.
// Requires an additional noise lookup texture linked.

#if defined(HAVE_OP_EFFECT_FXAA) || defined(eEDITOR)
OP_DEFINE_EFFECT(eFxFxaaOp, eFxFxaaOp_ID, "FXAA", 'x', 1, 1, "-1,Effect")
    OP_EXEC(eGraphicsApiDx9 *gfx) 
    {
        m_fxFxaa.clearInputs();
        _appendEffect(&m_fxFxaa);
    }

    OP_VAR(eFxaaEffect m_fxFxaa);
OP_END(eFxFxaaOp);
#endif

// Color Grading (effect) operator
// ----------------------
// Adds color grading to a render target.
// Requires an additional color grading lookup texture linked.

#if defined(HAVE_OP_EFFECT_COLOR_GRADING) || defined(eEDITOR)
OP_DEFINE_EFFECT(eFxColorGradingOp, eFxColorGradingOp_ID, "Color grading", ' ', 1, 1, "-1,Effect")
    OP_INIT()
    {
		eOP_PARAM_ADD_LINK("Lookup map", "Bitmap");

        m_lookupMap = eNULL;
    }

    OP_DEINIT()
    {
        eSAFE_DELETE(m_lookupMap);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eIBitmapOp *lookupMapOp) 
    {
        if (getParameter(0).getChanged() && lookupMapOp)
        {
            const eIBitmapOp::Result &res = lookupMapOp->getResult();

            eSAFE_DELETE(m_lookupMap);

			if (res.width == res.height * res.height)
			{
				m_lookupMap = gfx->createTexture3d(res.height, res.height, res.height, eFALSE, eFALSE, eFORMAT_ARGB8);
				ePtr data = m_lookupMap->lock();
				eMemCopy(data, res.bitmap, res.size*sizeof(eColor));
				m_lookupMap->unlock();
			}
        }

        // Add effect if there's a valid grading bitmap given.
        if (m_lookupMap)
        {
            m_fxColorGrading = eColorGradingEffect(m_lookupMap);
            _appendEffect(&m_fxColorGrading);
        }
    }

    OP_VAR(eColorGradingEffect  m_fxColorGrading);
    OP_VAR(eITexture3d *        m_lookupMap);
OP_END(eFxColorGradingOp);
#endif

// Distort (effect) operator
// ----------------------
// Adds distortion to a render target.
// Requires an additional distortion lookup normal texture linked.

#if defined(HAVE_OP_EFFECT_DISTORT) || defined(eEDITOR)
OP_DEFINE_EFFECT(eFxDistortOp, eFxDistortOp_ID, "Distort", 's', 1, 1, "-1,Effect")
	OP_INIT()
	{
		m_distortMap = eNULL;

		eOP_PARAM_ADD_FXY("Intensity", -eF32_MAX, eF32_MAX, 1.0f, 1.0f);
		eOP_PARAM_ADD_FXY("Offset", -eF32_MAX, eF32_MAX, 0.0f, 0.0f);
		eOP_PARAM_ADD_LINK("Distort map", "Bitmap");
	}

	OP_DEINIT()
	{
		eSAFE_DELETE(m_distortMap);
	}

	OP_EXEC(eGraphicsApiDx9 *gfx, const eVector2 &intensity, const eVector2 &offset, eIBitmapOp *distortMapOp) 
	{
		if (getParameter(2).getChanged() && distortMapOp)
		{
			const eIBitmapOp::Result &res = distortMapOp->getResult();

			eSAFE_DELETE(m_distortMap);
			m_distortMap = gfx->createTexture2d(res.width, res.height, eFALSE, eFALSE, eFALSE, eFORMAT_ARGB8);
			ePtr data = m_distortMap->lock();
			eMemCopy(data, res.bitmap, res.size*sizeof(eColor));
			m_distortMap->unlock();
		}

		// Add effect if there's a valid noise map given.
		if (m_distortMap)
		{
			m_fxDistort = eDistortEffect(intensity, offset, m_distortMap);
			m_fxDistort.setDistortMap(m_distortMap);

			_appendEffect(&m_fxDistort);
		}
	}

	OP_VAR(eDistortEffect m_fxDistort);
	OP_VAR(eITexture2d * m_distortMap);
OP_END(eFxDistortOp);
#endif