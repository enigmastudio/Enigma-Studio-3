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

// Demo (misc) operator
// --------------------
// Operator which holds all data relevant for
// a complete demo (sequencer + sound).

OP_DEFINE(eDemoOp, eDemoOp_ID, eIDemoOp, "Demo", Misc, Misc_CID, eColor(170, 170, 170), 'd', 1, 256, "-1,Sequencer")
    OP_INIT()
    {
        eOP_PARAM_ADD_ENUM("Aspect ratio", "4:3|16:9|16:10", 0);
        eOP_PARAM_ADD_STRING("Production name", "Unnamed");
        eOP_PARAM_ADD_LINK("Loading screen", "Misc : Demo");
        eOP_PARAM_ADD_SYNTH("Song");
		eOP_PARAM_ADD_INT("Rotation start", 0, eSequencer::MAX_TRACKS, 0);
		eOP_PARAM_ADD_INT("Rotation end", 0, eSequencer::MAX_TRACKS, 0);
		eOP_PARAM_ADD_INT("Rotation current", 0, eSequencer::MAX_TRACKS, 0);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt aspectRatioSel, const eChar *prodName, eDemoOp *loadingScreenOp, const eChar *songName, eInt rotationStart, eInt rotationEnd, eInt rotationCurrent)
    {
        // Set aspect ratio.
        const eF32 aspectRatios[] =
        {
            4.0f/3.0f,
            16.0f/9.0f,
            16.0f/10.0f
        };

        eSequencer &seq = m_demo.getSequencer();
        seq.setAspectRatio(aspectRatios[aspectRatioSel]);
		seq.setRotation(rotationStart, rotationEnd, rotationCurrent);

        // Copy over sequencer entries.
        seq.clear();

        for (eU32 i=0; i<getInputCount(); i++)
        {
            const eSequencer &curSeq = ((eISequencerOp *)getInputOperator(i))->getResult().sequencer;
            seq.merge(curSeq);
        }

        // Set song to be played.
        if (getParameter(3).getChanged())
        {
            for (eU32 i=0; i<eDemoData::getSongCount(); i++)
            {
                tfSong *song = eDemoData::getSongByIndex(i);
                eASSERT(song != eNULL);

                if (eStrCompare(songName, song->getUserName()) == 0)
                {
                    m_demo.setSong(song);
                    break;
                }
            }
        }
    }

    eBool process(eIRenderer *renderer, eF32 time, eBool (*callback)(eIRenderer *renderer, eU32 processed, eU32 total, ePtr param), ePtr param)
    {
        // Process the whole stack.
        if (m_processAll)
        {
            return eIOperator::process(renderer, time, callback, param);
        }

        // Just process the currently active sequencer operators.
        eBool changed = eFALSE;

        for (eU32 i=0; i<m_inputOps.size(); i++)
        {
            eISequencerOp *seqOp = (eISequencerOp *)m_inputOps[i];
            eASSERT(seqOp != eNULL);
            eASSERT(TEST_CATEGORY(seqOp, "Sequencer", Sequencer_CID));

            const eF32 startTime = seqOp->getParameter(0).getValue().flt;
            const eF32 duration = seqOp->getParameter(1).getValue().flt;
            const eU32 track = seqOp->getParameter(2).getValue().integer;
            
            eU32 rotationStart = 0;
            eU32 rotationEnd = 0;
            eU32 rotationCurrent = 0;
            m_demo.getSequencer().getRotation(rotationStart, rotationEnd, rotationCurrent);      

            if (rotationStart != rotationEnd) {
                if (track != rotationCurrent && track >= rotationStart && track <= rotationEnd)
                    continue;
            }

            if (time >= startTime && time <= startTime+duration)
            {
                if (seqOp->process(renderer, time-startTime, callback, param))
                {
                    changed = eTRUE;
                }
            }
        }

        // Execute linked operators.
        for (eU32 i=0; i<m_params.size(); i++)
        {
            const eParameter &p = *m_params[i];

            if (p.isAnimated())
            {
                eIOperator *op = eDemoData::findOperator(p.getAnimationPathOpId());

                if (op)
                {
                    op->process(renderer, time, callback, param);
                }
            }
            if (p.getType() == eParameter::TYPE_LINK && i != 2) // Don't process loading-screen operator (2).
            {
                eIOperator *linkedOp = eDemoData::findOperator(p.getValue().linkedOpId);

                if (linkedOp)
                {
                    if (linkedOp->process(renderer, time, callback, param))
                    {
                        changed = eTRUE;
                    }
                }
            }
        }

        _animateParameters(time);
        if (m_changed)
        {
            // Execute this operator.
            eGraphicsApiDx9 *gfx = renderer ? renderer->getGraphicsApi() : eNULL;

            _preExecute(gfx);
            _callExecute(gfx);

            // Set this operator to be unchanged.
            m_changed = eFALSE;

            for (eU32 j=0; j<m_params.size(); j++)
            {
                m_params[j]->setChanged(eFALSE);
            }
        }

        return changed;
    }
OP_END(eDemoOp);

// Material (misc) operator
// ------------------------
// Creates a material which can be mapped onto a mesh.

OP_DEFINE(eMaterialOp, eMaterialOp_ID, eIMaterialOp, "Material", Misc, Misc_CID, eColor(245, 182, 118), 'm', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_INT("Render pass", 0, 255, 0);
        eOP_PARAM_ADD_BOOL("Lighted", eTRUE);
        eOP_PARAM_ADD_BOOL("Flat", eFALSE);
        eOP_PARAM_ADD_BOOL("Two sided", eFALSE);
        eOP_PARAM_ADD_ENUM("Z-Buffer", "On|Readonly (for blending)|Off", 0);
        eOP_PARAM_ADD_LABEL("Blending", "Blending");
        eOP_PARAM_ADD_BOOL("Blending on", eFALSE);
        eOP_PARAM_ADD_ENUM("Blending source", "Zero|One|Source color|1 - Source color|Source alpha|1 - Source alpha|Dest. alpha|1 - Dest. alpha|Dest. color|1 - Dest. color", 1);
        eOP_PARAM_ADD_ENUM("Blending dest.", "Zero|One|Source color|1 - Source color|Source alpha|1 - Source alpha|Dest. alpha|1 - Dest. alpha|Dest. color|1 - Dest. color", 1);
        eOP_PARAM_ADD_ENUM("Blending op.", "Add|Subtract|Reverse subtract|Minimum|Maximum", 0);
        eOP_PARAM_ADD_LABEL("Lighting", "Lighting");
        eOP_PARAM_ADD_RGB("Diffuse", 1.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_RGB("Specular", 1.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_FLOAT("Shininess", 0.0f, 1.0f, 0.25f);
        eOP_PARAM_ADD_RGB("Refraction", 1.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_FLOAT("Refr. intensity", 0.0f, 1.0f, 0.1f);
        eOP_PARAM_ADD_BOOL("Refraction on", eFALSE);
        eOP_PARAM_ADD_LABEL("Diffuse map", "Diffuse map");
        eOP_PARAM_ADD_ENUM("Diff. filtering", "Nearest|Bilinear|Trilinear", 1);
        eOP_PARAM_ADD_ENUM("Diff. addressing", "Wrap|Clamp|Mirror", 0);
        eOP_PARAM_ADD_BOOL("Diff. mipmapping", eTRUE);
        eOP_PARAM_ADD_LINK("Diff. texture", "Bitmap|R2T");
        eOP_PARAM_ADD_LABEL("Bump mapping", "Bump mapping");
        eOP_PARAM_ADD_ENUM("Bump filtering", "Nearest|Bilinear|Trilinear", 1);
        eOP_PARAM_ADD_ENUM("Bump addressing", "Wrap|Clamp|Mirror", 0);
        eOP_PARAM_ADD_BOOL("Bump mipmapping", eTRUE);
        eOP_PARAM_ADD_LINK("Bump texture", "Bitmap|R2T");
        eOP_PARAM_ADD_LABEL("Parallax occlusion mapping", "Parallax occlusion mapping");
        eOP_PARAM_ADD_ENUM("POM filtering", "Nearest|Bilinear|Trilinear", 1);
        eOP_PARAM_ADD_ENUM("POM addressing", "Wrap|Clamp|Mirror", 0);
        eOP_PARAM_ADD_BOOL("POM mipmapping", eTRUE);
        eOP_PARAM_ADD_LINK("POM texture", "Bitmap|R2T");
        eOP_PARAM_ADD_LABEL("Specular map", "Specular map");
        eOP_PARAM_ADD_ENUM("Spec. filtering", "Nearest|Bilinear|Trilinear", 1);
        eOP_PARAM_ADD_ENUM("Spec. addressing", "Wrap|Clamp|Mirror", 0);
        eOP_PARAM_ADD_BOOL("Spec. mipmapping", eTRUE);
        eOP_PARAM_ADD_LINK("Spec. texture", "Bitmap|R2T");
        eOP_PARAM_ADD_LABEL("Environment map", "Environment map");
        eOP_PARAM_ADD_ENUM("Env. filtering", "Nearest|Bilinear|Trilinear", 1);
        eOP_PARAM_ADD_ENUM("Env. addressing", "Wrap|Clamp|Mirror", 0);
        eOP_PARAM_ADD_LINK("Env. texture", "Bitmap|R2T");

        m_texDiffuse    = eNULL;
        m_texNormal     = eNULL;
        m_texDepth      = eNULL;
        m_texSpecular   = eNULL;
        m_texEnv        = eNULL;
        m_ownsDiffuse   = eFALSE;
        m_ownsNormal    = eFALSE;
        m_ownsDepth     = eFALSE;
        m_ownsSpecular  = eFALSE;
        m_ownsEnv       = eFALSE;
    }

    OP_DEINIT()
    {
        _freeTextures();
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eU32 renderPass, eBool lighted, eBool flatShaded, eBool twoSided, eInt zBuffer,
            eBool blending, eInt blendSrc, eInt blendDst, eInt blendOp, const eFloatColor &diffuse, const eFloatColor &specular,
            eF32 shininess, const eFloatColor &refraction, eF32 refracIntensity, eBool useRefraction, eInt diffFilter, eInt diffAddr,
            eBool diffMip, const eIOperator *diffOp, eInt bumpFilter, eInt bumpAddr, eBool bumpMip, const eIOperator *bumpOp,
            eInt pomFilter, eInt pomAddr, eBool pomMip, const eIOperator *pomOp, eInt specFilter, eInt specAddr, eBool specMip,
            const eIOperator *specOp, eInt envFilter, eInt envAddr, const eIOperator *envOp)
    {
        _freeTextures();

#ifdef ePLAYER
        // direct access initialization is MUCH smaller (saves ~50 bytes), since atm the compiler does not inline functions
        m_texDiffuse = _loadTexture(gfx, m_ownsDiffuse, diffMip, diffOp, eFALSE);
        m_texNormal = _loadTexture(gfx, m_ownsNormal, bumpMip, bumpOp, eFALSE);
        m_texDepth = _loadTexture(gfx, m_ownsDepth, pomMip, pomOp, eFALSE);
        m_texSpecular = _loadTexture(gfx, m_ownsSpecular, specMip, specOp, eFALSE);
        m_texEnv = _loadTexture(gfx, m_ownsEnv, eFALSE, envOp, eTRUE);


        m_mat.m_textures[0] = m_texDiffuse;
        m_mat.m_textures[1] = m_texNormal;
        m_mat.m_textures[2] = m_texSpecular;
        m_mat.m_textures[3] = m_texEnv;
        m_mat.m_textures[4] = m_texDepth;
        m_mat.m_texFilters[0] = (eTextureFilter)diffFilter;
        m_mat.m_texFilters[1] = (eTextureFilter)bumpFilter;
        m_mat.m_texFilters[2] = (eTextureFilter)specFilter;
        m_mat.m_texFilters[3] = (eTextureFilter)envFilter;
        m_mat.m_texFilters[4] = (eTextureFilter)pomFilter;
        m_mat.m_texAddrModes[0] = (eTextureAddressMode)diffAddr;
        m_mat.m_texAddrModes[1] = (eTextureAddressMode)bumpAddr;
        m_mat.m_texAddrModes[2] = (eTextureAddressMode)specAddr;
        m_mat.m_texAddrModes[3] = (eTextureAddressMode)envAddr;
        m_mat.m_texAddrModes[4] = (eTextureAddressMode)pomAddr;

        m_mat.m_lighted = lighted;
        m_mat.m_useBlending = blending;
        m_mat.m_flat = flatShaded;
        m_mat.m_blendSrc = (eBlendMode)blendSrc;
        m_mat.m_blendDst = (eBlendMode)blendDst;
        m_mat.m_blendOp = (eBlendOp)blendOp;
        m_mat.m_diffuseCol = diffuse;
        m_mat.m_specularCol = specular;
        m_mat.m_shininess = shininess;
        m_mat.m_refractionCol = refraction;
        m_mat.m_refracIntensity = refracIntensity;
        m_mat.m_useRefraction = useRefraction;
        m_mat.m_cullingMode = twoSided ? eCULLING_NONE : eCULLING_BACK;
        m_mat.m_zBuffer = zBuffer <= 1;
        m_mat.m_zMask = zBuffer != 1;
#else
        m_texDiffuse = _loadTexture(gfx, m_ownsDiffuse, diffMip, diffOp, eFALSE);
        m_mat.setTexture(eMaterial::UNIT_DIFFUSE, m_texDiffuse);
        m_mat.setTextureFilter(eMaterial::UNIT_DIFFUSE, (eTextureFilter)diffFilter);
        m_mat.setTextureAddressMode(eMaterial::UNIT_DIFFUSE, (eTextureAddressMode)diffAddr);

        m_texNormal = _loadTexture(gfx, m_ownsNormal, bumpMip, bumpOp, eFALSE);
        m_mat.setTexture(eMaterial::UNIT_NORMAL, m_texNormal);
        m_mat.setTextureFilter(eMaterial::UNIT_NORMAL, (eTextureFilter)bumpFilter);
        m_mat.setTextureAddressMode(eMaterial::UNIT_NORMAL, (eTextureAddressMode)bumpAddr);

        m_texDepth = _loadTexture(gfx, m_ownsDepth, pomMip, pomOp, eFALSE);
        m_mat.setTexture(eMaterial::UNIT_DEPTH, m_texDepth);
        m_mat.setTextureFilter(eMaterial::UNIT_DEPTH, (eTextureFilter)pomFilter);
        m_mat.setTextureAddressMode(eMaterial::UNIT_DEPTH, (eTextureAddressMode)pomAddr);

        m_texSpecular = _loadTexture(gfx, m_ownsSpecular, specMip, specOp, eFALSE);
        m_mat.setTexture(eMaterial::UNIT_SPECULAR, m_texSpecular);
        m_mat.setTextureFilter(eMaterial::UNIT_SPECULAR, (eTextureFilter)specFilter);
        m_mat.setTextureAddressMode(eMaterial::UNIT_SPECULAR, (eTextureAddressMode)specAddr);

        m_texEnv = _loadTexture(gfx, m_ownsEnv, eFALSE, envOp, eTRUE);
        m_mat.setTexture(eMaterial::UNIT_ENVIRONMENT, m_texEnv);
        m_mat.setTextureFilter(eMaterial::UNIT_ENVIRONMENT, (eTextureFilter)envFilter);
        m_mat.setTextureAddressMode(eMaterial::UNIT_ENVIRONMENT, (eTextureAddressMode)envAddr);

        m_mat.setLighted(lighted);
        m_mat.setUseBlending(blending);
        m_mat.setFlatShaded(flatShaded);
        m_mat.setBlending((eBlendMode)blendSrc, (eBlendMode)blendDst, (eBlendOp)blendOp);
        m_mat.setDiffuseColor(diffuse);
        m_mat.setSpecularColor(specular);
        m_mat.setShininess(shininess);
        m_mat.setRefractionColor(refraction);
        m_mat.setRefractionIntensity(refracIntensity);
        m_mat.setUseRefraction(useRefraction);
        m_mat.setCullingMode(twoSided ? eCULLING_NONE : eCULLING_BACK);
        m_mat.setZBuffer(zBuffer <= 1);
        m_mat.setZMask(zBuffer != 1);
#endif
    }

    eITexture * _loadTexture(eGraphicsApiDx9 *gfx, eBool &owns, eBool mipMapped, const eIOperator *op, eBool cubeMap)
    {
        eASSERT(gfx != eNULL);

        if (op && TEST_CATEGORY(op, "Bitmap", Bitmap_CID))
        {
            const eIBitmapOp::Result &res = ((eIBitmapOp *)op)->getResult();

            owns = eTRUE;

            if (cubeMap)
            {
                eITextureCube *tex = gfx->createTextureCube(eMin(res.width, res.height), eFALSE, mipMapped, op->isAffectedByAnimation(), eFORMAT_ARGB8);
                eASSERT(tex != eNULL);

                for (eInt i=0; i<eCMFACE_COUNT; i++)
                {
                    ePtr data = tex->lock((eCubeMapFace)i);
                    eMemCopy(data, res.bitmap, tex->getWidth()*tex->getWidth()*sizeof(eColor));
                    tex->unlock();
                }

                return tex;
            }
            else
            {
                eITexture2d *tex = gfx->createTexture2d(res.width, res.height, eFALSE, mipMapped, op->isAffectedByAnimation(), eFORMAT_ARGB8);
                eASSERT(tex != eNULL);
                ePtr data = tex->lock();
                eMemCopy(data, res.bitmap, res.size*sizeof(eColor));
                tex->unlock();

                return tex;
            }
        }
#if defined(HAVE_OP_R2T_R2T) || defined(eEDITOR)
        else if (op && op->getType() == "Misc : R2T")
        {
            owns = eFALSE;
            return ((eIRenderToTextureOp *)op)->getResult().renderTarget;
        }
#endif

        return eNULL;
    }

    void _freeTextures()
    {
        if (m_ownsEnv)
        {
            eSAFE_DELETE(m_texEnv);
        }

        if (m_ownsDepth)
        {
            eSAFE_DELETE(m_texDepth);
        }

        if (m_ownsDiffuse)
        {
            eSAFE_DELETE(m_texDiffuse);
        }

        if (m_ownsNormal)
        {
            eSAFE_DELETE(m_texNormal);
        }

        if (m_ownsSpecular)
        {
            eSAFE_DELETE(m_texSpecular);
        }
    }

    OP_VAR(eITexture * m_texDiffuse);
    OP_VAR(eITexture * m_texNormal);
    OP_VAR(eITexture * m_texDepth);
    OP_VAR(eITexture * m_texSpecular);
    OP_VAR(eITexture * m_texEnv);

    OP_VAR(eBool m_ownsDiffuse);
    OP_VAR(eBool m_ownsNormal);
    OP_VAR(eBool m_ownsDepth);
    OP_VAR(eBool m_ownsSpecular);
    OP_VAR(eBool m_ownsEnv);
OP_END(eMaterialOp);

// Scene (misc) operator
// ---------------------
// Operator which holds all data relevant for
// a complete demo (sequencer + sound).

#if defined(HAVE_OP_MISC_SCENE) || defined(eEDITOR)
OP_DEFINE(eSceneOp, eSceneOp_ID, eISceneOp, "Scene", Misc, Misc_CID, eColor(170, 170, 170), 's', 1, 1, "0,Model")
    OP_INIT()
    {
    }

    OP_EXEC(eGraphicsApiDx9 *gfx)
    {
        eSceneData &sd = ((eIModelOp *)getInputOperator(0))->getResult().sceneData;
        m_scene.setSceneData(sd);
    }
OP_END(eSceneOp);
#endif

// Render-to-texture (misc) operator
// ---------------------------------
// Represents a render target which can be rendered to.

#if defined(HAVE_OP_R2T_R2T) || defined(eEDITOR)
OP_DEFINE(eRenderToTextureOp, eRenderToTextureOp_ID, eIRenderToTextureOp, "R2T", Misc, Misc_CID, eColor(170, 170, 170), ' ', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_ENUM("Width", "1|2|4|8|16|32|64|128|256|512|1024|2048", 9);
        eOP_PARAM_ADD_ENUM("Height", "1|2|4|8|16|32|64|128|256|512|1024|2048", 9);
    }

    OP_DEINIT()
    {
        eSAFE_DELETE(m_renderTarget);
        eSAFE_DELETE(m_depthTarget);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt widthSel, eInt heightSel)
    {
        const eU32 sizes[] =
        {
            1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048
        };

        const eU32 width = sizes[widthSel];
        const eU32 height = sizes[heightSel];

        if (m_renderTarget == eNULL || m_renderTarget->getSize() != eSize(width, height))
        {
            eSAFE_DELETE(m_renderTarget);
            eSAFE_DELETE(m_depthTarget);

            m_depthTarget = gfx->createTexture2d(width, height, eTRUE, eFALSE, eFALSE, eFORMAT_DEPTH16);
            m_renderTarget = gfx->createTexture2d(width, height, eTRUE, eFALSE, eFALSE, eFORMAT_ARGB8);
        }
    }
OP_END(eRenderToTextureOp);
#endif

// Load (misc) operator
// --------------------
// Loads a previously saved operator and passes
// its output to load operators output operators.

#if defined(HAVE_OP_MISC_LOAD) || defined(eEDITOR)
OP_DEFINE(eLoadOp, eLoadOp_ID, eIStructureOp, "Load", Misc, Misc_CID, eColor(170, 170, 170), 'l', 0, 0, "")
    OP_INIT()
    {
         eOP_PARAM_ADD_LINK("Load", "All");
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eIOperator *loadedOp)
    {
    }

    virtual const eString & eLoadOp::getUserName() const
    {
        const eIOperator *realOp = eDemoData::findOperator(getParameter(0).getValue().linkedOpId);
        return (realOp ? realOp->getUserName() : eIStructureOp::getUserName());
    }

    virtual eBool eLoadOp::checkValidity() const
    {
        const eIOperator *op = _getRealOp();
        return (op ? op->checkValidity() : eIStructureOp::checkValidity());
    }

    virtual eIOperator * _getRealOp() const
    {
        return eDemoData::findOperator(getParameter(0).getValue().linkedOpId);
    }
OP_END(eLoadOp);
#endif

// Store (misc) operator
// ---------------------
// Stores an operator so it can be loaded
// somewhere else using the load operator.

#if defined(HAVE_OP_MISC_STORE) || defined(eEDITOR)
OP_DEFINE(eStoreOp, eStoreOp_ID, eIStructureOp, "Store", Misc, Misc_CID, eColor(170, 170, 170), 's', 1, 1, "-1,All")
    OP_EXEC(eGraphicsApiDx9 *gfx)
    {
    }
OP_END(eStoreOp);
#endif

// Nop (misc) operator
// -------------------
// Does nothing more than forwarding the input operator.

#if defined(HAVE_OP_MISC_NOP) || defined(eEDITOR)
OP_DEFINE(eNopOp, eNopOp_ID, eIStructureOp, "Nop", Misc, Misc_CID, eColor(170, 170, 170), 'n', 1, 1, "-1,All")
    OP_EXEC(eGraphicsApiDx9 *gfx)
    {
    }
OP_END(eNopOp);
#endif