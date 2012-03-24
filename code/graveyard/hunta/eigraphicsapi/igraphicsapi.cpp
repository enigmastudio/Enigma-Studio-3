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

eITexture2d * eIGraphicsApi::TARGET_SCREEN = (eITexture2d *)0xdeadbeef;

eIGraphicsApi::eIGraphicsApi()
{
    eResourceManager::setGraphics(this);
    eStateManager::setGraphics(this);
    eStateManager::reset();
}

eIGraphicsApi::~eIGraphicsApi()
{
}

eBool eITexture::unload()
{
    // Make sure this texture isn't bound to any state anymore.
    for (eU32 i=0; i<eIGraphicsApi::MAX_TARGETS; i++)
    {
        if (eStateManager::getNewStates().texture[i] == this)
        {
            eStateManager::getNewStates().texture[i] = eNULL;
        }

        if (eStateManager::getActiveStates().texture[i] == this)
        {
            eStateManager::getActiveStates().texture[i] = eNULL;
        }

        for (eU32 j=0; j<eStateManager::getStatesStackSize(); j++)
        {
            if (eStateManager::getStackStates(j).texture[i] == this)
            {
                eStateManager::getStackStates(j).texture[i] = eNULL;
            }
        }
    }

    return eTRUE;
}

eBool eIIndexBuffer::unload()
{
    // Make sure this index buffer isn't bound to any state anymore.
    if (eStateManager::getNewStates().indexBuffer == this)
    {
        eStateManager::getNewStates().indexBuffer = eNULL;
    }

    if (eStateManager::getActiveStates().indexBuffer == this)
    {
        eStateManager::getActiveStates().indexBuffer = eNULL;
    }

    for (eU32 j=0; j<eStateManager::getStatesStackSize(); j++)
    {
        if (eStateManager::getStackStates(j).indexBuffer == this)
        {
            eStateManager::getStackStates(j).indexBuffer = eNULL;
        }
    }

    return eTRUE;
}

eBool eIVertexBuffer::unload()
{
    // Make sure this vertex buffer isn't bound to any state anymore.
    for (eU32 i=0; i<2; i++)
    {
        eIVertexBuffer *& vbNew = eStateManager::getNewStates().vtxBufInfos[i].vertexBuffer;
        eIVertexBuffer *& vbActive = eStateManager::getNewStates().vtxBufInfos[i].vertexBuffer;

        if (vbNew == this)
        {
            vbNew = eNULL;
        }

        if (vbActive == this)
        {
            vbActive = eNULL;
        }

        for (eU32 j=0; j<eStateManager::getStatesStackSize(); j++)
        {
            eIVertexBuffer *& vb = eStateManager::getNewStates().vtxBufInfos[i].vertexBuffer;

            if (vb == this)
            {
                vb = eNULL;
            }
        
        }
    }

    return eTRUE;
}

eITexture2d * eIGraphicsApi::createChessTexture(eU32 width, eU32 height, eU32 step, const eColor &col0, const eColor &col1) const
{
    eASSERT(width > 2);
    eASSERT(height > 2);
    eASSERT(step < width);
    eASSERT(step < height);
    eASSERT(width%step == 0);
    eASSERT(height%step == 0);

    eITexture2d *tex = createTexture2d(width, height, eFALSE, eTRUE, eFALSE, eFORMAT_ARGB8);
    eColor *data = (eColor *)tex->lock();

    const eColor colors[2] =
    {
        col0,
        col1
    };

    for (eU32 y=0, index=0; y<height; y++)
    {
        const eU32 yds = y/step;

        for (eU32 x=0; x<width; x++)
        {
            const eU32 col = ((x/step)+yds)%2;
            data[index++] = colors[col];
        }
    }

    tex->unlock();
    return tex;
}

void eIGraphicsApi::setActiveMatrices(const eMatrix4x4 &modelMtx, const eMatrix4x4 &viewMtx, const eMatrix4x4 &projMtx)
{
    m_activeModelMtx = modelMtx;
    m_activeViewMtx = viewMtx;
    m_activeModelViewMtx = modelMtx*viewMtx;
    m_activeProjMtx = projMtx;

    const eMatrix4x4 mvpMtx = m_activeModelViewMtx*projMtx;

    setVsConst(eVSCONST_VIEW_MATRIX, viewMtx);
    setVsConst(eVSCONST_PROJ_MATRIX, projMtx);
    setVsConst(eVSCONST_MVP_MATRIX, mvpMtx);
}

void eIGraphicsApi::setPsConst(eU32 offset, eF32 f)
{
    setPsConst(offset, eVector2(f));
}

void eIGraphicsApi::setPsConst(eU32 offset, const eMatrix4x4 &m)
{
    setPsConst(offset, 4, m.m);
}

void eIGraphicsApi::setPsConst(eU32 offset, const eColor &v)
{
    setPsConst(offset, 1, v);
}

void eIGraphicsApi::setPsConst(eU32 offset, const eVector4 &v)
{
    setPsConst(offset, 1, v);
}

void eIGraphicsApi::setPsConst(eU32 offset, const eVector3 &v)
{
    const eF32 c[4] = {v.x, v.y, v.z, 0.0f};
    setPsConst(offset, 1, c);
}

void eIGraphicsApi::setPsConst(eU32 offset, const eVector2 &v)
{
    const eF32 c[4] = {v.x, v.y, 0.0f, 0.0f};
    setPsConst(offset, 1, c);
}

void eIGraphicsApi::setVsConst(eU32 offset, eF32 f)
{
    setVsConst(offset, eVector2(f));
}

void eIGraphicsApi::setVsConst(eU32 offset, const eMatrix4x4 &m)
{
    setVsConst(offset, 4, m);
}

void eIGraphicsApi::setVsConst(eU32 offset, const eVector4 &v)
{
    setVsConst(offset, 1, v);
}

void eIGraphicsApi::setVsConst(eU32 offset, const eVector3 &v)
{
    const eF32 c[4] = {v.x, v.y, v.z, 0.0f};
    setVsConst(offset, 1, c);
}

void eIGraphicsApi::setVsConst(eU32 offset, const eVector2 &v)
{
    const eF32 c[4] = {v.x, v.y, 0.0f, 0.0f};
    setVsConst(offset, 1, c);
}

eMatrix4x4 eIGraphicsApi::getActiveViewMatrix() const
{
    return m_activeViewMtx;
}

eMatrix4x4 eIGraphicsApi::getActiveModelMatrix() const
{
    return m_activeModelMtx;
}

eMatrix4x4 eIGraphicsApi::getActiveProjectionMatrix() const
{
    return m_activeProjMtx;
}

void eIGraphicsApi::getBillboardVectors(eVector3 &right, eVector3 &up, eVector3 *view) const
{
    right.set(m_activeViewMtx.m11, m_activeViewMtx.m21, m_activeViewMtx.m31);
    up.set(m_activeViewMtx.m12, m_activeViewMtx.m22, m_activeViewMtx.m32);

    right.normalize();
    up.normalize();

    if (view)
    {
        view->set(m_activeViewMtx.m13, m_activeViewMtx.m23, m_activeViewMtx.m33);
        view->normalize();
    }
}