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

#ifndef TEXTURES_DX9_HPP
#define TEXTURES_DX9_HPP

class eTexture2dDx9 : public eITexture2d
{
private:
    eTexture2dDx9();
    eTexture2dDx9(const eTexture2dDx9 &tex);
    eTexture2dDx9 & operator = (eTexture2dDx9 &tex);

public:
    eTexture2dDx9(IDirect3DDevice9 *d3dDev, eU32 width, eU32 height, eBool renderTarget, eBool mipMapped, eBool dynamic, eFormat format);
    virtual ~eTexture2dDx9();

    virtual eBool               bind(eU32 unit, eCubeMapFace face, eBool targetAsTexture=eFALSE);
    virtual eBool               upload();
    virtual eBool               unload();
    virtual ePtr                lock();
    virtual eBool               unlock();

    IDirect3DSurface9 *         getSurface();

#ifndef eINTRO
    virtual void                saveToFile(const eChar *fileName);
#endif

    virtual eSize               getSize() const;
    virtual eU32                getWidth() const;
    virtual eU32                getHeight() const;
    virtual eFormat             getFormat() const;
    virtual eBool               isDynamic() const;
    virtual eBool               isMipMapped() const;
    virtual eBool               isRenderTarget() const;

private:
    IDirect3DTexture9 *         m_d3dTex;
    IDirect3DSurface9 *         m_d3dSurface;
    IDirect3DDevice9 *          m_d3dDev;
    D3DFORMAT                   m_d3dFormat;
    eU32                        m_width;
    eU32                        m_height;
    eU32                        m_pixelSize;
    eBool                       m_renderTarget;
    eBool                       m_mipMapped;
    eBool                       m_locked;
    eBool                       m_dynamic;
    eFormat                     m_format;
    ePtr                        m_data;
};

class eTexture3dDx9 : public eITexture3d
{
private:
    eTexture3dDx9();
    eTexture3dDx9(const eTexture3dDx9 &tex);
    eTexture3dDx9 & operator = (eTexture3dDx9 &tex);

public:
    eTexture3dDx9(IDirect3DDevice9 *d3dDev, eU32 width, eU32 height, eU32 depth, eBool mipMapped, eBool dynamic, eFormat format);
    virtual ~eTexture3dDx9();

    virtual eBool               bind(eU32 unit, eCubeMapFace face, eBool targetAsTexture=eFALSE);
    virtual eBool               upload();
    virtual eBool               unload();
    virtual ePtr                lock();
    virtual eBool               unlock();

    virtual eU32                getWidth() const;
    virtual eU32                getHeight() const;
    virtual eU32                getDepth() const;
    virtual eFormat             getFormat() const;
    virtual eBool               isDynamic() const;
    virtual eBool               isMipMapped() const;

private:
    IDirect3DVolumeTexture9 *   m_d3dTex;
    IDirect3DDevice9 *          m_d3dDev;
    D3DFORMAT                   m_d3dFormat;
    eU32                        m_width;
    eU32                        m_height;
    eU32                        m_depth;
    eU32                        m_pixelSize;
    eBool                       m_mipMapped;
    eBool                       m_locked;
    eBool                       m_dynamic;
    eFormat                     m_format;
    ePtr                        m_data;
};

class eTextureCubeDx9 : public eITextureCube
{
private:
    eTextureCubeDx9();
    eTextureCubeDx9(const eTextureCubeDx9 &tex);
    eTextureCubeDx9 & operator = (eTextureCubeDx9 &tex);

public:
    eTextureCubeDx9(IDirect3DDevice9 *d3dDev, eU32 width, eBool renderTarget, eBool mipMapped, eBool dynamic, eFormat format);
    virtual ~eTextureCubeDx9();

    virtual eBool               bind(eU32 unit, eCubeMapFace face, eBool targetAsTexture=eFALSE);
    virtual eBool               upload();
    virtual eBool               unload();
    virtual ePtr                lock(eCubeMapFace face);
    virtual eBool               unlock();

#ifndef eINTRO
    virtual void                saveToFile(const eChar *fileName);
#endif

    virtual eSize               getSize() const;
    virtual eU32                getWidth() const;
    virtual eU32                getHeight() const;
    virtual eFormat             getFormat() const;
    virtual eBool               isDynamic() const;
    virtual eBool               isMipMapped() const;
    virtual eBool               isRenderTarget() const;

private:
    IDirect3DCubeTexture9 *     m_d3dTex;
    IDirect3DDevice9 *          m_d3dDev;
    D3DFORMAT                   m_d3dFormat;
    eU32                        m_width;
    eU32                        m_pixelSize;
    eBool                       m_mipMapped;
    eFormat                     m_format;
    ePtr                        m_data[eCMFACE_COUNT];
    eBool                       m_renderTarget;
    eCubeMapFace                m_lockedFace;
    eBool                       m_dynamic;
    eBool                       m_locked;
};

#endif // TEXTURES_DX9_HPP