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

eIEffect::TargetArray eIEffect::m_targetPool;

eIEffect::eIEffect() :
    m_time(0.0f),
    m_renderer(eNULL),
    m_gfx(eNULL),
    m_ps(eNULL),
    m_vsQuad(eShaderManager::loadVertexShader(eVS(quad)))
{
}

eIEffect::~eIEffect()
{
}

void eIEffect::run(eF32 time, eITexture2d *target, eIRenderer *renderer)
{
    ePROFILER_ZONE("Run post FX");

    eASSERT(target != eNULL);
    eASSERT(time >= 0.0f);
    eASSERT(renderer != eNULL);

    m_time = time;
    m_renderer = renderer;
    m_gfx = renderer->getGraphicsApi();

    eStateManager::push();
    eStateManager::setCullingMode(eCULLING_NONE);
    eStateManager::setCap(eCAP_ZBUFFER, eFALSE);
    eStateManager::setCap(eCAP_BLENDING, eFALSE);
    eStateManager::bindVertexShader(m_vsQuad);
    eStateManager::setTextureFilter(0, eTEXFILTER_BILINEAR);
    eStateManager::setTextureFilter(1, eTEXFILTER_BILINEAR);
    eStateManager::bindTexture(2, renderer->getPositionMap());
    eStateManager::bindTexture(3, renderer->getNormalMap());

    // Clears cache of already rendered inputs
    // (e.g. scenes are not rendered twice).
    eInputEffect::clearCache();

    Target *resTarget = _runHierarchy();
    eASSERT(resTarget != eNULL);

    eStateManager::pop();

    eStateManager::push();
    eStateManager::bindRenderTarget(0, target);
    renderer->renderTexturedQuad(eRect(0, 0, target->getWidth(), target->getHeight()), target->getSize(), resTarget->tex);

    for (eU32 i=0; i<m_targetPool.size(); i++)
    {
        eASSERT(m_targetPool[i].inUse == eFALSE);
    }

    eStateManager::pop();
    _garbageCollectTargets();
}

void eIEffect::addInput(eITexture2d *tex)
{
    eASSERT(tex != eNULL);
    m_inputFx.append(new eInputEffect(tex));
}

void eIEffect::addInput(eScene &scene, const eCamera &cam)
{
    m_inputFx.append(new eInputEffect(scene, cam));
}

void eIEffect::addInput(eIEffect *fx)
{
    eASSERT(fx != eNULL);
    m_inputFx.append(fx);
}

void eIEffect::clearInputs()
{
    m_inputFx.clear();
}

void eIEffect::shutdown()
{
    for (eU32 i=0; i<m_targetPool.size(); i++)
    {
        Target &target = m_targetPool[i];

        eASSERT(target.inUse == eFALSE);
        eSAFE_DELETE(target.tex);
    }

    m_targetPool.clear();
}

eIEffect::Target * eIEffect::_getTarget(const eSize &size)
{
    ePROFILER_ZONE("Get target");

    eASSERT(size.width > 0 && size.height > 0);

    // Does an unused target already exists?
    for (eU32 i=0; i<m_targetPool.size(); i++)
    {
        Target &target = m_targetPool[i];

        if (target.tex->getSize() == size && !target.inUse)
        {
            // Yes, so return it.
            return &target;
        }
    }

    // No, so create and return a new one.
    m_targetPool.append(Target());
    Target &target = m_targetPool[m_targetPool.size()-1];

    target.tex = m_gfx->createTexture2d(size.width, size.height, eTRUE, eFALSE, eFALSE, eFORMAT_ARGB8);
    eASSERT(m_gfx != eNULL);
    target.inUse = eFALSE;
    target.wasUsed = eFALSE;
    target.createdBy = this;
    
    return &target;
}

eIEffect::Target * eIEffect::_renderSimple(Target *src, eIPixelShader *ps)
{
    eASSERT(src != eNULL);
    eASSERT(src->tex != eNULL);
    eASSERT(ps != eNULL);

    Target *dst = _getTarget(src->tex->getSize());
    eASSERT(dst != eNULL);
    dst->wasUsed = eTRUE;

    eStateManager::bindPixelShader(ps);
    eStateManager::bindTexture(0, src->tex);
    eStateManager::bindRenderTarget(0, dst->tex);

    m_renderer->renderQuad(eRect(0, 0, dst->tex->getWidth(), dst->tex->getHeight()), dst->tex->getSize());
    return dst;
}

eIEffect::Target * eIEffect::_runHierarchy()
{
    TargetPtrArray inputs;

    for (eU32 i=0; i<m_inputFx.size(); i++)
    {
        m_inputFx[i]->m_renderer = m_renderer;
        m_inputFx[i]->m_gfx = m_gfx;
        m_inputFx[i]->m_time = m_time;

        Target *rt = m_inputFx[i]->_runHierarchy();
        eASSERT(rt != eNULL);
        rt->inUse = eTRUE;
        
        inputs.append(rt);
    }

    eStateManager::push();
    Target *dst = _run(inputs);
    eStateManager::pop();

    eASSERT(dst != eNULL);
    dst->wasUsed = eTRUE;

    for (eU32 i=0; i<inputs.size(); i++)
    {
        inputs[i]->inUse = eFALSE;
    }

    return dst;
}

void eIEffect::_garbageCollectTargets()
{
    for (eInt i=(eInt)m_targetPool.size()-1; i>=0; i--)
    {
        Target &t = m_targetPool[i];

        if (!t.wasUsed && t.createdBy == this)
        {
            eSAFE_DELETE(t.tex);
            m_targetPool.removeAt(i);
        }
        else
        {
            t.wasUsed = eFALSE;
        }
    }
}

eIIterableEffect::eIIterableEffect(eU32 iterations) :
    m_iterations(iterations)
{
}

void eIIterableEffect::setIterations(eU32 iterations)
{
    m_iterations = iterations;
}

eU32 eIIterableEffect::getIterations() const
{
    return m_iterations;
}

eIEffect::Target * eIIterableEffect::_renderPingPong(Target *src, Target *dst) const
{
    eASSERT(src != eNULL);
    eASSERT(dst != eNULL);

    for (eU32 i=0; i<m_iterations; i++)
    {
        eStateManager::bindTexture(0, src->tex);
        eStateManager::bindRenderTarget(0, dst->tex);
        m_renderer->renderQuad(eRect(0, 0, dst->tex->getWidth(), dst->tex->getHeight()), dst->tex->getSize());

        eSwap(src, dst);
    }

    return src;
}

eInputEffect::SceneCache eInputEffect::m_sceneCache;

eInputEffect::eInputEffect(eScene &scene, const eCamera &cam) :
    m_scene(&scene),
    m_cam(cam),
    m_tex(eNULL)
{
}

eInputEffect::eInputEffect(eITexture2d *tex) :
    m_scene(eNULL),
    m_cam(45.0f, 1.33f, 0.1f, 1000.0f),
    m_tex(tex)
{
    eASSERT(tex != eNULL);
}

eInputEffect::~eInputEffect()
{
    clearCache();
}

void eInputEffect::clearCache()
{
    for (eInt i=(eInt)m_sceneCache.size()-1; i>=0; i--)
    {
        m_sceneCache[i].inUse = eFALSE;
    }
}

eIEffect::Target * eInputEffect::_run(TargetPtrArray &srcs)
{
    eASSERT(m_tex || m_scene);
    return (m_tex ? _runForTexture() : _runForScene());
}

eIEffect::Target * eInputEffect::_runForScene()
{
    // Find target for scene.
    Target *dst = _getTarget(m_gfx->getWindowSize());
    eASSERT(dst != eNULL);

    SceneCacheEntry &sce = _getSceneCacheEntry();

    // Was scene already rendered this frame?
    if (sce.inUse)
    {
        eStateManager::bindRenderTarget(0, dst->tex);
        m_renderer->renderTexturedQuad(eRect(0, 0, dst->tex->getWidth(), dst->tex->getHeight()), dst->tex->getSize(), sce.tex);
    }
    else
    {
        sce.inUse = eTRUE;

        // Unbind position and normal texture for input effect
        // (cause scene rendering use these textures as targets).
        eStateManager::bindTexture(2, eNULL);
        eStateManager::bindTexture(3, eNULL);

        m_renderer->renderScene(*m_scene, m_cam, sce.tex, m_time);

        eStateManager::bindRenderTarget(0, dst->tex);
        m_renderer->renderTexturedQuad(eRect(0, 0, dst->tex->getWidth(), dst->tex->getHeight()), dst->tex->getSize(), sce.tex);
    }

    return dst;
}

eIEffect::Target * eInputEffect::_runForTexture()
{
    Target *dst = _getTarget(m_gfx->getWindowSize());
    eASSERT(dst != eNULL);

    eStateManager::bindRenderTarget(0, dst->tex);
    m_renderer->renderTexturedQuad(eRect(0, 0, m_tex->getWidth(), m_tex->getHeight()), m_tex->getSize(), m_tex);

    return dst;
}

eInputEffect::SceneCacheEntry & eInputEffect::_getSceneCacheEntry()
{
    // Try to find a cache entry for the scene.
    for (eInt i=(eInt)m_sceneCache.size()-1; i>=0; i--)
    {
        SceneCacheEntry &sce = m_sceneCache[i];

        if (sce.tex->getSize() == m_gfx->getWindowSize())
        {
            if (!sce.inUse)
            {
                sce.scene = m_scene;
            }
             
            return sce;
        }
        else
        {
            eSAFE_DELETE(sce.tex);
            m_sceneCache.removeAt(i);
        }
    }

    // No scene cache entry found, create new one.
    SceneCacheEntry sce;

    sce.inUse = eTRUE;
    sce.tex = m_gfx->createTexture2d(m_gfx->getWindowSize().width, m_gfx->getWindowSize().height, eTRUE, eFALSE, eFALSE, eFORMAT_ARGB8);
    sce.scene = m_scene;

    m_sceneCache.append(sce);
    return m_sceneCache.lastElement();
}

eMergeEffect::eMergeEffect(BlendMode blendMode, const eVector2 &blendRatios) :
    m_blendMode(blendMode),
    m_blendRatios(blendRatios)
{
    m_ps = eShaderManager::loadPixelShader(ePS(fx_merge));
}

eIEffect::Target * eMergeEffect::_run(TargetPtrArray &srcs)
{
    eASSERT(srcs.size() > 0);

    eStateManager::bindPixelShader(m_ps);

    m_gfx->setPsConst(0, eVector4((eF32)m_blendMode, m_blendRatios.x, m_blendRatios.y, 0.0f));

    Target *src = srcs[0];
    eASSERT(src != eNULL);

    Target *dst = _getTarget(src->tex->getSize());
    eASSERT(dst != eNULL);
    dst->wasUsed = eTRUE;

    for (eU32 i=1; i<srcs.size(); i++)
    {
        eStateManager::bindTexture(0, src->tex);
        eStateManager::bindTexture(1, srcs[i]->tex);
        eStateManager::bindRenderTarget(0, dst->tex);

        m_renderer->renderQuad(eRect(0, 0, dst->tex->getWidth(), dst->tex->getHeight()), dst->tex->getSize());
        eSwap(src, dst);
    }

    return src;
}

eBlurEffect::eBlurEffect(Direction dir, eU32 distance) : eIIterableEffect(1),
    m_dir(dir),
    m_dist(dir == DIR_HORZ ? (eF32)distance : 0.0f, dir == DIR_VERT ? (eF32)distance : 0.0f)
{
    m_ps = eShaderManager::loadPixelShader(ePS(fx_blur));
}

eIEffect::Target * eBlurEffect::_run(TargetPtrArray &srcs)
{
    eASSERT(srcs.size() == 1);

    Target *dst = _getTarget(srcs[0]->tex->getSize());
    eASSERT(dst != eNULL);
    dst->wasUsed = eTRUE;

    eStateManager::bindPixelShader(m_ps);

    m_gfx->setPsConst(0, m_dist);
    m_gfx->setPsConst(15, 1.0f/eVector2((eF32)srcs[0]->tex->getWidth(), (eF32)srcs[0]->tex->getHeight()));

    return _renderPingPong(srcs[0], dst);
}

eDofEffect::eDofEffect(eF32 focusDepth, eF32 focusRange) :
    m_focusDepth(focusDepth),
    m_focusRange(focusRange)
{
    m_ps = eShaderManager::loadPixelShader(ePS(fx_dof));
}

eIEffect::Target * eDofEffect::_run(TargetPtrArray &srcs)
{
    eASSERT(srcs.size() == 2);

    m_gfx->setPsConst(0, eVector2(m_focusDepth, m_focusRange));

    eStateManager::bindTexture(1, srcs[1]->tex);
    return _renderSimple(srcs[0], m_ps);
}

eSsaoEffect::eSsaoEffect(eF32 scale, eF32 intensity, eF32 bias, eF32 radius, eITexture2d *noiseMap) :
    m_scale(scale),
    m_intensity(intensity),
    m_bias(bias),
    m_radius(radius),
    m_noiseMap(noiseMap)
{
    m_ps = eShaderManager::loadPixelShader(ePS(fx_ssao));
}

void eSsaoEffect::setNoiseMap(eITexture2d *noiseMap)
{
    m_noiseMap = noiseMap;
}

eIEffect::Target * eSsaoEffect::_run(TargetPtrArray &srcs)
{
    eASSERT(srcs.size() == 1);

    m_gfx->setPsConst(0, eVector4(m_scale, m_intensity/16.0f, m_bias, m_radius));

    eStateManager::bindTexture(4, m_noiseMap);
    eStateManager::setTextureAddressMode(4, eTEXADDRMODE_WRAP);
    return _renderSimple(srcs[0], m_ps);
}

eColorGradingEffect::eColorGradingEffect(eITexture3d *lookupMap) :
    m_lookupMap(lookupMap)
{
    m_ps = eShaderManager::loadPixelShader(ePS(fx_colorgrading));
}

eIEffect::Target * eColorGradingEffect::_run(TargetPtrArray &srcs)
{
    eASSERT(m_lookupMap != eNULL);
    eASSERT(srcs.size() == 1);

    eStateManager::bindTexture(4, m_lookupMap);
    eStateManager::setTextureAddressMode(4, eTEXADDRMODE_CLAMP);
    return _renderSimple(srcs[0], m_ps);
}

eFxaaEffect::eFxaaEffect()
{
    m_ps = eShaderManager::loadPixelShader(ePS(fx_fxaa));
}

eIEffect::Target * eFxaaEffect::_run(TargetPtrArray &srcs)
{
    eASSERT(srcs.size() == 1);

    m_gfx->setPsConst(15, 1.0f/eVector2((eF32)srcs[0]->tex->getWidth(), (eF32)srcs[0]->tex->getHeight()));
    return _renderSimple(srcs[0], m_ps);
}

eAdjustEffect::eAdjustEffect(eF32 brightness, eF32 contrast, const eColor &adjCol, const eColor &subCol) : eIIterableEffect(1),
    m_brightness(brightness),
    m_contrast(contrast),
    m_adjCol(adjCol),
    m_subCol(subCol)
{
    m_ps = eShaderManager::loadPixelShader(ePS(fx_adjust));
}

eIEffect::Target * eAdjustEffect::_run(TargetPtrArray &srcs)
{
    eASSERT(srcs.size() == 1);

    Target *buffer = _getTarget(srcs[0]->tex->getSize());
    eASSERT(buffer != eNULL);
    buffer->wasUsed = eTRUE;

    eStateManager::bindPixelShader(m_ps);

    // Pre-multiply color by brightness to save
    // computation time in shader.
    const eF32 *fcol = (const eF32 *)m_adjCol;
    eVector4 vcol(fcol[0], fcol[1], fcol[2], fcol[3]);
    vcol *= m_brightness;

    m_gfx->setPsConst(0, eVector2(m_contrast));
    m_gfx->setPsConst(1, vcol);
    m_gfx->setPsConst(2, m_subCol);

    return _renderPingPong(srcs[0], buffer);
}

eRadialBlurEffect::eRadialBlurEffect(const eVector2 &origin, eF32 distance, eF32 strength) :
    m_origin(origin),
    m_distance(distance),
    m_strength(strength)
{
    m_ps = eShaderManager::loadPixelShader(ePS(fx_radialblur));
}

eIEffect::Target * eRadialBlurEffect::_run(TargetPtrArray &srcs)
{
    eASSERT(srcs.size() == 1);

    m_gfx->setPsConst(0, eVector4(m_distance, m_strength, m_origin.x, m_origin.y));
    return _renderSimple(srcs[0], m_ps);
}

eRippleEffect::eRippleEffect(eF32 ampli, eF32 length, eF32 speed, eF32 time, const eVector2 &offset, Mode mode) : eIIterableEffect(1),
    m_ampli(ampli),
    m_length(length),
    m_speed(speed),
    m_time(time),
    m_offset(offset),
    m_mode(mode)
{
    m_ps = eShaderManager::loadPixelShader(ePS(fx_ripple));
}

eIEffect::Target * eRippleEffect::_run(TargetPtrArray &srcs)
{
    eASSERT(srcs.size() == 1);

    m_gfx->setPsConst(0, eVector4(m_ampli, m_length, m_speed, m_time));
    m_gfx->setPsConst(1, m_offset);
    m_gfx->setPsConst(2, (eF32)m_mode);

    return _renderSimple(srcs[0], m_ps);
}

eFogEffect::eFogEffect(Type type, eF32 start, eF32 end, eF32 density, const eColor &color) :
    m_type(type),
    m_start(start),
    m_end(end),
    m_color(color)
{
    m_ps = eShaderManager::loadPixelShader(ePS(fx_fog));
}

eIEffect::Target *eFogEffect::_run(TargetPtrArray &srcs)
{
    eASSERT(srcs.size() == 1);

    m_gfx->setPsConst(0, eVector4((eF32)m_type, m_start, m_end, m_density));
    m_gfx->setPsConst(1, m_color);

    return _renderSimple(srcs[0], m_ps);
}

eSaveEffect::eSaveEffect(eITexture2d *renderTarget, eITexture2d *depthTarget) :
    m_renderTarget(renderTarget),
    m_depthTarget(depthTarget)
{
}

eIEffect::Target * eSaveEffect::_run(TargetPtrArray &srcs)
{
    eASSERT(m_renderTarget != eNULL);
    eASSERT(m_depthTarget != eNULL);
    eASSERT(m_renderTarget->getSize() == m_depthTarget->getSize());
    eASSERT(srcs.size() == 1);

    eStateManager::push();
    eStateManager::bindRenderTarget(0, m_renderTarget);
    eStateManager::bindDepthTarget(m_depthTarget);
    m_renderer->renderTexturedQuad(eRect(0, 0, m_renderTarget->getWidth(), m_renderTarget->getHeight()), m_renderTarget->getSize(), srcs[0]->tex);
    eStateManager::pop();

    return srcs[0];
}

eDownsampleEffect::eDownsampleEffect(const eVector2 &amount) : eIIterableEffect(1),
    m_amount(amount)
{
    m_ps = eShaderManager::loadPixelShader(ePS(quad));
}

eIEffect::Target * eDownsampleEffect::_run(TargetPtrArray &srcs)
{
    eASSERT(srcs.size() == 1);

    Target *src = srcs[0];
    eASSERT(src != eNULL);

    for (eU32 i=0; i<m_iterations; i++)
    {
        const eF32 newWidth = m_amount.x*(eF32)src->tex->getWidth();
        const eF32 newHeight = m_amount.y*(eF32)src->tex->getHeight();
        const eSize newSize(eMax(1, eFtoL(newWidth)), eMax(1, eFtoL(newHeight)));

        Target *dst = _getTarget(newSize);
        eASSERT(dst != eNULL);

        eStateManager::bindPixelShader(m_ps);
        eStateManager::bindTexture(0, src->tex);
        eStateManager::bindRenderTarget(0, dst->tex);
        m_renderer->renderQuad(eRect(0, 0, dst->tex->getWidth(), dst->tex->getHeight()), dst->tex->getSize());

        src = dst;
    }

    return src;
}

eDistortEffect::eDistortEffect(eVector2 intensity, eVector2 offset, eITexture2d *distortMap) :
	m_intensity(intensity),
	m_offset(offset),
	m_distortMap(distortMap)
{
	m_ps = eShaderManager::loadPixelShader(ePS(fx_distort));
}

void eDistortEffect::setDistortMap(eITexture2d *noiseMap)
{
	m_distortMap = noiseMap;
}

eIEffect::Target * eDistortEffect::_run(TargetPtrArray &srcs)
{
	eASSERT(srcs.size() == 1);

	m_gfx->setPsConst(0, eVector4(m_intensity.x, m_intensity.y, m_offset.x, m_offset.y));

	eStateManager::bindTexture(4, m_distortMap);
	eStateManager::setTextureAddressMode(4, eTEXADDRMODE_WRAP);
	return _renderSimple(srcs[0], m_ps);
}