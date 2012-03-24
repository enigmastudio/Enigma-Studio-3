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

#ifndef DEFERRED_RENDERER_HPP
#define DEFERRED_RENDERER_HPP

class eDeferredRenderer : public eIRenderer
{
public:
    eDeferredRenderer(eGraphicsApiDx9 *gfx, eU32 shadowSize=256, eU32 refractionSize=256);
    virtual ~eDeferredRenderer();

    virtual void            renderScene(eScene &scene, const eCamera &cam, eITexture2d *target, eF32 time);
    virtual void            renderTexturedQuad(const eRect &r, const eSize &size, eITexture2d *tex, const eVector2 &tileUv, const eVector2 &scrollUv) const;
    virtual void            renderTexturedQuad(const eRect &r, const eSize &size, eITexture2d *tex) const;
    virtual void            renderQuad(const eRect &r, const eSize &size, const eVector2 &tileUv, const eVector2 &scrollUv) const;
    virtual void            renderQuad(const eRect &r, const eSize &size) const;

    virtual eITexture2d *   getPositionMap() const;
    virtual eITexture2d *   getNormalMap() const;

private:
    void                    _setupTargets(eU32 width, eU32 height);
    void                    _freeTargets();
    void                    _loadShaders();

    void                    _renderGeometryPass(const eCamera &cam);
    void                    _renderAmbientPass(const eScene &scene, const eRect &area);
    void                    _renderAlphaLightPass(const eScene &scene, const eCamera &cam, const eRect &area);
    void                    _renderNoLightPass(const eCamera &cam);
    void                    _renderLightPass(eScene &scene, const eCamera &cam, const eRect &area);
    void                    _renderLightDistance(eScene &scene, const eLight &light);
    void                    _renderShadowMap(const eCamera &cam, const eLight &light);
    void                    _renderRefractionPass(const eCamera &cam, const eRect &area, eITexture2d *target);
    void                    _renderEnvPass(const eRect &area);

    void                    _createIndirectionCubeMap();
    void                    _assignMaterialIds();

    void                    _renderRenderJobs(const eRenderJobPtrArray &jobs, const eCamera &cam, eInt renderWhat, eInt renderFlags=0);
    void                    _visualizeGeometryBuffer(const eRect &area) const;

private:
    typedef eHashMap<const eMaterial *, eU32> MaterialIdHashMap;

private:
    // Shaders for deferred lighting.
    eIPixelShader *         m_psRefraction;
    eIPixelShader *         m_psForwLight;
    eIPixelShader *         m_psNoLight;
    eIPixelShader *         m_psDefAmbient;
    eIPixelShader *         m_psDefEnv;
    eIPixelShader *         m_psDefGeo;
    eIPixelShader *         m_psDefLight;
    eIPixelShader *         m_psDistance;
    eIPixelShader *         m_psShadow;
    eIPixelShader *         m_psQuad;
    eIPixelShader *         m_psParticles;

    eIVertexShader *        m_vsNoLight;
    eIVertexShader *        m_vsInstGeo;
    eIVertexShader *        m_vsDistance;
    eIVertexShader *        m_vsShadow;
    eIVertexShader *        m_vsQuad;
    eIVertexShader *        m_vsParticles;

    // Render targets for deferred lighting.
    eITexture2d *           m_diffuseRt;
    eITexture2d *           m_normalsRt;
    eITexture2d *           m_specularRt;
    eITexture2d *           m_positionRt;

    // Render targets for shadow mapping.
    eITexture2d *           m_colRt;

    eITexture2d *           m_deferredShadowMap;
    eITexture2d *           m_unwrappedDistMap;
    eITextureCube *         m_indirectionCm;
    eU32                    m_shadowSize;

    // Render target for refraction
    eITexture2d *           m_refractionTex;
    eU32                    m_refractionSize;

    // Render jobs of currently rendered container.
    eRenderJobPtrArray          m_allJobs;
    eArray<const eMaterial*>    m_materialArray;
//    MaterialIdHashMap       m_materialHm;
};

#endif // DEFERRED_RENDERER_HPP