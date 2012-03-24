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

eIRenderer::eIRenderer(eGraphicsApiDx9 *gfx) :
    m_gfx(gfx)
{
    eASSERT(gfx != eNULL);
}

eIRenderer::~eIRenderer()
{
}

void eIRenderer::setTextureResolution(TextureResolution res)
{
    m_textureRes = res;
}

void eIRenderer::setShadowQuality(ShadowQuality quality)
{
    m_shadowQuality = quality;
}

void eIRenderer::setShadowsEnabled(eBool enabled)
{
    m_shadowsEnabled = enabled;
}

eIRenderer::TextureResolution eIRenderer::getTextureResolution() const
{
    return m_textureRes;
}

eIRenderer::ShadowQuality eIRenderer::getShadowQuality() const
{
    return m_shadowQuality;
}

eBool eIRenderer::getShadowsEnabled() const
{
    return m_shadowsEnabled;
}

eGraphicsApiDx9 * eIRenderer::getGraphicsApi() const
{
    return m_gfx;
}