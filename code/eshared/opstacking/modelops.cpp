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
#include <stdio.h>

// Model (model) operator
// ----------------------
// Converts the input mesh-operator into
// a model-operator.

#if defined(HAVE_OP_MODEL_MODEL) || defined(eEDITOR)
OP_DEFINE_MODEL(eModelModelOp, eModelModelOp_ID, "Model", 'o', 1, 1, "0,Mesh")
    OP_INIT()
    {
        eOP_PARAM_ADD_BOOL("Throws shadow", eTRUE);
        
        m_mi = eNULL;
    }

    OP_DEINIT()
    {
        eSAFE_DELETE(m_mi);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eBool castsShadows)
    {
        const eIMeshOp *meshOp = (eIMeshOp *)getInputOperator(0);
        eASSERT(meshOp != eNULL);

        m_mesh.fromEditMesh(meshOp->getResult().mesh);
        m_mesh.finishLoading(meshOp->isAffectedByAnimation() ? eMesh::TYPE_DYNAMIC : eMesh::TYPE_STATIC);

        eSAFE_DELETE(m_mi);
        m_mi = new eMesh::Instance(m_mesh, castsShadows);
        eASSERT(m_mi != eNULL);

        m_sceneData.addRenderable(m_mi, eMatrix4x4());
    }

    OP_VAR(eMesh m_mesh);
    OP_VAR(eMesh::Instance *m_mi);
OP_END(eModelModelOp);
#endif

// Transform (model) operator
// --------------------------
// Transforms a model-operator.

#if defined(HAVE_OP_MODEL_TRANSFORM) || defined(eEDITOR)
OP_DEFINE_MODEL(eModelTransformOp, eModelTransformOp_ID, "Transform", 't', 1, 1, "-1,Model")
    OP_INIT()
    {
        eOP_PARAM_ADD_FXYZ("Translate", eF32_MIN, eF32_MAX, 0, 0, 0);
        eOP_PARAM_ADD_FXYZ("Rotate", eF32_MIN, eF32_MAX, 0, 0, 0);
        eOP_PARAM_ADD_FXYZ("Scale", eF32_MIN, eF32_MAX, 1, 1, 1);
        eOP_PARAM_ADD_BOOL("Reverse", eFALSE);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eVector3 &trans, const eVector3 &rot, const eVector3 &scale, eBool reverse)
    {
        eSceneData &sd = ((eIModelOp *)getInputOperator(0))->getResult().sceneData;

        m_sceneData.merge(sd);
        m_sceneData.transform(eMatrix4x4(rot*eTWOPI, trans, scale, reverse));
    }
OP_END(eModelTransformOp);
#endif

// Merge (model) operator
// ----------------------
// Merges multiple model operators.

#if defined(HAVE_OP_MODEL_MERGE) || defined(eEDITOR)
OP_DEFINE_MODEL(eModelMergeOp, eModelMergeOp_ID, "Merge", 'm', 1, 64, "-1,Model")
    OP_EXEC(eGraphicsApiDx9 *gfx)
    {
        for (eU32 i=0; i<getInputCount(); i++)
        {
            eSceneData &sd = ((eIModelOp *)getInputOperator(i))->getResult().sceneData;
            m_sceneData.merge(sd);
        }
    }
OP_END(eModelMergeOp);
#endif

// Multiply (model) operator
// -------------------------
// Duplicates and randomizes scene-graph instances.

#if defined(HAVE_OP_MODEL_MULTIPLY) || defined(eEDITOR)
OP_DEFINE_MODEL(eModelMultiplyOp, eModelMultiplyOp_ID, "Multiply", 'u', 1, 1, "-1,Model")
    OP_INIT()
    {
        eOP_PARAM_ADD_FXYZ("Translate", eF32_MIN, eF32_MAX, 0, 0, 0);
        eOP_PARAM_ADD_FXYZ("Rotate", eF32_MIN, eF32_MAX, 0, 0, 0);
        eOP_PARAM_ADD_FXYZ("Scale", eF32_MIN, eF32_MAX, 1, 1, 1);
        eOP_PARAM_ADD_INT("Count", 1, 255, 2);
        eOP_PARAM_ADD_LABEL("Randomize", "Randomize");
        eOP_PARAM_ADD_FXYZ("Rand. trans.", eF32_MIN, eF32_MAX, 0, 0, 0);
        eOP_PARAM_ADD_FXYZ("Rand. rotation", eF32_MIN, eF32_MAX, 0, 0, 0);
        eOP_PARAM_ADD_FXYZ("Rand. scale", eF32_MIN, eF32_MAX, 0, 0, 0);
        eOP_PARAM_ADD_INT("Seed", 0, 65535, 0);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eVector3 &trans, const eVector3 &rot, const eVector3 &scale,
            eU32 count, const eVector3 &randTrans, const eVector3 &randRot, const eVector3 &randScale, eU32 seed)
    {
        eSceneData &sd = ((eIModelOp *)getInputOperator(0))->getResult().sceneData;

        eMatrix4x4 tf;
        tf.transformation(rot*eTWOPI, trans, scale);
        eMatrix4x4 mtx;

		seed++;

        for (eU32 i=0; i<count+1; i++)
        {
			const eVector3 vec_s = eVector3(1.0f)+randScale.random(seed);
			const eVector3 vec_t = randTrans.random(seed);
			const eVector3 vec_r = (randRot*eTWOPI).random(seed);

            const eMatrix4x4 randMtx(vec_r, vec_t, vec_s, eTRUE);

            m_sceneData.merge(sd, randMtx*mtx);
            mtx *= tf;
        }
    }
OP_END(eModelMultiplyOp);
#endif

// Center (model) operator
// -----------------------
// Centers all renderables of scene graph around
// the origin of the coordinate system (0, 0, 0).

#if defined(HAVE_OP_MODEL_CENTER) || defined(eEDITOR)
OP_DEFINE_MODEL(eModelCenterOp, eModelCenterOp_ID, "Center", 'e', 1, 1, "-1,Model")
    OP_EXEC(eGraphicsApiDx9 *gfx)
    {
        eSceneData &sd = ((eIModelOp *)getInputOperator(0))->getResult().sceneData;

        eMatrix4x4 mtx;
        mtx.translate(-sd.getBoundingBox().getCenter());

        m_sceneData.merge(sd, mtx);
    }
OP_END(eModelCenterOp);
#endif

// Light (model) operator
// ----------------------
// Adds a light to the scene graph.

#if defined(HAVE_OP_MODEL_LIGHT) || defined(eEDITOR)
OP_DEFINE_MODEL(eLightOp, eLightOp_ID, "Light", 'l', 0, 1, "-1,Model")
    OP_INIT()
    {
        eOP_PARAM_ADD_FXYZ("Position", eF32_MIN, eF32_MAX, 10.0f, 10.0f, 10.0f);
        eOP_PARAM_ADD_FLOAT("Range", 0.01f, eF32_MAX, 100.0f);
        eOP_PARAM_ADD_RGB("Diffuse", 1.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_RGB("Ambient", 0.0f, 0.0f, 0.0f);
        eOP_PARAM_ADD_RGB("Specular", 1.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_LABEL("Shadows", "Shadows");
        eOP_PARAM_ADD_BOOL("Casts shadows", eFALSE);
        eOP_PARAM_ADD_FLAGS("Shadowed faces", "X+|X-|Y+|Y-|Z+|Z-", 0x3f); // All on = first 6 bits set.
        eOP_PARAM_ADD_FLOAT("Penumbra size", 0.0f, eF32_MAX, 2.0f);
        eOP_PARAM_ADD_FLOAT("Shadow bias", 0.0f, 1.0f, 0.001f);

        // Create light bounding sphere mesh.
        _addCircleToMesh(m_lightMesh, 100, eVector3(   0.0f,    0.0f, eHALFPI));
        _addCircleToMesh(m_lightMesh, 100, eVector3(   0.0f, eHALFPI,    0.0f));
        _addCircleToMesh(m_lightMesh, 100, eVector3(eHALFPI,    0.0f,    0.0f));

        m_lightMesh.finishLoading(eMesh::TYPE_DYNAMIC);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eVector3 &pos, eF32 range, const eFloatColor &diffuse, const eFloatColor &ambient,
            const eFloatColor &specular, eBool castsShadows, eU8 shadowedFaces, eF32 penumbraSize, eF32 shadowBias)
    {
        if (getInputCount() > 0)
        {
            eSceneData &sd = ((eIModelOp *)getInputOperator(0))->getResult().sceneData;
            m_sceneData.merge(sd);
        }

        m_light.setDiffuse(diffuse);
        m_light.setAmbient(ambient);
        m_light.setSpecular(specular);
        m_light.setPosition(pos);
        m_light.setRange(range);
        m_light.setPenumbraSize(penumbraSize);
        m_light.setShadowBias(shadowBias*0.1f); // *0.1, because available precision in editor is too small.

        for (eInt i=0; i<eCMFACE_COUNT; i++)
        {
            const eBool enable = (eGetBit(shadowedFaces, i) && castsShadows);
            m_light.setCastsShadows((eCubeMapFace)i, enable);
        }

        m_sceneData.addLight(&m_light);
    }

    OP_INTERACT(eGraphicsApiDx9 *gfx, eSceneData &sd)
    {
        static eMesh::Instance mi(m_lightMesh);

        eMatrix4x4 mtx;

        mtx.scale(eVector3(getParameter(1).getValue().flt)*0.5f);
        mtx.translate(getParameter(0).getValue().fxyz);

        sd.addRenderable(&mi, mtx);
    }
    
    void _addCircleToMesh(eMesh &mesh, eU32 segCount, const eVector3 &rot)
    {
        eASSERT(segCount >= 3);

        const eF32 step = eTWOPI/(eF32)segCount;

        eVector3 pos;
        eF32 angle = 0.0f;

        const eU32 oldVtxCount = mesh.getVertexCount();

        for (eU32 i=0; i<segCount; i++)
        {
            eSinCos(angle, pos.x, pos.y);
            pos.z = 0.0f;
            pos.rotate(rot);

            angle += step;
            mesh.addVertex(pos, eColor::ORANGE);
        }

        for (eU32 i=0; i<segCount; i++)
        {
            mesh.addLine(oldVtxCount+i, oldVtxCount+(i+1)%segCount, eMaterial::getWireframe());
        }

        mesh.addLine(oldVtxCount, oldVtxCount+segCount/2, eMaterial::getWireframe());
    }

    OP_VAR(eLight m_light);
    OP_VAR(eMesh m_lightMesh);
OP_END(eLightOp);
#endif

// Duplicate circular (model) operator
// -----------------------------------
// Duplicates renderables in a circular arrangement.

#if defined(HAVE_OP_MODEL_DUP__CIRCULAR) || defined(eEDITOR)
OP_DEFINE_MODEL(eModelDuplicateCircularOp, eModelDuplicateCircularOp_ID, "Dup. circular", 'c', 1, 1, "-1,Model")
    OP_INIT()
    {
        eOP_PARAM_ADD_FLOAT("Radius", eF32_MIN, eF32_MAX, 5.0f);
        eOP_PARAM_ADD_INT("Count", 1, 255, 5);
        eOP_PARAM_ADD_FXYZ("Rotation", eF32_MIN, eF32_MAX, 0, 0, 0);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 radius, eU32 count, const eVector3 &rot)
    {
        eSceneData &sd = ((eIModelOp *)getInputOperator(0))->getResult().sceneData;

        const eVector3 exRot = rot*eTWOPI;
        const eVector3 trans(0.0f, 0.0f, radius);

        eVector3 curRot = exRot;
        eF32 step = eTWOPI/(count+1);

        for (eU32 i=0; i<=count; i++)
        {
            curRot += exRot;
            curRot.y += step;

            m_sceneData.merge(sd, eMatrix4x4(curRot, trans, eVector3(1.0f), eTRUE));
        }
    }
OP_END(eModelDuplicateCircularOp);
#endif

// Particle system (model) operator
// --------------------------------
// Adds a particle system to model's scene graph.

#if defined(HAVE_OP_MODEL_PARTICLES) || defined(eEDITOR)
OP_DEFINE_MODEL(eParticleSystemOp, eParticleSystemOp_ID, "Particles", 'a', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_LABEL("Emitter", "Emitter");
        eOP_PARAM_ADD_ENUM("Emitter Mode", "Faces|Edges|Vertices|FluidForce", 0);
        eOP_PARAM_ADD_FLOAT("Emission frequency [1/s]", 0.01f, 100000.0f, 100.0f);
        eOP_PARAM_ADD_LINK("Emitter mesh", "Mesh");
        eOP_PARAM_ADD_LINK("Force (FluidOp)", "Mesh");

        eOP_PARAM_ADD_LABEL("Life of particle", "Life of particle");
        eOP_PARAM_ADD_FLOAT("Randomization", 0.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_FLOAT("Life time", 0.01f, 100.0f, 10.0f);
        eOP_PARAM_ADD_FLOAT("Emission velocity", 0.0f, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_LINK("Size", "Path");
        eOP_PARAM_ADD_LINK("Color", "Path");
        eOP_PARAM_ADD_LINK("Rotation", "Path");
        eOP_PARAM_ADD_FLOAT("Particle stretch", 0.0f, eF32_MAX, 0.0f);
        eOP_PARAM_ADD_FLOAT("Gravity", -eF32_MAX, eF32_MAX, 0.0f);

        eOP_PARAM_ADD_LABEL("Blending", "Blending");
        eOP_PARAM_ADD_LINK("Texture", "Bitmap");
        eOP_PARAM_ADD_ENUM("Blending source", "Zero|One|Source color|1 - Source color|Source alpha|1 - Source alpha|Dest. alpha|1 - Dest. alpha|Dest. color|1 - Dest. color", 4);
        eOP_PARAM_ADD_ENUM("Blending dest.", "Zero|One|Source color|1 - Source color|Source alpha|1 - Source alpha|Dest. alpha|1 - Dest. alpha|Dest. color|1 - Dest. color", 5);
        eOP_PARAM_ADD_ENUM("Blending op.", "Add|Subtract|Reverse subtract|Minimum|Maximum", 0);

        m_tex = eNULL;
        m_psysInst = new eParticleSystem::Instance(m_psys);
        eASSERT(m_psysInst != eNULL);
    }

    OP_DEINIT()
    {
        eSAFE_DELETE(m_psysInst);
        eSAFE_DELETE(m_tex);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt emitterMode, eF32 emissionFreq, const eIMeshOp *emitterOp, const eIMeshOp *forceOp,
            eF32 randomization, eF32 lifeTime, eF32 emissionVel, const eIPathOp *sizeOp, const eIPathOp *colorOp,
            const eIPathOp *rotOp, eF32 stretch, eF32 gravity, const eIBitmapOp *bmpOp, eInt blendSrc, eInt blendDst, eInt blendOp)
    {
	    m_psys.init(gfx);

	    m_psys.m_stretchAmount = stretch;
	    m_psys.m_randomization = randomization;
	    m_psys.m_emissionFreq = emissionFreq;
	    m_psys.m_emissionVel = emissionVel;
	    m_psys.m_lifeTime = lifeTime;
	    m_psys.m_blendSrc = (eBlendMode)blendSrc;
	    m_psys.m_blendDst = (eBlendMode)blendDst;
	    m_psys.m_blendOp = (eBlendOp)blendOp;
	    m_psys.m_gravity = gravity;
        m_psys.m_gravityConst->set(0.0f, -gravity, 0.0f);

        const eIPathOp *pathOps[3] = { sizeOp, colorOp, rotOp };

	    for (eU32 i=0; i<3; i++)
        {
		    if (getParameter(10+i).getChanged())
            {
			    if (pathOps[i])
                {
				    if (m_psys.m_pathSampler[i] == eNULL)
                    {
					    m_psys.m_pathSampler[i] = new ePathSampler;
					    eASSERT(m_psys.m_pathSampler[i] != eNULL);
				    }

				    m_psys.m_pathSampler[i]->sample(pathOps[i]->getResult().path);
			    }
                else 
                {
				    eSAFE_DELETE(m_psys.m_pathSampler[i]);
                }
		    }
	    }

        if (getParameter(4).getChanged() || getParameter(2).getChanged())
        {
#if defined(PSYS_USE_EMITTERMODES)
            m_psys->setEmitter(!emitterOp ? eNULL : &emitterOp->getResult().mesh, (eParticleSystem::EmitterMode)emitterMode);
#else
            m_psys.setEmitter(!emitterOp ? eNULL : &emitterOp->getResult().mesh);
#endif
        }

#if defined(PSYS_USE_LIQUID_INFLUENCE) && (defined(HAVE_OP_MESH_LIQUID) || defined(eEDITOR)) 
        if (getParameter(5).getChanged())
        {
            const eLiquidOp *liquidOp = (eLiquidOp *)eDemoData::findOperator(param.getValue().linkedOpId);
            m_psys->m_fluids = !liquidOp ? eNULL : liquidOp->getStableFluids();
        }
#endif

        if (getParameter(16).getChanged())
        {
            eSAFE_DELETE(m_tex);
            m_psys.m_tex = eNULL;

            if (bmpOp)
            {
                const eIBitmapOp::Result &res = bmpOp->getResult();

                m_tex = gfx->createTexture2d(res.width, res.height, eFALSE, eTRUE, bmpOp->isAffectedByAnimation(), eFORMAT_ARGB8);
                ePtr data = m_tex->lock();
                eMemCopy(data, (eColor *)res.bitmap, res.size*sizeof(eColor));
                m_tex->unlock();

                m_psys.m_tex = m_tex;
            }
        }

        m_sceneData.addRenderable(m_psysInst);
    }

    OP_VAR(eITexture2d *m_tex);
    OP_VAR(eParticleSystem m_psys);
    OP_VAR(eParticleSystem::Instance *m_psysInst);
OP_END(eParticleSystemOp);
#endif

