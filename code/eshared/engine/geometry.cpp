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

// Initialize static members.
eIVertexBuffer *    eGeometry::m_dynamicVb  = eNULL;
eIIndexBuffer *     eGeometry::m_dynamicIb  = eNULL;
eIVertexBuffer *    eGeometry::m_instanceVb = eNULL;
eU32                eGeometry::m_dynVbPos   = 0;
eU32                eGeometry::m_dynIbIndex = 0;
eU32                eGeometry::m_instVbPos  = 0;
eGraphicsApiDx9 *     eGeometry::m_gfx        = eNULL;

eGeometry::eGeometry(eU32 vertexCount, eU32 indexCount, eU32 primCount, eVertexType vertexType,
                    Type type, ePrimitiveType primType, FillCallback fillCb, ePtr cbParam) :
    m_loading(eFALSE),
    m_type(type),
    m_indexed(type == TYPE_DYNAMIC_INDEXED || type == TYPE_STATIC_INDEXED),
    m_dynamic(type == TYPE_DYNAMIC || type == TYPE_DYNAMIC_INDEXED),
    m_primType(primType),
    m_vertexType(vertexType),
    m_vertexSize(eVERTEX_SIZES[vertexType]),
    m_vertexCount(vertexCount),
    m_indexCount(indexCount),
    m_primCount(primCount),
    m_usedVertexCount(vertexCount),
    m_usedIndexCount(indexCount),
    m_usedPrimCount(primCount),
    m_lastDynVbFill(0),
    m_lastDynIbFill(0),
    m_staticIb(eNULL),
    m_staticVb(eNULL),
    m_fillCb(fillCb),
    m_cbParam(cbParam)
{
    if (!m_dynamic)
    {
        _createStaticBuffers();
    }
}

eGeometry::~eGeometry()
{
    eASSERT(m_loading == eFALSE);

    this->removeInstanciations();
    if (!m_dynamic)
    {
        _freeStaticBuffers();
    }
}

void eGeometry::initialize(eGraphicsApiDx9 *gfx)
{
    eASSERT(gfx != eNULL);

    m_gfx = gfx;

    _createDynamicBuffers();
    _createInstanceBuffer();
}

// Has to be called before exiting applications to
// free dynamic buffers, because they are static.
void eGeometry::shutdown()
{
    eSAFE_DELETE(m_instanceVb);
    eSAFE_DELETE(m_dynamicVb);
    eSAFE_DELETE(m_dynamicIb);
}

void eGeometry::startFilling(ePtr *vertices, eU32 **indices)
{
    eASSERT(m_loading == eFALSE);

    if (m_dynamic)
    {
        _lockDynamicBuffers(vertices, indices);
    }
    else
    {
        _lockStaticBuffers(vertices, indices);
    }

    m_loading = eTRUE;
}

// Vertex, index and primitive counts can be given
// after loading buffers again. This way you can
// adjust the counts after iterating through a list of
// let's say visible or invisible primitives. No extra
// 'counting pass' is needed before.
void eGeometry::stopFilling(eBool countChanged, eU32 vertexCount, eU32 indexCount, eU32 primCount)
{
    eASSERT(m_loading == eTRUE);

    eASSERT(vertexCount <= m_vertexCount);
    eASSERT(indexCount <= m_indexCount);
    eASSERT(primCount <= m_primCount);

    if (countChanged)
    {
        m_usedVertexCount = vertexCount;
        m_usedIndexCount = indexCount;
        m_usedPrimCount = primCount;
    }
    else
    {
        m_usedVertexCount = m_vertexCount;
        m_usedIndexCount = m_indexCount;
        m_usedPrimCount = m_primCount;
    }

    if (m_dynamic)
    {
        m_lastDynVbFill += m_usedVertexCount;
        m_lastDynIbFill += m_usedIndexCount;

        _unlockDynamicBuffers();
    }
    else
    {
        _unlockStaticBuffers();
    }

    m_loading = eFALSE;
}

void eGeometry::render(const eArray<eLinkedInstanceVertex>* instBuf, eInt firstInstanceIdx, eInt instCount)
{
    // Dynamic buffer that has to be filled by callback function
    // (for example because it's deferred rendered from a queue).
    if (m_dynamic && m_fillCb)
    {
        m_fillCb(m_cbParam, this);
    }

    eASSERT(m_loading == eFALSE);

    // Don't do anything if no geometry was filled in.
    if (m_usedPrimCount == 0 || m_usedVertexCount == 0)
    {
        return;
    }

    if (instBuf != eNULL)
    {
        // Instancing only allowed for indexed geometry.
        eASSERT(m_indexed == eTRUE);

        eInstanceVertex *agpInstBuf = eNULL;

        _lockInstanceBuffer(&agpInstBuf, instCount);
        eInt idx = firstInstanceIdx;
        eInstanceVertex* agpInstBufPtr = (eInstanceVertex*)agpInstBuf;
        while(idx != -1) {
            (*agpInstBufPtr++) = (*instBuf)[idx].vtx;
            idx = (*instBuf)[idx].next;
        }
        _unlockInstanceBuffer();
        eStateManager::bindVertexBuffer(1, m_instanceVb, eVTXTYPE_INSTANCE, m_instVbPos, instCount);
        m_instVbPos += instCount*sizeof(eInstanceVertex);
    }

    if (m_dynamic)
    {
        eStateManager::bindVertexBuffer(0, m_dynamicVb, m_vertexType, 0, instCount);

        eASSERT(m_dynVbPos%m_vertexSize == 0);
        const eU32 baseIndex = m_dynVbPos/m_vertexSize;

        if (m_indexed)
        {
            eStateManager::bindIndexBuffer(m_dynamicIb);
            eStateManager::apply();
            m_gfx->drawIndexedPrimitives(m_primType, baseIndex, m_usedVertexCount, m_dynIbIndex, m_usedPrimCount, instCount);
        }
        else
        {
            eStateManager::bindIndexBuffer(eNULL);
            eStateManager::apply();
            m_gfx->drawPrimitives(m_primType, baseIndex, m_usedPrimCount);
        }

        m_dynVbPos += m_lastDynVbFill*m_vertexSize;
        m_dynIbIndex += m_lastDynIbFill;
        m_lastDynVbFill = 0;
        m_lastDynIbFill = 0;
    }
    else
    {
        eStateManager::bindVertexBuffer(0, m_staticVb, m_vertexType, 0, instCount);

        if (m_indexed)
        {
            eStateManager::bindIndexBuffer(m_staticIb);
            eStateManager::apply();
            m_gfx->drawIndexedPrimitives(m_primType, 0, m_usedVertexCount, 0, m_usedPrimCount, instCount);
        }
        else
        {
            eStateManager::bindIndexBuffer(eNULL);
            eStateManager::apply();
            m_gfx->drawPrimitives(m_primType, 0, m_usedPrimCount);
        }
    }
}

eGraphicsApiDx9 * eGeometry::getGraphics() const
{
    return m_gfx;
}

void eGeometry::_createInstanceBuffer()
{
    eASSERT(m_instanceVb == eNULL);
    m_instanceVb = m_gfx->createVertexBuffer(INSTANCE_VB_ELEMENTS*sizeof(eInstanceVertex), eTRUE);
}

void eGeometry::_createDynamicBuffers()
{
    eASSERT(m_dynamicIb == eNULL);
    eASSERT(m_dynamicVb == eNULL);

    m_dynamicIb = m_gfx->createIndexBuffer(DYNAMIC_IB_ELEMENTS, eTRUE);
    m_dynamicVb = m_gfx->createVertexBuffer(DYNAMIC_VB_ELEMENTS*sizeof(eVertex), eTRUE);
}

void eGeometry::_lockInstanceBuffer(eInstanceVertex **instances, eU32 instCount)
{
    eASSERT(instances != eNULL);
    eASSERT(instCount > 0);

    const eU32 reqBufSize = instCount*sizeof(eInstanceVertex);
    const eU32 nextInstVbPos = m_instVbPos+reqBufSize;

    eASSERT(reqBufSize <= m_instanceVb->getByteSize());

    if (nextInstVbPos >= m_instanceVb->getByteSize())
    {
        m_instVbPos = 0;
        *instances = (eInstanceVertex *)m_instanceVb->lock(0, 0, eLOCK_DISCARD);
    }
    else
    {
        *instances = (eInstanceVertex *)m_instanceVb->lock(m_instVbPos, reqBufSize, eLOCK_NOOVERWRITE);
    }

    eASSERT(*instances != eNULL);
}

void eGeometry::_unlockInstanceBuffer()
{
    m_instanceVb->unlock();
}

void eGeometry::_lockDynamicBuffers(ePtr *vertices, eU32 **indices)
{
    eASSERT(vertices != eNULL);
    eASSERT(m_loading == eFALSE);

    // Round position in vertex buffer up to a multiple
    // of the used vertex size so that the DIP/DP call
    // can correctly index the vertex data.
    m_dynVbPos = eRoundToMultiple(m_dynVbPos, m_vertexSize);

    const eU32 reqBufSize = m_vertexCount*m_vertexSize;
    const eU32 nextVbPos = m_dynVbPos+reqBufSize;

	eASSERT(reqBufSize <= m_dynamicVb->getByteSize());

    if (nextVbPos >= m_dynamicVb->getByteSize())
    {
        m_dynVbPos = 0;
        *vertices = m_dynamicVb->lock(0, 0, eLOCK_DISCARD);
    }
    else
    {
        *vertices = m_dynamicVb->lock(m_dynVbPos, reqBufSize, eLOCK_NOOVERWRITE);
    }

    eASSERT(*vertices != eNULL);

    // Lock index buffer.
    if (m_indexed)
    {
        eASSERT(indices != eNULL);

        const eU32 nextIbIndex = m_dynIbIndex+m_indexCount;

        if (nextIbIndex >= m_dynamicIb->getCount())
        {
            m_dynIbIndex = 0;
            *indices = m_dynamicIb->lock(0, 0, eLOCK_DISCARD);
        }
        else
        {
            *indices = m_dynamicIb->lock(m_dynIbIndex, m_indexCount, eLOCK_NOOVERWRITE);
        }

        eASSERT(*indices != eNULL);
    }
}

void eGeometry::_unlockDynamicBuffers()
{
    eASSERT(m_loading == eTRUE);

    m_dynamicVb->unlock();

    if (m_indexed)
    {
        m_dynamicIb->unlock();
    }
}

void eGeometry::_createStaticBuffers()
{
    eASSERT(m_staticIb == eNULL);
    eASSERT(m_staticVb == eNULL);
    eASSERT(m_loading == eFALSE);

    m_staticVb = m_gfx->createVertexBuffer(m_vertexCount*eVERTEX_SIZES[m_vertexType], eTRUE);

    if (m_indexed)
    {
        eASSERT(m_indexCount > 0);
        m_staticIb = m_gfx->createIndexBuffer(m_indexCount, eFALSE);
    }
}

void eGeometry::_freeStaticBuffers()
{
    eSAFE_DELETE(m_staticVb);
    eSAFE_DELETE(m_staticIb);
}

void eGeometry::_lockStaticBuffers(ePtr *vertices, eU32 **indices)
{
    eASSERT(vertices != eNULL);
    eASSERT(m_loading == eFALSE);
    eASSERT(m_staticVb != eNULL);

    *vertices = m_staticVb->lock(0, 0, eLOCK_DEFAULT);

    if (m_indexed)
    {
        eASSERT(m_staticIb != eNULL);
        eASSERT(m_indexCount > 0);
        eASSERT(indices != eNULL);

        *indices = m_staticIb->lock(0, 0, eLOCK_DEFAULT);
    }
}

void eGeometry::_unlockStaticBuffers()
{
    eASSERT(m_loading == eTRUE);

    m_staticVb->unlock();

    if (m_indexed)
    {
        m_staticIb->unlock();
    }
}

void eGeometry::removeInstanciation(const void* mat, eU32 passID) {
    eInt idx = this->findInstanciation(mat, passID);
    if(idx != -1) {
        eRenderJob& job = *((eRenderJob*)this->m_renderInstances[idx].job);
        job.setGeometry(eNULL);
        this->m_renderInstances.removeAt(idx);
    }
}
