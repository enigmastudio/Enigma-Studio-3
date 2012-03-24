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

#ifndef IRENDERER_HPP
#define IRENDERER_HPP

class eScene;

class eIRenderer
{
public:
    enum ShadowQuality
    {
        SHADOW_QUALITY_LOW,
        SHADOW_QUALITY_HIGH,
        SHADOW_QUALITY_MEDIUM,
    };

    enum TextureResolution
    {
        TEXTURE_RES_50,
        TEXTURE_RES_100,
        TEXTURE_RES_200,
    };

public:
    eIRenderer(eGraphicsApiDx9 *gfx);
    virtual ~eIRenderer();

    virtual void            renderScene(eScene &scene, const eCamera &cam, eITexture2d *target, eF32 time) = 0;
    virtual void            renderTexturedQuad(const eRect &r, const eSize &size, eITexture2d *tex, const eVector2 &tileUv, const eVector2 &scrollUv) const = 0;
    virtual void            renderTexturedQuad(const eRect &r, const eSize &size, eITexture2d *tex) const = 0;
    virtual void            renderQuad(const eRect &r, const eSize &size, const eVector2 &tileUv, const eVector2 &scrollUv) const = 0;
    virtual void            renderQuad(const eRect &r, const eSize &size) const = 0;

    virtual eITexture2d *   getPositionMap() const = 0;
    virtual eITexture2d *   getNormalMap() const = 0;

public:
    void                    setTextureResolution(TextureResolution res);
    void                    setShadowQuality(ShadowQuality quality);
    void                    setShadowsEnabled(eBool enabled);

    TextureResolution       getTextureResolution() const;
    ShadowQuality           getShadowQuality() const;
    eBool                   getShadowsEnabled() const;

    eGraphicsApiDx9 *         getGraphicsApi() const;

protected:
    eBool                   m_shadowsEnabled;
    ShadowQuality           m_shadowQuality;
    TextureResolution       m_textureRes;

    eGraphicsApiDx9 *         m_gfx;
};

#endif // IRENDERER_HPP