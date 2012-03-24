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
#include <d3dx9.h>

#include "../../eshared.hpp"
#include "texturesdx9.hpp"

static const struct FormatInfo
{
    D3DFORMAT   d3dFormat;
    eU32        pixelSize;
}
FORMAT_INFOS[] =
{
    {D3DFMT_A8R8G8B8,       4},
    {D3DFMT_A16B16G16R16,   8},
    {D3DFMT_A16B16G16R16F,  8},
    {D3DFMT_D16,            2},
    {D3DFMT_D24X8,          4},
    {D3DFMT_R16F,           2},
    {D3DFMT_R32F,           4},
    {D3DFMT_G16R16F,        4},
    {D3DFMT_G32R32F,        8}
};

eTexture2dDx9::eTexture2dDx9()
{
}

eTexture2dDx9::eTexture2dDx9(const eTexture2dDx9 &tex)
{
}

eTexture2dDx9 & eTexture2dDx9::operator = (eTexture2dDx9 &tex)
{
    return *this;
}

eTexture2dDx9::eTexture2dDx9(IDirect3DDevice9 *d3dDev, eU32 width, eU32 height, eBool renderTarget, eBool mipMapped, eBool dynamic, eFormat format) :
    m_d3dDev(d3dDev),
    m_width(width),
    m_height(height),
    m_renderTarget(renderTarget),
    m_mipMapped(mipMapped),
    m_format(format),
    m_locked(eFALSE),
    m_dynamic(dynamic),
    m_d3dTex(eNULL),
    m_d3dSurface(eNULL),
    m_data(eNULL),
    m_d3dFormat(FORMAT_INFOS[format].d3dFormat),
    m_pixelSize(FORMAT_INFOS[format].pixelSize)
{
    eASSERT(d3dDev != eNULL);
    eASSERT(width*height > 0);

    // Only create data array if texture is not a target.
    if (!renderTarget)
    {
        m_data = new eU8[width*height*m_pixelSize];
        eASSERT(m_data != eNULL);
        eMemSet(m_data, 255, width*height*m_pixelSize);
    }

    upload();
}

eTexture2dDx9::~eTexture2dDx9()
{
    unload();
    eSAFE_DELETE_ARRAY(m_data);
}

eBool eTexture2dDx9::bind(eU32 unit, eCubeMapFace face, eBool targetAsTexture)
{
    eASSERT(unit < eGraphicsApiDx9::MAX_TEX_UNITS);

    if (m_renderTarget && !targetAsTexture)
    {
        if (m_d3dFormat == D3DFMT_D16 || m_d3dFormat == D3DFMT_D24X8)
        {
            const HRESULT res = m_d3dDev->SetDepthStencilSurface(m_d3dSurface);
            eASSERT(!FAILED(res));
        }
        else
        {
            const HRESULT res = m_d3dDev->SetRenderTarget(unit, m_d3dSurface);
            eASSERT(!FAILED(res));
        }
    }
    else
    {
        const HRESULT res = m_d3dDev->SetTexture(unit, (IDirect3DBaseTexture9 *)m_d3dTex);
        eASSERT(!FAILED(res));
    }

    return eTRUE;
}

eBool eTexture2dDx9::upload()
{
    eASSERT(m_d3dTex == eNULL);

    if (m_renderTarget)
    {
        const DWORD usage = (m_d3dFormat == D3DFMT_D16 ||
                             m_d3dFormat == D3DFMT_D24X8 ? D3DUSAGE_DEPTHSTENCIL : D3DUSAGE_RENDERTARGET);

        HRESULT res = m_d3dDev->EvictManagedResources();
        eASSERT(!FAILED(res));
        res = m_d3dDev->CreateTexture(m_width, m_height, 1, usage, m_d3dFormat, D3DPOOL_DEFAULT, &m_d3dTex, eNULL);
        eASSERT(!FAILED(res));
        res = m_d3dTex->GetSurfaceLevel(0, &m_d3dSurface);
        eASSERT(!FAILED(res));
    }
    else
    {
        const eU32 usage   = (m_dynamic ? D3DUSAGE_DYNAMIC : 0) | (m_mipMapped ? D3DUSAGE_AUTOGENMIPMAP : 0);
        const eU32 levels  = (m_mipMapped ? 0 : 1);
        const D3DPOOL pool = (m_dynamic ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED);

        if (m_dynamic)
        {
            const HRESULT res = m_d3dDev->EvictManagedResources();
            eASSERT(!FAILED(res));
        }

        const HRESULT res = m_d3dDev->CreateTexture(m_width, m_height, levels, usage, m_d3dFormat, pool, &m_d3dTex, eNULL);
        eASSERT(!FAILED(res));

        // Copy data into AGP memory.
        if (!m_renderTarget)
        {
            lock();
            unlock();
        }
    }

    return eTRUE;
}

eBool eTexture2dDx9::unload()
{
    eITexture2d::unload();

    eSAFE_RELEASE_COM(m_d3dTex);
    eSAFE_RELEASE_COM(m_d3dSurface);
    return eTRUE;
}

ePtr eTexture2dDx9::lock()
{
    eASSERT(m_renderTarget == eFALSE);
    eASSERT(m_locked == eFALSE);

    m_locked = eTRUE;
    return m_data;
}

eBool eTexture2dDx9::unlock()
{
    eASSERT(m_renderTarget == eFALSE);
    eASSERT(m_locked == eTRUE);

    D3DLOCKED_RECT r;

    HRESULT res = m_d3dTex->LockRect(0, &r, eNULL, (m_dynamic ? D3DLOCK_DISCARD : 0));
    eASSERT(!FAILED(res));
    eMemCopy(r.pBits, m_data, m_width*m_height*m_pixelSize);
    res = m_d3dTex->UnlockRect(0);
    eASSERT(!FAILED(res));

    m_locked = eFALSE;
    return eTRUE;
}

#ifndef eINTRO
void eTexture2dDx9::saveToFile(const eChar *fileName)
{
    eASSERT(fileName != eNULL);
    D3DXSaveTextureToFile(fileName, D3DXIFF_BMP, m_d3dTex, eNULL);
}
#endif

eSize eTexture2dDx9::getSize() const
{
    return eSize(m_width, m_height);
}

eU32 eTexture2dDx9::getWidth() const
{
    return m_width;
}

eU32 eTexture2dDx9::getHeight() const
{
    return m_height;
}

eFormat eTexture2dDx9::getFormat() const
{
    return m_format;
}

eBool eTexture2dDx9::isDynamic() const
{
    return m_dynamic;
}

eBool eTexture2dDx9::isMipMapped() const
{
    return m_mipMapped;
}

eBool eTexture2dDx9::isRenderTarget() const
{
    return m_renderTarget;
}

// 3D texture implementation starts here.
eTexture3dDx9::eTexture3dDx9()
{
}

eTexture3dDx9::eTexture3dDx9(const eTexture3dDx9 &tex)
{
}

eTexture3dDx9 & eTexture3dDx9::operator = (eTexture3dDx9 &tex)
{
    return *this;
}

eTexture3dDx9::eTexture3dDx9(IDirect3DDevice9 *d3dDev, eU32 width, eU32 height, eU32 depth, eBool mipMapped, eBool dynamic, eFormat format) :
    m_d3dDev(d3dDev),
    m_width(width),
    m_height(height),
    m_depth(depth),
    m_mipMapped(mipMapped),
    m_format(format),
    m_locked(eFALSE),
    m_dynamic(dynamic),
    m_d3dTex(eNULL),
    m_data(eNULL),
    m_d3dFormat(FORMAT_INFOS[format].d3dFormat),
    m_pixelSize(FORMAT_INFOS[format].pixelSize)
{
    eASSERT(d3dDev != eNULL);
    eASSERT(format != eFORMAT_DEPTH16 && format != eFORMAT_DEPTH24X8);
    eASSERT(width*height*depth > 0);

    const eU32 byteSize = width*height*depth*m_pixelSize;
    m_data = new eU8[byteSize];
    eASSERT(m_data != eNULL);
    eMemSet(m_data, 255, byteSize);

    upload();
}

eTexture3dDx9::~eTexture3dDx9()
{
    unload();
    eSAFE_DELETE_ARRAY(m_data);
}

eBool eTexture3dDx9::bind(eU32 unit, eCubeMapFace face, eBool targetAsTexture)
{
    eASSERT(unit < eGraphicsApiDx9::MAX_TEX_UNITS);

    const HRESULT res = m_d3dDev->SetTexture(unit, (IDirect3DBaseTexture9 *)m_d3dTex);
    eASSERT(!FAILED(res));
    
    return eTRUE;
}

eBool eTexture3dDx9::upload()
{
    eASSERT(m_d3dTex == eNULL);

    const eU32 usage   = (m_dynamic ? D3DUSAGE_DYNAMIC : 0);
    const D3DPOOL pool = (m_dynamic ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED);
    const eU32 levels  = (m_mipMapped ? 0 : 1);

    if (m_dynamic)
    {
        HRESULT res = m_d3dDev->EvictManagedResources();
        eASSERT(!FAILED(res));
    }

    const HRESULT res = m_d3dDev->CreateVolumeTexture(m_width, m_height, m_depth, levels, usage,
                                                      m_d3dFormat, pool, &m_d3dTex, eNULL);
    eASSERT(!FAILED(res));

    lock();
    unlock();
    return eTRUE;
}

eBool eTexture3dDx9::unload()
{
    eITexture3d::unload();

    eSAFE_RELEASE_COM(m_d3dTex);
    return eTRUE;
}

ePtr eTexture3dDx9::lock()
{
    eASSERT(m_locked == eFALSE);

    m_locked = eTRUE;
    return m_data;
}

eBool eTexture3dDx9::unlock()
{
    eASSERT(m_locked == eTRUE);

    D3DLOCKED_BOX box;

    HRESULT res = m_d3dTex->LockBox(0, &box, eNULL, (m_dynamic ? D3DLOCK_DISCARD : 0));
    eASSERT(!FAILED(res));
    eMemCopy(box.pBits, m_data, m_width*m_height*m_depth*m_pixelSize);
    res = m_d3dTex->UnlockBox(0);
    eASSERT(!FAILED(res));

    m_locked = eFALSE;
    return eTRUE;
}

eU32 eTexture3dDx9::getWidth() const
{
    return m_width;
}

eU32 eTexture3dDx9::getHeight() const
{
    return m_height;
}

eU32 eTexture3dDx9::getDepth() const
{
    return m_depth;
}

eFormat eTexture3dDx9::getFormat() const
{
    return m_format;
}

eBool eTexture3dDx9::isDynamic() const
{
    return m_dynamic;
}

eBool eTexture3dDx9::isMipMapped() const
{
    return m_mipMapped;
}

// Cube texture implementation starts here.
eTextureCubeDx9::eTextureCubeDx9()
{
}

eTextureCubeDx9::eTextureCubeDx9(const eTextureCubeDx9 &tex)
{
}

eTextureCubeDx9 & eTextureCubeDx9::operator = (eTextureCubeDx9 &tex)
{
    return *this;
}

eTextureCubeDx9::eTextureCubeDx9(IDirect3DDevice9 *d3dDev, eU32 width, eBool renderTarget, eBool mipMapped, eBool dynamic, eFormat format) :
    m_d3dDev(d3dDev),
    m_width(width),
    m_mipMapped(mipMapped),
    m_format(format),
    m_d3dTex(eNULL),
    m_renderTarget(renderTarget),
    m_dynamic(dynamic),
    m_locked(eFALSE),
    m_lockedFace(eCMFACE_POSX),
    m_d3dFormat(FORMAT_INFOS[format].d3dFormat),
    m_pixelSize(FORMAT_INFOS[format].pixelSize)
{
    eASSERT(d3dDev != eNULL);
    eASSERT(width > 0);

    if (!renderTarget)
    {
        const eU32 byteSize = width*width*m_pixelSize;

        for (eInt i=0; i<eCMFACE_COUNT; i++)
        {
            m_data[i] = new eU8[byteSize];
            eASSERT(m_data[i] != eNULL);
            eMemSet(m_data[i], 255, byteSize);
        }
    }
    else
    {
        eMemSet(m_data, 0, sizeof(m_data));
    }

    upload();
}

eTextureCubeDx9::~eTextureCubeDx9()
{
    unload();

    for (eInt i=0; i<eCMFACE_COUNT; i++)
    {
        eSAFE_DELETE_ARRAY(m_data[i]);
    }
}

eBool eTextureCubeDx9::bind(eU32 unit, eCubeMapFace face, eBool targetAsTexture)
{
    eASSERT(unit < eGraphicsApiDx9::MAX_TEX_UNITS);

    if (!targetAsTexture)
    {
        IDirect3DSurface9 *surface = eNULL;
        HRESULT res = m_d3dTex->GetCubeMapSurface((D3DCUBEMAP_FACES)face, 0, &surface);
        eASSERT(!FAILED(res));
        res = m_d3dDev->SetRenderTarget(unit, surface);
        eASSERT(!FAILED(res));
        eSAFE_RELEASE_COM(surface);
    }
    else
    {
        const HRESULT res = m_d3dDev->SetTexture(unit, (IDirect3DBaseTexture9 *)m_d3dTex);
        eASSERT(!FAILED(res));
    }

    return eTRUE;
}

eBool eTextureCubeDx9::upload()
{
    eASSERT(m_d3dTex == eNULL);

    const D3DPOOL pool = (m_dynamic ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED);
    const eU32 levels  = (m_mipMapped ? 0 : 1);
    const eU32 usage   = (m_dynamic ? D3DUSAGE_DYNAMIC : 0) |
                         (m_mipMapped ? D3DUSAGE_AUTOGENMIPMAP : 0) |
                         (m_renderTarget ? D3DUSAGE_RENDERTARGET : 0);

    if (m_dynamic)
    {
        const HRESULT res = m_d3dDev->EvictManagedResources();
        eASSERT(!FAILED(res));
    }

    const HRESULT res = m_d3dDev->CreateCubeTexture(m_width, levels, usage, m_d3dFormat, pool, &m_d3dTex, eNULL);
    eASSERT(!FAILED(res));

    // Copy data into AGP memory.
    if (!m_renderTarget)
    {
        for (eInt i=0; i<eCMFACE_COUNT; i++)
        {
            lock((eCubeMapFace)i);
            unlock();
        }
    }

    return eTRUE;
}

eBool eTextureCubeDx9::unload()
{
    eITextureCube::unload();

    eSAFE_RELEASE_COM(m_d3dTex);
    return eTRUE;
}

#ifndef eINTRO
void eTextureCubeDx9::saveToFile(const eChar *fileName)
{
    eASSERT(fileName != eNULL);
    D3DXSaveTextureToFile(fileName, D3DXIFF_DDS, m_d3dTex, eNULL);
}
#endif

eSize eTextureCubeDx9::getSize() const
{
    return eSize(m_width, m_width);
}

eU32 eTextureCubeDx9::getWidth() const
{
    return m_width;
}

eU32 eTextureCubeDx9::getHeight() const
{
    return m_width;
}

eFormat eTextureCubeDx9::getFormat() const
{
    return m_format;
}

eBool eTextureCubeDx9::isDynamic() const
{
    return eFALSE;
}

eBool eTextureCubeDx9::isMipMapped() const
{
    return eFALSE;
}

eBool eTextureCubeDx9::isRenderTarget() const
{
    return m_renderTarget;
}

ePtr eTextureCubeDx9::lock(eCubeMapFace face)
{
    eASSERT(m_locked == eFALSE);

    m_locked = eTRUE;
    m_lockedFace = face;

    return m_data[face];
}

eBool eTextureCubeDx9::unlock()
{
    eASSERT(m_renderTarget == eFALSE);
    eASSERT(m_locked == eTRUE);

    D3DLOCKED_RECT r;

    HRESULT res = m_d3dTex->LockRect((D3DCUBEMAP_FACES)m_lockedFace, 0, &r, NULL, (m_dynamic ? D3DLOCK_DISCARD : 0));
    eASSERT(!FAILED(res));
    eMemCopy(r.pBits, m_data[m_lockedFace], m_width*m_width*m_pixelSize);
    res = m_d3dTex->UnlockRect((D3DCUBEMAP_FACES)m_lockedFace, 0);
    eASSERT(!FAILED(res));

    m_locked = eFALSE;
    return eTRUE;
}