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

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "tshader/machinecodegen.hpp"
#include "tshader/tshader.hpp"
#include "tshader/tscompiler.hpp"

#include "iresource.hpp"
#include "directx/graphicsapidx9.hpp"
#include "material.hpp"
#include "vertex.hpp"
#include "geometry.hpp"
#include "editmesh.hpp"
#include "path.hpp"
#include "camera.hpp"
#include "renderjob.hpp"
#include "light.hpp"
#include "irenderable.hpp"
#include "scenedata.hpp"
#include "kdtree.hpp"
#include "scene.hpp"
#include "particlesys.hpp"
#include "mesh.hpp"
#include "shadermgr.hpp"
#include "resourcemgr.hpp"
#include "statemgr.hpp"
#include "irenderer.hpp"
#include "deferredrenderer.hpp"
//#include "igraphicsapi.hpp"
#include "effect.hpp"
#include "sequencer.hpp"

//#include "directx/graphicsapidx9.hpp"
#include "shaders/shaders.hpp"

class eEngine
{
public:
    eEngine();
    eEngine(eBool fullScreen, const eSize &wndSize, ePtr hwnd);
    ~eEngine();

    void            openWindow(eBool fullScreen, const eSize &wndSize, ePtr hwnd);

    eGraphicsApiDx9 * getGraphicsApi() const;
    eIRenderer *    getRenderer() const;

private:
    eGraphicsApiDx9 * m_gfx;
    eIRenderer *    m_renderer;
};

#endif // ENGINE_HPP