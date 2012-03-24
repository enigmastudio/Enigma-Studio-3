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

#ifndef BUFFERS_DX9_HPP
#define BUFFERS_DX9_HPP

class eIndexBufferDx9 : public eIIndexBuffer
{
private:
    eIndexBufferDx9();
    eIndexBufferDx9(const eIndexBufferDx9 &ib);
    eIndexBufferDx9 & operator = (eIndexBufferDx9 &ib);

public:
    eIndexBufferDx9(IDirect3DDevice9 *d3dDev, eU32 count, eBool dynamic);
    virtual ~eIndexBufferDx9();

    virtual eBool                           bind();
    virtual eBool                           upload();
    virtual eBool                           unload();
    virtual eU32 *                          lock(eU32 offset, eU32 count, eBufferLock lockMode);
    virtual eBool                           unlock();

    virtual eU32                            getCount() const;
    virtual eBool                           isDynamic() const;

private:
    IDirect3DIndexBuffer9 *                 m_d3dIb;
    IDirect3DDevice9 *                      m_d3dDev;
    eU32                                    m_count;
    eBool                                   m_dynamic;
    eBool                                   m_locked;
};

class eVertexBufferDx9 : public eIVertexBuffer
{
private:
    eVertexBufferDx9();
    eVertexBufferDx9(const eVertexBufferDx9 &vb);
    eVertexBufferDx9 & operator = (eVertexBufferDx9 &vb);

public:
    eVertexBufferDx9(IDirect3DDevice9 *d3dDev, IDirect3DVertexDeclaration9 * const d3dVDecls[], eU32 byteSize, eBool dynamic);
    virtual ~eVertexBufferDx9();

    virtual eBool                           bind(eU32 index, eVertexType vertexType, eU32 byteOffset, eU32 instanceCount);
    virtual eBool                           upload();
    virtual eBool                           unload();
    virtual ePtr                            lock(eU32 byteOffset, eU32 byteCount, eBufferLock lockMode);
    virtual eBool                           unlock();

    virtual eU32                            getByteSize() const;
    virtual eBool                           isDynamic() const;

private:
    IDirect3DVertexBuffer9 *                m_d3dVb;
    IDirect3DDevice9 *                      m_d3dDev;
    IDirect3DVertexDeclaration9 * const *   m_d3dVDecls;

    eU32                                    m_byteSize;
    eBool                                   m_dynamic;
    eBool                                   m_locked;
};

#endif // BUFFERS_DX9_HPP