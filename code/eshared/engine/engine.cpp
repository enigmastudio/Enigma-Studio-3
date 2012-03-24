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

eEngine::eEngine() :
    m_gfx(new eGraphicsApiDx9)
{
    
    eASSERT(m_gfx != eNULL);
    m_gfx->initialize();
}

eEngine::eEngine(eBool fullScreen, const eSize &wndSize, ePtr hwnd) :
    m_gfx(new eGraphicsApiDx9)
{
    eASSERT(m_gfx != eNULL);

    if (m_gfx->initialize())
    {
        openWindow(fullScreen, wndSize, hwnd);
    }
}

eEngine::~eEngine()
{
    eSAFE_DELETE(m_renderer);

    eGeometry::shutdown();
    eIEffect::shutdown();
    eMaterial::shutdown();
    eShaderManager::shutdown();

    eSAFE_DELETE(m_gfx);
}

void eEngine::openWindow(eBool fullScreen, const eSize &wndSize, ePtr hwnd)
{
    eASSERT(m_gfx != eNULL);

    if (m_gfx->openWindow(wndSize.width, wndSize.height, fullScreen, eFALSE, hwnd))
    {
        eShaderManager::initialize(m_gfx);
        eMaterial::initialize();
        eGeometry::initialize(m_gfx);

        m_renderer = new eDeferredRenderer(m_gfx);
        eASSERT(m_renderer != eNULL);
    }
}

eGraphicsApiDx9 * eEngine::getGraphicsApi() const
{
    return m_gfx;
}

eIRenderer * eEngine::getRenderer() const
{
    return m_renderer;
}