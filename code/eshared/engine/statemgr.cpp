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

// Initialize static member variables.
eGraphicsApiDx9 *             eStateManager::m_gfx = eNULL;
eStateManager::RenderStates eStateManager::m_activeStates;
eStateManager::RenderStates eStateManager::m_newStates;
eStateManager::RenderStates eStateManager::m_statesStack[eStateManager::STACK_SIZE];
eInt                        eStateManager::m_stackPos = STACK_EMPTY;
eBool                       eStateManager::m_changed = eTRUE;

void eStateManager::setGraphics(eGraphicsApiDx9 *gfx)
{
    eASSERT(gfx != eNULL);
    m_gfx = gfx;
}

void eStateManager::forceApply()
{
    apply(eTRUE);
}

void eStateManager::apply(eBool forceApply)
{
    ePROFILER_ZONE("State changes");

    // If no state has changed since last call
    // and applying isn't forced then return.
    if (!m_changed && !forceApply)
    {
        return;
    }

    if (m_activeStates.scissorRect != m_newStates.scissorRect || forceApply)
    {
        m_gfx->setScissorRect(m_newStates.scissorRect);
    }

    for (eU32 i=0; i<eMAX_CAP_COUNT; i++)
    {
        if (m_activeStates.caps[i] != m_newStates.caps[i] || forceApply)
        {
            m_gfx->setCap((eRenderCap)i, m_newStates.caps[i]);
        }
    }

    for (eU32 i=0; i<eGraphicsApiDx9::MAX_TEX_UNITS; i++)
    {
        if (m_activeStates.texture[i] != m_newStates.texture[i] || forceApply)
        {
            m_gfx->setTexture(i, m_newStates.texture[i]);
        }

        if (m_activeStates.texFilters[i] != m_newStates.texFilters[i] || forceApply)
        {
            m_gfx->setTextureFilter(i, m_newStates.texFilters[i]);
        }

        if (m_activeStates.texAddrModes[i] != m_newStates.texAddrModes[i] || forceApply)
        {
            m_gfx->setTextureAddressMode(i, m_newStates.texAddrModes[i]);
        }
    }

    for (eU32 i=0; i<eGraphicsApiDx9::MAX_TARGETS; i++)
    {
        if (m_activeStates.targets[i] != m_newStates.targets[i] || 
            m_activeStates.targetFace[i] != m_newStates.targetFace[i] || forceApply)
        {
            m_gfx->setRenderTarget(i, m_newStates.targets[i], m_newStates.targetFace[i]);
        }
    }

    if (m_activeStates.alphaTest != m_newStates.alphaTest || forceApply)
    {
        m_gfx->setAlphaTest(m_newStates.alphaTest);
    }

    if (m_activeStates.blendDst != m_newStates.blendDst || 
        m_activeStates.blendSrc != m_newStates.blendSrc ||
        m_activeStates.blendOp != m_newStates.blendOp || 
        forceApply)
    {
        m_gfx->setBlendModes(m_newStates.blendSrc, m_newStates.blendDst, m_newStates.blendOp);
    }

    if (m_activeStates.depthTarget != m_newStates.depthTarget || forceApply)
    {
        m_gfx->setDepthTarget(m_newStates.depthTarget);
    }

    if (m_activeStates.zFunc != m_newStates.zFunc || forceApply)
    {
        m_gfx->setZFunction(m_newStates.zFunc);
    }

    if (m_activeStates.polyMode != m_newStates.polyMode || forceApply)
    {
        m_gfx->setPolygonMode(m_newStates.polyMode);
    }

    if (m_activeStates.cullingMode != m_newStates.cullingMode || forceApply)
    {
        m_gfx->setCullingMode(m_newStates.cullingMode);
    }

    if (m_activeStates.vertexShader != m_newStates.vertexShader || forceApply)
    {
        m_gfx->setVertexShader(m_newStates.vertexShader);
    }

    if (m_activeStates.pixelShader != m_newStates.pixelShader || forceApply)
    {
        m_gfx->setPixelShader(m_newStates.pixelShader);
    }

    for (eU32 i=0; i<2; i++)
    {
        const RenderStates::VertexBufferInfos &vbi0 = m_activeStates.vtxBufInfos[i];
        const RenderStates::VertexBufferInfos &vbi1 = m_newStates.vtxBufInfos[i];

        if (!eMemEqual(&vbi0, &vbi1, sizeof(vbi0)))
        {
            m_gfx->setVertexBuffer(i, vbi1.vertexBuffer, vbi1.vertexType, vbi1.byteOffset, vbi1.instanceCount);
        }
    }

    if (m_activeStates.indexBuffer != m_newStates.indexBuffer || forceApply)
    {
        m_gfx->setIndexBuffer(m_newStates.indexBuffer);
    }

    if (m_activeStates.viewport.x != m_newStates.viewport.x ||
        m_activeStates.viewport.y != m_newStates.viewport.y ||
        m_activeStates.viewport.width != m_newStates.viewport.width ||
        m_activeStates.viewport.height != m_newStates.viewport.height || forceApply)
    {
        m_gfx->setViewport(m_newStates.viewport.x,
                           m_newStates.viewport.y,
                           m_newStates.viewport.width,
                           m_newStates.viewport.height);
    }

    // Active state gets new state.
    m_activeStates = m_newStates;
    m_changed = eFALSE;
}

void eStateManager::reset()
{
    m_changed                           = eTRUE;

    m_newStates.alphaTest               = eFALSE;
    m_newStates.blendDst                = eBLEND_ONE;
    m_newStates.blendSrc                = eBLEND_ONE;
    m_newStates.blendOp                 = eBLENDOP_ADD;
    m_newStates.zFunc                   = eZFUNC_LESSEQUAL;
    m_newStates.polyMode                = ePOLYMODE_FILLED;
    m_newStates.cullingMode             = eCULLING_NONE;

    m_newStates.viewport.x              = 0;
    m_newStates.viewport.y              = 0;
    m_newStates.viewport.width          = 0;
    m_newStates.viewport.height         = 0;

    m_newStates.pixelShader             = eNULL;
    m_newStates.vertexShader            = eNULL;
    m_newStates.indexBuffer             = eNULL;

    m_newStates.caps[eCAP_BLENDING]     = eFALSE;
    m_newStates.caps[eCAP_ZBUFFER]      = eFALSE;
    m_newStates.caps[eCAP_ZWRITE]       = eTRUE;
    m_newStates.caps[eCAP_COLORWRITE]   = eTRUE;
    m_newStates.caps[eCAP_SCISSORTEST]  = eFALSE;

    eMemSet(m_newStates.vtxBufInfos, 0, sizeof(m_newStates.vtxBufInfos));

    for (eU32 i=0; i<eGraphicsApiDx9::MAX_TEX_UNITS; i++)
    {
        m_newStates.texture[i] = eNULL;
        m_newStates.texFilters[i] = eTEXFILTER_BILINEAR;
    }

    for (eU32 i=0; i<eGraphicsApiDx9::MAX_TARGETS; i++)
    {
        m_newStates.targets[i] = eNULL;
        m_newStates.targetFace[i] = eCMFACE_POSX;
    }

    m_newStates.targets[0] = eGraphicsApiDx9::TARGET_SCREEN;
    m_newStates.depthTarget = eGraphicsApiDx9::TARGET_SCREEN;
}

// Pushes the new state struct, not the active state struct!
void eStateManager::push()
{
    eASSERT(m_stackPos < STACK_SIZE);
    m_statesStack[++m_stackPos] = m_newStates;
}

void eStateManager::pop()
{
    eASSERT(m_stackPos > STACK_EMPTY);
    m_newStates = m_statesStack[m_stackPos--];
}

void eStateManager::setCap(eRenderCap cap, eBool enabled)
{
    m_newStates.caps[cap] = enabled;
    m_changed = eTRUE;
}

void eStateManager::setBlendModes(eBlendMode src, eBlendMode dst, eBlendOp op)
{
    m_newStates.blendSrc = src;
    m_newStates.blendDst = dst;
    m_newStates.blendOp = op;

    m_changed = eTRUE;
}

void eStateManager::setAlphaTest(eBool enabled)
{
    m_newStates.alphaTest = enabled;
    m_changed = eTRUE;
}

void eStateManager::setPolygonMode(ePolygonMode mode)
{
    m_newStates.polyMode = mode;
    m_changed = eTRUE;
}

void eStateManager::setViewport(eU32 x, eU32 y, eU32 width, eU32 height)
{
    m_newStates.viewport.x = x;
    m_newStates.viewport.y = y;
    m_newStates.viewport.width = width;
    m_newStates.viewport.height = height;

    m_changed = eTRUE;
}

void eStateManager::setCullingMode(eCullingMode cm)
{
    m_newStates.cullingMode = cm;
    m_changed = eTRUE;
}

void eStateManager::setZFunction(eZFunction zFunc)
{
    m_newStates.zFunc = zFunc;
    m_changed = eTRUE;
}

void eStateManager::setTextureFilter(eU32 unit, eTextureFilter texFilter)
{
    eASSERT(unit < eGraphicsApiDx9::MAX_TEX_UNITS);

    m_newStates.texFilters[unit] = texFilter;
    m_changed = eTRUE;
}

void eStateManager::setTextureAddressMode(eU32 unit, eTextureAddressMode address)
{
    eASSERT(unit < eGraphicsApiDx9::MAX_TEX_UNITS);

    m_newStates.texAddrModes[unit] = address;
    m_changed = eTRUE;
}

void eStateManager::bindVertexBuffer(eU32 index, eIVertexBuffer *vb, eVertexType vertexType, eU32 byteOffset, eU32 instanceCount)
{
    eASSERT(index < 2);

    RenderStates::VertexBufferInfos &vbi = m_newStates.vtxBufInfos[index];

    vbi.vertexBuffer    = vb;
    vbi.vertexType      = vertexType;
    vbi.byteOffset      = byteOffset;
    vbi.instanceCount   = instanceCount;

    m_changed = eTRUE;
}

void eStateManager::bindIndexBuffer(eIIndexBuffer *ib)
{
    m_newStates.indexBuffer = ib;
    m_changed = eTRUE;
}

void eStateManager::bindVertexShader(eIVertexShader *vs)
{
    m_newStates.vertexShader = vs;
    m_changed = eTRUE;
}

void eStateManager::bindPixelShader(eIPixelShader *ps)
{
    m_newStates.pixelShader = ps;
    m_changed = eTRUE;
}

void eStateManager::bindTexture(eU32 unit, eITexture *tex)
{
    eASSERT(unit < eGraphicsApiDx9::MAX_TEX_UNITS);

    m_newStates.texture[unit] = tex;
    m_changed = eTRUE;
}

void eStateManager::bindRenderTarget(eU32 index, eITexture *tex, eCubeMapFace face)
{
    eASSERT(index < eGraphicsApiDx9::MAX_TARGETS);

    m_newStates.targets[index] = tex;
    m_newStates.targetFace[index] = face;
    m_changed = eTRUE;
}

void eStateManager::bindDepthTarget(eITexture2d *tex)
{
    m_newStates.depthTarget = tex;
    m_changed = eTRUE;
}

void eStateManager::setScissorRect(const eRect &rect)
{
    eASSERT(rect.left <= rect.right);
    eASSERT(rect.top <= rect.bottom);

    m_newStates.scissorRect = rect;
    m_changed = eTRUE;
}

eBool eStateManager::getCap(eRenderCap cap)
{
    return m_activeStates.caps[cap];
}

void eStateManager::getBlendModes(eBlendMode &src, eBlendMode &dst, eBlendOp &op)
{
    src = m_activeStates.blendSrc;
    dst = m_activeStates.blendDst;
    op  = m_activeStates.blendOp;
}

eBool eStateManager::getAlphaTest()
{
    return m_activeStates.alphaTest;
}

ePolygonMode eStateManager::getPolygonMode()
{
    return m_activeStates.polyMode;
}

eViewport eStateManager::getViewport()
{
    return m_activeStates.viewport;
}

eCullingMode eStateManager::getCullingMode()
{
    return m_activeStates.cullingMode;
}

eZFunction eStateManager::getZFunction()
{
    return m_activeStates.zFunc;
}

eTextureFilter eStateManager::getTextureFilter(eU32 unit)
{
    eASSERT(unit < eGraphicsApiDx9::MAX_TEX_UNITS);
    return m_activeStates.texFilters[unit];
}

eTextureAddressMode eStateManager::getTextureAddressMode(eU32 unit)
{
    eASSERT(unit < eGraphicsApiDx9::MAX_TEX_UNITS);
    return m_activeStates.texAddrModes[unit];
}

eIVertexBuffer * eStateManager::getVertexBuffer(eU32 index)
{
    return m_activeStates.vtxBufInfos[index].vertexBuffer;
}

eIIndexBuffer * eStateManager::getIndexBuffer()
{
    return m_activeStates.indexBuffer;
}

eIVertexShader * eStateManager::getVertexShader()
{
    return m_activeStates.vertexShader;
}

eIPixelShader * eStateManager::getPixelShader()
{
    return m_activeStates.pixelShader;
}

eITexture * eStateManager::getTexture(eU32 unit)
{
    eASSERT(unit < eGraphicsApiDx9::MAX_TEX_UNITS);
    return m_activeStates.texture[unit];
}

eITexture * eStateManager::getRenderTarget(eU32 index)
{
    eASSERT(index < eGraphicsApiDx9::MAX_TARGETS);
    return m_newStates.targets[index];
}

eITexture2d * eStateManager::getDepthTarget()
{
    return m_newStates.depthTarget;
}

eRect eStateManager::getScissorRect()
{
    return m_newStates.scissorRect;
}

eStateManager::RenderStates & eStateManager::getActiveStates()
{
    return m_activeStates;
}

eStateManager::RenderStates & eStateManager::getNewStates()
{
    return m_newStates;
}

eStateManager::RenderStates & eStateManager::getStackStates(eU32 index)
{
    eASSERT((eInt)index < m_stackPos+1);
    return m_statesStack[index];
}

eU32 eStateManager::getStatesStackSize()
{
    return m_stackPos+1;
}