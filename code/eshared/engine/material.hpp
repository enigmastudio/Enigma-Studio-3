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

#ifndef MATERIAL_HPP
#define MATERIAL_HPP

class eMaterial 
{
public:
    enum TextureUnit
    {
        UNIT_DIFFUSE,
        UNIT_NORMAL,
        UNIT_SPECULAR,
        UNIT_ENVIRONMENT,
        UNIT_DEPTH,

        UNIT_COUNT,
    };

public:
    eMaterial();

    void                        activate(eGraphicsApiDx9 *gfx, eBool allowBlending=eTRUE) const;
    eU32                        getSortKey() const;

    void                        setRenderPass(eU32 renderPass);
    void                        setTexture(TextureUnit unit, eITexture *tex);
    void                        setTextureFilter(TextureUnit unit, eTextureFilter texFilter);
    void                        setTextureAddressMode(TextureUnit unit, eTextureAddressMode texAddrMode);
    void                        setCullingMode(eCullingMode cullingMode);
    void                        setBlending(eBlendMode src, eBlendMode dst, eBlendOp op);
    void                        setDiffuseColor(const eColor &diffuseCol);
    void                        setSpecularColor(const eColor &specularCol);
    void                        setShininess(eF32 shininess);
    void                        setRefractionColor(const eColor &refractionCol);
    void                        setRefractionIntensity(eF32 intensity);
    void                        setUseBlending(eBool useBlending);
    void                        setUseRefraction(eBool useRefraction);
    void                        setFlatShaded(eBool fladShaded);
    void                        setZBuffer(eBool zBuffer);
    void                        setZFunction(eZFunction zFunc);
    void                        setZMask(eBool zMask);
    void                        setLighted(eBool lighted);
    void                        setPointSize(eF32 pointLineSize);

    eU32                        getRenderPass() const;
    eITexture *                 getTexture(TextureUnit unit) const;
    eTextureFilter              getTextureFilter(TextureUnit unit) const;
    eTextureAddressMode         getTextureAddressMode(TextureUnit unit) const;
    eCullingMode                getCullingMode() const;
    void                        getBlending(eBlendMode &src, eBlendMode &dst, eBlendOp &op) const;
    const eColor &              getDiffuseColor() const;
    const eColor &              getSpecularColor() const;
    eF32                        getShininess() const;
    const eColor &              getRefractionColor() const;
    eF32                        getRefractionIntensity() const;
    eBool                       getUseBlending() const;
    eBool                       getUseRefraction() const;
    eBool                       getFlatShaded() const;
    eBool                       getZBuffer() const;
    eZFunction                  getZFunction() const;
    eBool                       getZMask() const;
    eBool                       getLighted() const;
    eF32                        getPointSize() const;

public:
    static void                 initialize();
    static void                 shutdown();

    static void                 setDefault(const eMaterial &mat);
    static const eMaterial *    getDefault();
    static const eMaterial *    getWireframe();

private:
    static void                 _createWhiteTexture(eGraphicsApiDx9 *gfx);
    static void                 _createNormalMap(eGraphicsApiDx9 *gfx);

private:
    static eITexture2d *        m_whiteTex;
    static eITexture2d *        m_normalMap;
    static eMaterial            m_defaultMat;
    static eMaterial            m_wireframeMat;

private:
    friend class eMaterialOp; // direct access for smaller initialization code

    eITexture *                 m_textures[UNIT_COUNT];
    eTextureFilter              m_texFilters[UNIT_COUNT];
    eTextureAddressMode         m_texAddrModes[UNIT_COUNT];
    eCullingMode                m_cullingMode;
    eBlendMode                  m_blendSrc;
    eBlendMode                  m_blendDst;
    eBlendOp                    m_blendOp;
    eBool                       m_useBlending;
    eBool                       m_useRefraction;
    eBool                       m_flat;
    eBool                       m_zBuffer;
    eZFunction                  m_zFunc;
    eBool                       m_zMask;
    eBool                       m_lighted;
    eU32                        m_renderPass;
    eColor                      m_diffuseCol;
    eColor                      m_specularCol;
    eF32                        m_shininess;
    eColor                      m_refractionCol;
    eF32                        m_refracIntensity;
    eF32                        m_pointLineSize;
public:
    mutable eU32                m_assignment_pass_id;    // used for deferred rendering
    mutable eU32                m_render_material_id;    // used for deferred rendering
};

#endif // MATERIAL_HPP