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

#include "../system/system.hpp"
#include "../math/math.hpp"
#include "engine.hpp"

//#define VISUALIZE_RENDER_PASSES

#define BLA(x) x

eDeferredRenderer::eDeferredRenderer(eGraphicsApiDx9 *gfx, eU32 shadowSize, eU32 refractionSize) : eIRenderer(gfx),
    m_diffuseRt(eNULL),
    m_normalsRt(eNULL),
    m_specularRt(eNULL),
    m_positionRt(eNULL),
    m_deferredShadowMap(eNULL),
    m_unwrappedDistMap(eNULL),
    m_indirectionCm(eNULL),
    m_shadowSize(shadowSize),
    m_refractionTex(eNULL),
    m_refractionSize(refractionSize)
{
    eASSERT(gfx != eNULL);

    _createIndirectionCubeMap();
    _loadShaders();

    // Create default material for meshes.
    eMaterial defaultMat;

    defaultMat.setFlatShaded(eTRUE);
    eMaterial::setDefault(defaultMat);
}

eDeferredRenderer::~eDeferredRenderer()
{
    eSAFE_DELETE(m_indirectionCm);

    _freeTargets();
}

void eDeferredRenderer::renderScene(eScene &scene, const eCamera &cam, eITexture2d *target, eF32 time)
{
    ePROFILER_ZONE("Render scene");

    eASSERT(target != eNULL);
    eASSERT(target->isRenderTarget() == eTRUE);
    eASSERT(time >= 0.0f);

    eRenderJob::reset(); // TODO: relocate me if necessary

    eStateManager::push();

    // Begin rendering the scene.
    scene.update(time);
    scene.collectRenderJobs(cam, m_allJobs);

    // Setup all render-targets and fill g-buffer.
    _setupTargets(target->getWidth(), target->getHeight());
    _assignMaterialIds();
    _renderGeometryPass(cam);

    // Render the final image.
    eStateManager::bindRenderTarget(0, target);
    eStateManager::setCullingMode(eCULLING_NONE);
    eStateManager::setCap(eCAP_ZBUFFER, eFALSE);
    eStateManager::setCap(eCAP_BLENDING, eFALSE);
    eStateManager::setBlendModes(eBLEND_ONE, eBLEND_ONE, eBLENDOP_ADD);
    eStateManager::setTextureAddressMode(0, eTEXADDRMODE_CLAMP);
    eStateManager::setTextureAddressMode(1, eTEXADDRMODE_CLAMP);
    eStateManager::setTextureAddressMode(2, eTEXADDRMODE_CLAMP);
    eStateManager::setTextureAddressMode(3, eTEXADDRMODE_CLAMP);

    const eRect area(0, 0, target->getWidth(), target->getHeight());

    // Do deferred (no-) lighting passes.
    _renderAmbientPass(scene, area);
    _renderLightPass(scene, cam, area);
    _renderNoLightPass(cam);

    // Do alpha lighting passes. 
    _renderAlphaLightPass(scene, cam, area);

    // Do effect passes.
    _renderEnvPass(area);
    _renderRefractionPass(cam, area, target);
   
    eStateManager::pop();

#ifdef VISUALIZE_RENDER_PASSES
    eStateManager::bindRenderTarget(0, target);
    _visualizeGeometryBuffer(area);
#endif
}

void eDeferredRenderer::renderTexturedQuad(const eRect &r, const eSize &targetSize, eITexture2d *tex, const eVector2 &tileUv, const eVector2 &scrollUv) const
{
    eStateManager::push();

    if (tex)
    {
        eStateManager::bindTexture(0, tex);
    }

    eStateManager::bindVertexShader(m_vsQuad);
    eStateManager::bindPixelShader(m_psQuad);
    eStateManager::setCullingMode(eCULLING_NONE);
    eStateManager::setCap(eCAP_ZBUFFER, eFALSE);

    renderQuad(r, targetSize, tileUv, scrollUv);

    eStateManager::pop();
}

void eDeferredRenderer::renderTexturedQuad(const eRect &r, const eSize &size, eITexture2d *tex) const
{
    renderTexturedQuad(r, size, tex, eVector2(1.0f, 1.0f), eVector2(0.0f, 0.0f));
}

void eDeferredRenderer::renderQuad(const eRect &r, const eSize &size, const eVector2 &tileUv, const eVector2 &scrollUv) const
{
    const eCamera cam(0.0f, (eF32)size.width, 0.0f, (eF32)size.height, -1.0f, 1.0f);
    cam.activate(m_gfx);

    eGeometry geo(4, 0, 2, eVTXTYPE_DEFAULT, eGeometry::TYPE_DYNAMIC, ePRIMTYPE_TRIANGLESTRIPS);
    eVertex *vertices = eNULL;

    geo.startFilling((ePtr *)&vertices, eNULL);
    {
        // +0.5 and -0.5 are required for offsetting screen-space
        // coordinates of quad a bit in order to compensate
        // for correct texel to pixel mapping in Direct3D9.
        vertices[0].set(eVector3((eF32)r.left-0.5f,  (eF32)r.bottom+0.5f, 0.0f), eVector3(), eVector2(scrollUv.u,          scrollUv.v),          eColor::WHITE);
        vertices[1].set(eVector3((eF32)r.left-0.5f,  (eF32)r.top+0.5f,    0.0f), eVector3(), eVector2(scrollUv.u,          tileUv.v+scrollUv.v), eColor::WHITE);
        vertices[2].set(eVector3((eF32)r.right-0.5f, (eF32)r.bottom+0.5f, 0.0f), eVector3(), eVector2(tileUv.u+scrollUv.u, scrollUv.v),          eColor::WHITE);
        vertices[3].set(eVector3((eF32)r.right-0.5f, (eF32)r.top+0.5f,    0.0f), eVector3(), eVector2(tileUv.u+scrollUv.u, tileUv.v+scrollUv.v), eColor::WHITE);
    }
    geo.stopFilling();
    geo.render();
}

void eDeferredRenderer::renderQuad(const eRect &r, const eSize &size) const
{
    renderQuad(r, size, eVector2(1.0f, 1.0f), eVector2(0.0f, 0.0f));
}

eITexture2d * eDeferredRenderer::getPositionMap() const
{
    return m_positionRt;
}

eITexture2d * eDeferredRenderer::getNormalMap() const
{
    return m_normalsRt;
}

void eDeferredRenderer::_setupTargets(eU32 width, eU32 height)
{
    eASSERT(width > 0);
    eASSERT(height > 0);

    // Render targets are up-to-date => don't reacreate them.
    if (m_diffuseRt && m_diffuseRt->getWidth() == width && m_diffuseRt->getHeight() == height)
    {
        return;
    }

    // Recreate render-targets (first free them).
    _freeTargets();

    m_diffuseRt         = m_gfx->createTexture2d(width, height, eTRUE, eFALSE, eFALSE, eFORMAT_ARGB8);
    m_normalsRt         = m_gfx->createTexture2d(width, height, eTRUE, eFALSE, eFALSE, eFORMAT_ARGB16F);
    m_specularRt        = m_gfx->createTexture2d(width, height, eTRUE, eFALSE, eFALSE, eFORMAT_ARGB16F);
    m_positionRt        = m_gfx->createTexture2d(width, height, eTRUE, eFALSE, eFALSE, eFORMAT_ARGB16F);

    m_deferredShadowMap = m_gfx->createTexture2d(width, height, eTRUE, eFALSE, eFALSE, eFORMAT_R16F);
    m_unwrappedDistMap  = m_gfx->createTexture2d(m_shadowSize*eCMFACE_COUNT, m_shadowSize, eTRUE, eFALSE, eFALSE, eFORMAT_DEPTH16);

    m_refractionTex     = m_gfx->createTexture2d(m_refractionSize, m_refractionSize, eTRUE, eFALSE, eFALSE, eFORMAT_ARGB8);
}

void eDeferredRenderer::_freeTargets()
{
    eSAFE_DELETE(m_diffuseRt);
    eSAFE_DELETE(m_normalsRt);
    eSAFE_DELETE(m_specularRt);
    eSAFE_DELETE(m_positionRt);

    eSAFE_DELETE(m_deferredShadowMap);
    eSAFE_DELETE(m_unwrappedDistMap);

    eSAFE_DELETE(m_refractionTex);
}

void eDeferredRenderer::_loadShaders()
{
    m_psRefraction  = eShaderManager::loadPixelShader(ePS(refraction));
    m_psNoLight     = eShaderManager::loadPixelShader(ePS(nolight));
    m_psForwLight   = eShaderManager::loadPixelShader(ePS(forward_light));
    m_psDefAmbient  = eShaderManager::loadPixelShader(ePS(deferred_ambient));
    m_psDefEnv      = eShaderManager::loadPixelShader(ePS(deferred_env));
    m_psDefGeo      = eShaderManager::loadPixelShader(ePS(deferred_geo));
    m_psDefLight    = eShaderManager::loadPixelShader(ePS(deferred_light));
    m_psDistance    = eShaderManager::loadPixelShader(ePS(distance));
    m_psShadow      = eShaderManager::loadPixelShader(ePS(shadow));
    m_psQuad        = eShaderManager::loadPixelShader(ePS(quad));
    m_psParticles   = eShaderManager::loadPixelShader(ePS(particles));

    m_vsNoLight     = eShaderManager::loadVertexShader(eVS(nolight));
    m_vsInstGeo     = eShaderManager::loadVertexShader(eVS(instanced_geo));
    m_vsDistance    = eShaderManager::loadVertexShader(eVS(distance));
    m_vsShadow      = eShaderManager::loadVertexShader(eVS(shadow));
    m_vsQuad        = eShaderManager::loadVertexShader(eVS(quad));
    m_vsParticles   = eShaderManager::loadVertexShader(eVS(particles));
}

void eDeferredRenderer::_createIndirectionCubeMap()
{
    const eU32 size = m_shadowSize*2;

    m_indirectionCm = m_gfx->createTextureCube(size, eFALSE, eFALSE, eFALSE, eFORMAT_GR32F);

    for (eInt i=0; i<eCMFACE_COUNT; i++)
    {
        eVector2 *data = (eVector2 *)m_indirectionCm->lock((eCubeMapFace)i);
        eASSERT(data != eNULL);

        for (eU32 y=0, index=0; y<size; y++)
        {
            const eF32 v = (eF32)y/(eF32)(size-1);

            for (eU32 x=0; x<size; x++)
            {
                const eF32 u = (eF32)(x+i*size)/(eF32)(size*eCMFACE_COUNT-1);                
                data[index++].set(u, v);
            }
        }

        m_indirectionCm->unlock();
    }
}

void eDeferredRenderer::_renderGeometryPass(const eCamera &cam)
{
    eStateManager::push();
    eStateManager::bindRenderTarget(0, m_diffuseRt);
    eStateManager::bindRenderTarget(1, m_normalsRt);
    eStateManager::bindRenderTarget(2, m_specularRt);
    eStateManager::bindRenderTarget(3, m_positionRt);
    eStateManager::bindVertexShader(m_vsInstGeo);
    eStateManager::bindPixelShader(m_psDefGeo);
    eStateManager::apply();

    m_gfx->clear(eCLEAR_DEPTHCOLOR, eColor::BLACK);
    _renderRenderJobs(m_allJobs, cam, eRenderJob::RENDER_ALL & ~eRenderJob::ALPHA_ON & ~eRenderJob::REFRACTED_ON);

    eStateManager::pop();
}

void eDeferredRenderer::_renderAmbientPass(const eScene &scene, const eRect &area)
{
    // Calculate the ambient color.
    eColor ambient;

    for (eU32 i=0; i<scene.getLightCount(); i++)
    {
        ambient += scene.getLight(i).getAmbient();
    }

    m_gfx->setPsConst(ePSCONST_LIGHT_TOTALAMBIENT, ambient);

    // Render the ambient light.
    eStateManager::push();
    eStateManager::bindTexture(0, m_diffuseRt);
    eStateManager::bindVertexShader(m_vsQuad);
    eStateManager::bindPixelShader(m_psDefAmbient);
    
    renderQuad(area, area.getDimension());
    
    eStateManager::pop();
}

void eDeferredRenderer::_renderAlphaLightPass(const eScene &scene, const eCamera &cam, const eRect &area)
{
    eStateManager::push();
    eStateManager::bindVertexShader(m_vsInstGeo);
    eStateManager::bindPixelShader(m_psForwLight);

    for (eU32 i=0; i<scene.getLightCount(); i++)
    {
        const eLight &light = scene.getLight(i);

        if (light.activateScissor(area.getDimension(), cam))
        {
            light.activate(m_gfx, cam.getViewMatrix());
            _renderRenderJobs(m_allJobs, cam, eRenderJob::ALPHA_ON | eRenderJob::LIGHTED_ON | eRenderJob::CASTSHADOW_BOTH | eRenderJob::REFRACTED_OFF);
        }
    }

    eStateManager::pop();
}

void eDeferredRenderer::_renderNoLightPass(const eCamera &cam)
{
    eStateManager::push();
    eStateManager::bindVertexShader(m_vsInstGeo);
    eStateManager::bindPixelShader(m_psNoLight);

    _renderRenderJobs(m_allJobs, cam, eRenderJob::ALPHA_BOTH | eRenderJob::LIGHTED_OFF | eRenderJob::CASTSHADOW_BOTH | eRenderJob::REFRACTED_OFF);

    eStateManager::pop();
}

void eDeferredRenderer::_renderLightPass(eScene &scene, const eCamera &cam, const eRect &area)
{
    eStateManager::push();
    eStateManager::bindTexture(0, m_diffuseRt);
    eStateManager::bindTexture(1, m_normalsRt);
    eStateManager::bindTexture(2, m_specularRt);
    eStateManager::bindTexture(3, m_positionRt);
    eStateManager::bindTexture(4, m_deferredShadowMap);
    eStateManager::setTextureFilter(0, eTEXFILTER_NEAREST);
    eStateManager::setTextureFilter(1, eTEXFILTER_NEAREST);
    eStateManager::setTextureFilter(2, eTEXFILTER_NEAREST);
    eStateManager::setTextureFilter(3, eTEXFILTER_NEAREST);
    eStateManager::setTextureFilter(4, eTEXFILTER_BILINEAR);
    eStateManager::setTextureAddressMode(4, eTEXADDRMODE_CLAMP);
    eStateManager::setCap(eCAP_BLENDING, eTRUE);
    eStateManager::setBlendModes(eBLEND_ONE, eBLEND_ONE, eBLENDOP_ADD);
    eStateManager::bindPixelShader(m_psDefLight);
    eStateManager::bindVertexShader(m_vsQuad);

    for (eU32 i=0; i<scene.getLightCount(); i++)
    {
        const eLight &light = scene.getLight(i);

        if (light.activateScissor(area.getDimension(), cam))
        {
            _renderLightDistance(scene, light);
            _renderShadowMap(cam, light);

            light.activate(m_gfx, cam.getViewMatrix());
            renderQuad(area, area.getDimension());
        }
    }

    eStateManager::pop();
}

void eDeferredRenderer::_renderLightDistance(eScene &scene, const eLight &light)
{
    const eVector3 &lightPos = light.getPosition();

    eStateManager::push();
    eStateManager::setCap(eCAP_ZBUFFER, eTRUE);
    eStateManager::setCap(eCAP_BLENDING, eFALSE);
    eStateManager::setCap(eCAP_ZWRITE, eTRUE);
    eStateManager::setCap(eCAP_COLORWRITE, eFALSE);
    eStateManager::bindVertexShader(m_vsDistance);
    eStateManager::bindPixelShader(m_psDistance);
    eStateManager::bindRenderTarget(0, eNULL);
    eStateManager::bindDepthTarget(m_unwrappedDistMap);
    eStateManager::apply();

    m_gfx->clear(eCLEAR_DEPTHCOLOR, eColor::WHITE);

    eCamera cam(90.0f, 1.0f, 0.1f, light.getRange());
    eRenderJobPtrArray jobs;

    for (eU32 i=0; i<eCMFACE_COUNT; i++)
    {
        if (light.getCastsShadows((eCubeMapFace)i))
        {
            eMatrix4x4 cubeMtx, viewMtx;

            cubeMtx.cubemap(i);
            viewMtx.translate(-lightPos);
            viewMtx *= cubeMtx;

            cam.setViewMatrix(viewMtx);
            light.activate(m_gfx, viewMtx);
            eStateManager::setViewport(i*m_shadowSize, 0, m_shadowSize, m_shadowSize);

            scene.collectRenderJobs(cam, jobs);
            _renderRenderJobs(jobs, cam, eRenderJob::RENDER_ALL & ~eRenderJob::CASTSHADOW_OFF & ~eRenderJob::ALPHA_ON, eRenderJob::MATERIALS_OFF);
        }
    }

    eStateManager::pop();
}

void eDeferredRenderer::_renderShadowMap(const eCamera &cam, const eLight &light)
{
    eStateManager::push();
    eStateManager::bindVertexShader(m_vsShadow);
    eStateManager::bindPixelShader(m_psShadow);
    eStateManager::setCap(eCAP_ZBUFFER, eTRUE);
    eStateManager::setCap(eCAP_BLENDING, eFALSE);
    eStateManager::bindDepthTarget(eGraphicsApiDx9::TARGET_SCREEN);
    eStateManager::bindRenderTarget(0, m_deferredShadowMap);
    eStateManager::bindTexture(0, m_indirectionCm);
    eStateManager::bindTexture(1, m_unwrappedDistMap);
    eStateManager::bindTexture(2, eNULL);
    eStateManager::bindTexture(3, eNULL);
    eStateManager::bindTexture(4, eNULL);
    eStateManager::setTextureFilter(0, eTEXFILTER_NEAREST);
    eStateManager::setTextureFilter(1, eTEXFILTER_BILINEAR);
    eStateManager::setTextureAddressMode(0, eTEXADDRMODE_CLAMP);
    eStateManager::setTextureAddressMode(1, eTEXADDRMODE_CLAMP);
    eStateManager::setTextureAddressMode(2, eTEXADDRMODE_WRAP);
    eStateManager::apply();

    m_gfx->clear(eCLEAR_COLORBUFFER, eColor::BLACK);

    if (light.getCastsAnyShadows())
    {
        const eF32 nearZ = 0.1f;
        const eF32 farZ = light.getRange();

        m_gfx->setPsConst(ePSCONST_SHADOW_MAP_SIZE, eVector2((eF32)m_unwrappedDistMap->getWidth(), (eF32)m_unwrappedDistMap->getHeight()));
        m_gfx->setPsConst(ePSCONST_SHADOW_PROJZ, eVector2(nearZ*farZ/(farZ-nearZ), farZ/(farZ-nearZ)));

        light.activate(m_gfx, cam.getViewMatrix());
        _renderRenderJobs(m_allJobs, cam, eRenderJob::RENDER_ALL & ~eRenderJob::CASTSHADOW_OFF & ~eRenderJob::ALPHA_ON, eRenderJob::MATERIALS_OFF);
    }

    eStateManager::pop();
}

void eDeferredRenderer::_renderRefractionPass(const eCamera &cam, const eRect &area, eITexture2d *target)
{
    eStateManager::push();

    // Copy scene to texture.
    eStateManager::bindRenderTarget(0, m_refractionTex);
    renderTexturedQuad(area, target->getSize(), target);
    eStateManager::bindRenderTarget(0, target);

    // Render refractive objects.
    eStateManager::bindTexture(7, m_refractionTex);
    eStateManager::setTextureAddressMode(7, eTEXADDRMODE_MIRROR);
    eStateManager::bindVertexShader(m_vsInstGeo);
    eStateManager::bindPixelShader(m_psRefraction);

    _renderRenderJobs(m_allJobs, cam, eRenderJob::RENDER_ALL & ~eRenderJob::REFRACTED_OFF);

    eStateManager::pop();
}

void eDeferredRenderer::_renderEnvPass(const eRect &area)
{
    eStateManager::push();
    eStateManager::bindTexture(1, m_normalsRt);
    eStateManager::bindTexture(2, m_specularRt);
    eStateManager::bindTexture(3, m_positionRt);
    eStateManager::setTextureFilter(0, eTEXFILTER_BILINEAR);
    eStateManager::setTextureFilter(1, eTEXFILTER_NEAREST);
    eStateManager::setTextureFilter(3, eTEXFILTER_NEAREST);
    eStateManager::setCap(eCAP_BLENDING, eTRUE);
    eStateManager::bindVertexShader(m_vsQuad);
    eStateManager::bindPixelShader(m_psDefEnv);

    for (eU32 i=0; i<m_materialArray.size(); i++)
    {
        const eMaterial* mat = m_materialArray[i];

        eStateManager::bindTexture(0, mat->getTexture(eMaterial::UNIT_ENVIRONMENT));
        m_gfx->setPsConst(ePSCONST_MAT_INDEX, (eF32)mat->m_render_material_id/256.0f);
        renderQuad(area, area.getDimension());
    }

    eStateManager::pop();
}

void eDeferredRenderer::_renderRenderJobs(const eRenderJobPtrArray &jobs, const eCamera &cam, eInt renderWhat, eInt renderFlags)
{
    eStateManager::push();

    for (eU32 i=0; i<jobs.size(); i++)
    {
        const eRenderJob *job = jobs[i];
        eASSERT(job != eNULL);

        if (job->getType() == eIRenderable::TYPE_PARTICLE_SYSTEM)
        {
            eStateManager::push();
            eStateManager::bindVertexShader(m_vsParticles);
            eStateManager::bindPixelShader(m_psParticles);

            job->render(m_gfx, cam, renderWhat, renderFlags);

            eStateManager::pop();
        }
        else
        {
            job->render(m_gfx, cam, renderWhat, renderFlags);
        }
    }

    eStateManager::pop();
}

eU32 materialAssignmentPassCounter = 0;

void eDeferredRenderer::_assignMaterialIds()
{
    materialAssignmentPassCounter++;
    m_materialArray.clear();

    for (eU32 i=0, index=1; i<m_allJobs.size(); i++)
    {
        eRenderJob *job = m_allJobs[i];
        eASSERT(job != eNULL);

        const eMaterial *mat = job->getMaterial();
        eASSERT(mat != eNULL);

        // Only materials that use environment
        // mapping need a material index.
        if (mat->getTexture(eMaterial::UNIT_ENVIRONMENT) != eNULL)
        {
            if(mat->m_assignment_pass_id == materialAssignmentPassCounter)
            {   
                job->setMaterialIndex(mat->m_render_material_id);
            }
            else
            {
                m_materialArray.append(mat);
                mat->m_assignment_pass_id = materialAssignmentPassCounter;
                mat->m_render_material_id = index;
                job->setMaterialIndex(index++);
            }
        }
        else
        {
            // All non-env-mapping materials are assigned index zero.
            job->setMaterialIndex(0);
        }
    }
}

void eDeferredRenderer::_visualizeGeometryBuffer(const eRect &area) const
{
    eStateManager::push();
    eStateManager::bindVertexShader(m_vsQuad);
    eStateManager::bindPixelShader(m_psQuad);
    eStateManager::setCullingMode(eCULLING_NONE);
    eStateManager::setTextureAddressMode(0, eTEXADDRMODE_CLAMP);
    eStateManager::setCap(eCAP_ZBUFFER, eFALSE);
    eStateManager::setCap(eCAP_BLENDING, eFALSE);

    const eInt w = area.getWidth();
    const eInt h = area.getHeight();

    // Image at top left.
    eStateManager::bindTexture(0, m_specularRt);
   // renderQuad(eRect(0, h/2, w/2, h-1), area.getDimension());

    // Image at top right.
    eStateManager::bindTexture(0, m_diffuseRt);
    renderQuad(eRect(w/2, h/2, w-1, h-1), area.getDimension());

    // Image at bottom left.
    eStateManager::bindTexture(0, m_normalsRt);
    renderQuad(eRect(0, 0, w/2, h/2), area.getDimension());

    // Image at bottom right.
    eStateManager::bindTexture(0, m_positionRt);
    renderQuad(eRect(w/2, 0, w-1, h/2), area.getDimension());

    eStateManager::pop();
}