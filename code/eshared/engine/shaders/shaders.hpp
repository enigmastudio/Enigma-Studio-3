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

#ifndef SHADERS_HPP
#define SHADERS_HPP

#ifdef eRELEASE
// Required for following headers which are
// generated automatically by shader assembler.
#ifndef BYTE
typedef unsigned char BYTE;
#endif

#include "particles.ps.hpp"
#include "particles.vs.hpp"
#include "instanced_geo.vs.hpp"
#include "refraction.ps.hpp"
#include "forward_light.ps.hpp"
#include "nolight.ps.hpp"
#include "nolight.vs.hpp"
#include "deferred_geo.ps.hpp"
#include "deferred_ambient.ps.hpp"
#include "deferred_light.ps.hpp"
#include "deferred_env.ps.hpp"
#include "distance.ps.hpp"
#include "distance.vs.hpp"
#include "shadow.vs.hpp"
#include "shadow.ps.hpp"
#include "fx_adjust.ps.hpp"
#include "fx_blur.ps.hpp"
#include "fx_ripple.ps.hpp"
#include "fx_fog.ps.hpp"
#include "fx_distort.ps.hpp"
#include "fx_dof.ps.hpp"
#include "fx_ssao.ps.hpp"
#include "fx_fxaa.ps.hpp"
#include "fx_merge.ps.hpp"
#include "fx_radialblur.ps.hpp"
#include "fx_colorgrading.ps.hpp"
#include "quad.ps.hpp"
#include "quad.vs.hpp"
#endif

#endif // SHADERS_HPP