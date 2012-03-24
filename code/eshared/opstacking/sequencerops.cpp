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

// Scene (sequencer) operator
// --------------------------
// Adds a scene entry to the sequencer.

#if defined(HAVE_OP_SEQUENCER_SCENE) || defined(eEDITOR)
OP_DEFINE_SEQ(eSeqSceneOp, eSeqSceneOp_ID, "Scene", 's', 1, 1, "0,Effect")
    OP_INIT()
    {
        eOP_PARAM_ADD_FLOAT("Start time", 0.0f, eF32_MAX, 0.0f);
        eOP_PARAM_ADD_FLOAT("Duration", 0.01f, eF32_MAX, 10.0f);
        eOP_PARAM_ADD_INT("Track", 0, eSequencer::MAX_TRACKS-1, 0);
        eOP_PARAM_ADD_LABEL("Extended properties", "Extended properties");
        eOP_PARAM_ADD_FLOAT("Time offset", 0.0f, eF32_MAX, 0.0f);
        eOP_PARAM_ADD_FLOAT("Time scale", eF32_MIN, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_ENUM("Blending", "Additive|Subtractive|Multiplicative|Brighter|Darker|None", 0);
        eOP_PARAM_ADD_FXY("Blend ratios", 0.0f, 1.0f, 1.0f, 1.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 startTime, eF32 duration, eU32 track,
            eF32 timeOffset, eF32 timeScale, eInt blendMode, const eVector2 &blendRatios) 
    {
        const eIEffectOp::Result &inputRes = ((eIEffectOp *)getInputOperator(0))->getResult();

        eSequencer::Entry entry;

        entry.type = eSequencer::Entry::TYPE_SCENE;
        entry.startTime = startTime;
        entry.duration = duration;
        entry.blendMode = (eSequencer::Entry::BlendMode)blendMode;
        entry.blendRatios = blendRatios;

        entry.scene.effect = inputRes.effect;
        entry.scene.timeOffset = timeOffset;
        entry.scene.timeScale = timeScale;

        m_sequencer.addEntry(entry, track);
    }
OP_END(eSeqSceneOp);
#endif

// Overlay (sequencer) operator
// ----------------------------
// Adds an overlay entry to the sequencer.

#if defined(HAVE_OP_SEQUENCER_OVERLAY) || defined(eEDITOR)
OP_DEFINE_SEQ(eSeqOverlayOp, eSeqOverlayOp_ID, "Overlay", 'o', 1, 1, "0,Bitmap")
    OP_INIT()
    {
        eOP_PARAM_ADD_FLOAT("Start time", 0.0f, eF32_MAX, 0.0f);
        eOP_PARAM_ADD_FLOAT("Duration", 10.0f, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_INT("Track", 0, eSequencer::MAX_TRACKS-1, 0);
        eOP_PARAM_ADD_LABEL("Extended properties", "Extended properties");
        eOP_PARAM_ADD_ENUM("Blending", "Additive|Subtractive|Multiplicative|Brighter|Darker|None", 0);
        eOP_PARAM_ADD_FXY("Blend ratios", 0.0f, 1.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_FXYZW("Rectangle", eF32_MIN, eF32_MAX, 0.0f, 0.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_FXY("Scroll U/V", eF32_MIN, eF32_MAX, 0.0f, 0.0f);
        eOP_PARAM_ADD_FXY("Tile U/V", eF32_MIN, eF32_MAX, 1.0f, 1.0f);
        eOP_PARAM_ADD_ENUM("U/V address mode", "Wrap|Clamp|Mirror", 1);
        eOP_PARAM_ADD_BOOL("Filtered", eTRUE);

        m_overlayTex = eNULL;
    }

    OP_DEINIT()
    {
        eSAFE_DELETE(m_overlayTex);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 startTime, eF32 duration, eU32 track, eInt blendMode, const eVector2 &blendRatios,
            const eVector4 &rect, const eVector2 &scrollUv, const eVector2 &tileUv, eInt uvAddrMode, eBool filtered)
    {
        const eIBitmapOp *bmpOp = (eIBitmapOp *)getInputOperator(0);
        eASSERT(bmpOp != eNULL);
        const eIBitmapOp::Result &bmpRes = bmpOp->getResult();

        eSAFE_DELETE(m_overlayTex);
        m_overlayTex = gfx->createTexture2d(bmpRes.width, bmpRes.height, eFALSE, eTRUE, bmpOp->isAffectedByAnimation(), eFORMAT_ARGB8);
        ePtr data = m_overlayTex->lock();
        eMemCopy(data, bmpRes.bitmap, bmpRes.size*sizeof(eColor));
        m_overlayTex->unlock();

        eSequencer::Entry entry;

        entry.type = eSequencer::Entry::TYPE_OVERLAY;
        entry.duration = duration;
        entry.startTime = startTime;
        entry.blendMode = (eSequencer::Entry::BlendMode)blendMode;
        entry.blendRatios = blendRatios;
    
        entry.overlay.rect = rect;
        entry.overlay.tileUv = tileUv;
        entry.overlay.texture = m_overlayTex;
        entry.overlay.filtered = filtered;
        entry.overlay.scrollUv = scrollUv;
        entry.overlay.uvAddrMode = (eTextureAddressMode)uvAddrMode;

        m_sequencer.addEntry(entry, track);
    }

    OP_VAR(eITexture2d *m_overlayTex);
OP_END(eSeqOverlayOp);
#endif