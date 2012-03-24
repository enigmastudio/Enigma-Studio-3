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

#ifndef LIGHT_HPP
#define LIGHT_HPP

class eLight
{
public:
    eLight();
    eLight(const eColor &diffuse, const eColor &ambient, const eColor &specular, eF32 range, eBool castsShadows);

    void                    activate(eGraphicsApiDx9 *gfx, const eMatrix4x4 &viewMtx) const;
    eBool                   activateScissor(const eSize &viewport, const eCamera &cam) const;

    void                    setPosition(const eVector3 &pos);
    void                    setDiffuse(const eColor &diffuse);
    void                    setAmbient(const eColor &ambient);
    void                    setSpecular(const eColor &specular);
    void                    setRange(eF32 range);
    void                    setPenumbraSize(eF32 penumbraSize);
    void                    setShadowBias(eF32 shadowBias);
    void                    setCastsShadows(eCubeMapFace face, eBool castsShadows);

    const eVector3 &        getPosition() const;
    const eColor &          getDiffuse() const;
    const eColor &          getAmbient() const;
    const eColor &          getSpecular() const;
    eF32                    getRange() const;
    eF32                    getPenumbraSize() const;
    eF32                    getShadowBias() const;
    eBool                   getCastsShadows(eCubeMapFace face) const;
    eBool                   getCastsAnyShadows() const;

private:
    eVector3                m_pos;
    eColor                  m_diffuse;
    eColor                  m_ambient;
    eColor                  m_specular;
    eF32                    m_range;
    eBool                   m_castsShadows[eCMFACE_COUNT];
    eF32                    m_penumbraSize;
    eF32                    m_shadowBias;
};

typedef eArray<const class eLight *> eConstLightPtrArray;

#endif // LIGHT_HPP