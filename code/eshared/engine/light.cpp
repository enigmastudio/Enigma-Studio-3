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

eLight::eLight() :
    m_diffuse(eColor::WHITE),
    m_range(1.0f),
    m_shadowBias(0.0001f)
{
    eMemSet(m_castsShadows, eFALSE, sizeof(m_castsShadows));
}

eLight::eLight(const eColor &diffuse, const eColor &ambient, const eColor &specular, eF32 range, eBool castsShadows) :
    m_diffuse(diffuse),
    m_ambient(ambient),
    m_specular(specular),
    m_range(range),
    m_penumbraSize(2.0f),
    m_shadowBias(0.0001f)
{
    eASSERT(range > 0.0f);
    eMemSet(m_castsShadows, castsShadows, sizeof(m_castsShadows));
}

void eLight::activate(eGraphicsApiDx9 *gfx, const eMatrix4x4 &viewMtx) const
{
    eASSERT(gfx != eNULL);
    eASSERT(m_range > 0.0f);

    const eF32 invRange = 1.0f/m_range;
    const eVector3 viewPos = m_pos*viewMtx;
    const eVector3 worldPos = m_pos;

    // Set VS constants.
    gfx->setVsConst(eVSCONST_LIGHT_VIEWPOS, viewPos);
    gfx->setVsConst(eVSCONST_LIGHT_WORLDPOS, worldPos);
    gfx->setVsConst(eVSCONST_LIGHT_INVRANGE, invRange);

    // Set PS constants.
    gfx->setPsConst(ePSCONST_LIGHT_VIEWPOS, viewPos);
    gfx->setPsConst(ePSCONST_LIGHT_WORLDPOS, worldPos);
    gfx->setPsConst(ePSCONST_LIGHT_INVRANGE, invRange);
    gfx->setPsConst(ePSCONST_LIGHT_PENUMBRA, m_penumbraSize);
    gfx->setPsConst(ePSCONST_LIGHT_SHADOWBIAS, m_shadowBias);
    gfx->setPsConst(ePSCONST_LIGHT_DIFFUSE, m_diffuse);
    gfx->setPsConst(ePSCONST_LIGHT_TOTALAMBIENT, m_ambient);
    gfx->setPsConst(ePSCONST_LIGHT_SPECULAR, m_specular);
}

void eLight::setPosition(const eVector3 &pos)
{
    m_pos = pos;
}

eBool eLight::activateScissor(const eSize &viewport, const eCamera &cam) const
{
    eRect rect(0, 0, viewport.x, viewport.y);
    const eVector3 viewPos = m_pos*cam.getViewMatrix();

    // Negate z, because original code was for OpenGL
    // (right-handed coordinate system, but DirectX
    // uses a left-handed one).
    const eVector3 l(viewPos.x, viewPos.y, -viewPos.z);
    const eVector3 ll(l.x*l.x, l.y*l.y, l.z*l.z);
    const eF32 r = m_range;
    const eF32 rr = r*r;
    const eF32 e0 = 1.2f;
    const eF32 e1 = 1.2f*cam.getAspectRatio();

    eF32 d = rr*ll.x-(ll.x+ll.z)*(rr-ll.z);

    if (d >= 0.0f)
    {
        d = eSqrt(d);

        const eF32 nx0 = (r*l.x +d)/(ll.x+ll.z);
        const eF32 nx1 = (r*l.x-d)/(ll.x+ll.z);
        const eF32 nz0 = (r-nx0*l.x)/l.z;
        const eF32 nz1 = (r-nx1*l.x)/l.z;
        const eF32 pz0 = (ll.x+ll.z-rr)/(l.z-(nz0/nx0)*l.x);
        const eF32 pz1 = (ll.x+ll.z-rr)/(l.z-(nz1/nx1)*l.x);

        if (pz0 < 0.0f)
        {
            const eF32 fx = nz0*e0/nx0;
            const eInt ix = eFtoL((fx+1.0f)*(eF32)viewport.x*0.5f);
            const eF32 px = -pz0*nz0/nx0;

            if (px < l.x)
            {
                rect.x0 = eMax(rect.x0, ix);
            }
            else
            {
                rect.x1 = eMin(rect.x1, ix);
            }
        }

        if (pz1 < 0.0f)
        {
            const eF32 fx = nz1*e0/nx1;
            const eInt ix = eFtoL((fx+1.0f)*(eF32)viewport.x*0.5f);
            const eF32 px = -pz1*nz1/nx1;

            if (px < l.x)
            {
                rect.x0 = eMax(rect.x0, ix);
            }
            else
            {
                rect.x1 = eMin(rect.x1, ix);
            }
        }
    }

    d = rr*ll.y-(ll.y+ll.z)*(rr-ll.z);

    if (d >= 0.0f)
    {
        d = eSqrt(d);

        const eF32 ny0 = (r*l.y +d)/(ll.y+ll.z);
        const eF32 ny1 = (r*l.y-d)/(ll.y+ll.z);
        const eF32 nz0 = (r-ny0*l.y)/l.z;
        const eF32 nz1 = (r-ny1*l.y)/l.z;
        const eF32 pz0 = (ll.y+ll.z-rr)/(l.z-(nz0/ny0)*l.y);
        const eF32 pz1 = (ll.y+ll.z-rr)/(l.z-(nz1/ny1)*l.y);

        if (pz0 < 0.0f)
        {
            const eF32 fy = nz0*e1/ny0;
            const eInt iy = eFtoL((fy+1.0f)*(eF32)viewport.y*0.5f);
            const eF32 py = -pz0*nz0/ny0;

            if (py < l.y)
            {
                rect.y0 = eMax(rect.y0, iy);
            }
            else
            {
                rect.y1 = eMin(rect.y1, iy);
            }
        }

        if (pz1 < 0.0f)
        {
            const eF32 fy = nz1*e1/ny1;
            const eInt iy = eFtoL((fy+1.0f)*(eF32)viewport.y*0.5f);
            const eF32 py = -pz1*nz1/ny1;

            if (py < l.y)
            {
                rect.y0 = eMax(rect.y0, iy);
            }
            else
            {
                rect.y1 = eMin(rect.y1, iy);
            }
        }
    }

    // Finally check calculated rect and set if it's valid.
    const eInt n = rect.getWidth()*rect.getHeight();

    if (n <= 0)
    {
        return eFALSE;
    }

    if (n == viewport.x*viewport.y || rect.x0 > rect.x1 || rect.y0 > rect.y1)
    {
        eStateManager::setCap(eCAP_SCISSORTEST, eFALSE);
    }
    else
    {
        eStateManager::setScissorRect(eRect(rect.x0, viewport.y-rect.y1, rect.x1, viewport.y-rect.y0));
        eStateManager::setCap(eCAP_SCISSORTEST, eTRUE);
    }

    return eTRUE;
}

void eLight::setDiffuse(const eColor &diffuse)
{
    m_diffuse = diffuse;
}

void eLight::setAmbient(const eColor &ambient)
{
    m_ambient = ambient;
}

void eLight::setSpecular(const eColor &specular)
{
    m_specular = specular;
}

void eLight::setRange(eF32 range)
{
    eASSERT(range > 0.0f);
    eASSERT(!eIsFloatZero(range));

    m_range = range;
}

void eLight::setPenumbraSize(eF32 penumbraSize)
{
    eASSERT(penumbraSize >= 0.0f);
    m_penumbraSize = penumbraSize;
}

void eLight::setShadowBias(eF32 shadowBias)
{
    m_shadowBias = shadowBias;
}

void eLight::setCastsShadows(eCubeMapFace face, eBool castsShadows)
{
    m_castsShadows[face] = castsShadows;
}

const eVector3 & eLight::getPosition() const
{
    return m_pos;
}

const eColor & eLight::getDiffuse() const
{
    return m_diffuse;
}

const eColor & eLight::getAmbient() const
{
    return m_ambient;
}

const eColor & eLight::getSpecular() const
{
    return m_specular;
}

eF32 eLight::getRange() const
{
    return m_range;
}

eF32 eLight::getPenumbraSize() const
{
    return m_penumbraSize;
}

eF32 eLight::getShadowBias() const
{
    return m_shadowBias;
}

eBool eLight::getCastsShadows(eCubeMapFace face) const
{
    return m_castsShadows[face];
}

eBool eLight::getCastsAnyShadows() const
{
    for (eInt i=0; i<eCMFACE_COUNT; i++)
    {
        if (m_castsShadows[(eCubeMapFace)i])
        {
            return eTRUE;
        }
    }

    return eFALSE;
}