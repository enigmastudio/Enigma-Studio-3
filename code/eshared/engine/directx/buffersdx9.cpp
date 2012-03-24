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

#include <d3d9.h>

#include "../../eshared.hpp"
#include "buffersdx9.hpp"

eIndexBufferDx9::eIndexBufferDx9()
{
}

eIndexBufferDx9::eIndexBufferDx9(const eIndexBufferDx9 &ib)
{
}

eIndexBufferDx9 & eIndexBufferDx9::operator = (eIndexBufferDx9 &ib)
{
    return *this;
}

eIndexBufferDx9::eIndexBufferDx9(IDirect3DDevice9 *d3dDev, eU32 count, eBool dynamic) :
    m_d3dDev(d3dDev),
    m_count(count),
    m_locked(eFALSE),
    m_dynamic(dynamic),
    m_d3dIb(eNULL)
{
    eASSERT(d3dDev != eNULL);
    eASSERT(count > 0);

    upload();
}

eIndexBufferDx9::~eIndexBufferDx9()
{
    unload();
}

eBool eIndexBufferDx9::bind()
{
    const HRESULT res = m_d3dDev->SetIndices(m_d3dIb);
    eASSERT(!FAILED(res));

    return eTRUE;
}

eBool eIndexBufferDx9::upload()
{
    eASSERT(m_d3dIb == eNULL);

    eU32 flags = D3DUSAGE_WRITEONLY;

    if (m_dynamic)
    {
        flags |= D3DUSAGE_DYNAMIC; 
    }

    HRESULT res = m_d3dDev->EvictManagedResources();
    eASSERT(!FAILED(res));
    res = m_d3dDev->CreateIndexBuffer(m_count*sizeof(eU32), flags, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_d3dIb, eNULL);
    eASSERT(!FAILED(res));

    return eTRUE;
}

eBool eIndexBufferDx9::unload()
{
    eIIndexBuffer::unload();

    eSAFE_RELEASE_COM(m_d3dIb);
    return eTRUE;
}

eU32 * eIndexBufferDx9::lock(eU32 offset, eU32 count, eBufferLock lockMode)
{
    eASSERT(m_locked == eFALSE);

    static const eU32 flags[eBUFFERLOCK_COUNT] =
    {
        0,
        D3DLOCK_DISCARD,
        D3DLOCK_NOOVERWRITE
    };

    eU32 *ptr = eNULL;

    const HRESULT res = m_d3dIb->Lock(offset*sizeof(eU32), count*sizeof(eU32), (ePtr *)&ptr, flags[lockMode]);
    eASSERT(!FAILED(res));
    m_locked = eTRUE;

    return ptr;
}

eBool eIndexBufferDx9::unlock()
{
    eASSERT(m_locked != eFALSE);

    const HRESULT res = m_d3dIb->Unlock();
    eASSERT(!FAILED(res));
    m_locked = eFALSE;

    return eTRUE;
}

eU32 eIndexBufferDx9::getCount() const
{
    return m_count;
}

eBool eIndexBufferDx9::isDynamic() const
{
    return m_dynamic;
}

eVertexBufferDx9::eVertexBufferDx9()
{
}

eVertexBufferDx9::eVertexBufferDx9(const eVertexBufferDx9 &vb)
{
}

eVertexBufferDx9 & eVertexBufferDx9::operator = (eVertexBufferDx9 &vb)
{
    return *this;
}

eVertexBufferDx9::eVertexBufferDx9(IDirect3DDevice9 *d3dDev, IDirect3DVertexDeclaration9 * const d3dVDecls[], eU32 byteSize, eBool dynamic) :
    m_d3dVb(eNULL),
    m_d3dDev(d3dDev),
    m_d3dVDecls(d3dVDecls),
    m_byteSize(byteSize),
    m_dynamic(dynamic),
    m_locked(eFALSE)
{
    eASSERT(d3dDev != eNULL);
    eASSERT(d3dVDecls != eNULL);
    eASSERT(byteSize > 0);

    upload();
}

eVertexBufferDx9::~eVertexBufferDx9()
{
    unload();
}

eBool eVertexBufferDx9::bind(eU32 index, eVertexType vertexType, eU32 byteOffset, eU32 instanceCount)
{
    const eU32 vertexSize = eVERTEX_SIZES[vertexType];

    eU32 setting = 1;
       
    if (instanceCount > 0)
    {
        if (index == 0)
        {
            setting = (D3DSTREAMSOURCE_INDEXEDDATA | instanceCount);
        }
        else
        {
            setting = (D3DSTREAMSOURCE_INSTANCEDATA | 1);
        }
    }
        
    HRESULT res = m_d3dDev->SetStreamSourceFreq(index, setting);
    eASSERT(!FAILED(res));
    res = m_d3dDev->SetStreamSource(index, m_d3dVb, byteOffset, vertexSize);
    eASSERT(!FAILED(res));

    if (vertexType != eVTXTYPE_INSTANCE)
    {
        res = m_d3dDev->SetVertexDeclaration(m_d3dVDecls[vertexType]);
        eASSERT(!FAILED(res));
    }

    return eTRUE;
}

eBool eVertexBufferDx9::upload()
{
    eASSERT(m_d3dVb == eNULL);

    eU32 flags = D3DUSAGE_WRITEONLY;

    if (m_dynamic)
    {
        flags |= D3DUSAGE_DYNAMIC;
    }

    HRESULT res = m_d3dDev->EvictManagedResources();
    eASSERT(!FAILED(res));
    res = m_d3dDev->CreateVertexBuffer(m_byteSize, flags, 0, D3DPOOL_DEFAULT, &m_d3dVb, eNULL);
    eASSERT(!FAILED(res));

    return eTRUE;
}

eBool eVertexBufferDx9::unload()
{
    eIVertexBuffer::unload();

    eSAFE_RELEASE_COM(m_d3dVb);
    return eTRUE;
}

ePtr eVertexBufferDx9::lock(eU32 byteOffset, eU32 byteCount, eBufferLock lockMode)
{
    eASSERT(m_locked == eFALSE);

    static const eU32 flags[eBUFFERLOCK_COUNT] =
    {
        0,
        D3DLOCK_DISCARD,
        D3DLOCK_NOOVERWRITE
    };

    ePtr ptr = eNULL;

    const HRESULT res = m_d3dVb->Lock(byteOffset, byteCount, &ptr, flags[lockMode]);
    eASSERT(!FAILED(res));
    m_locked = eTRUE;

    return ptr;
}

eBool eVertexBufferDx9::unlock()
{
    eASSERT(m_locked != eFALSE);

    const HRESULT res = m_d3dVb->Unlock();
    eASSERT(!FAILED(res));
    m_locked = eFALSE;

    return eTRUE;
}

eU32 eVertexBufferDx9::getByteSize() const
{
    return m_byteSize;
}

eBool eVertexBufferDx9::isDynamic() const
{
    return m_dynamic;
}