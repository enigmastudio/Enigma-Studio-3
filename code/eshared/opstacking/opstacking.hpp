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

#ifndef OP_STACKING_HPP
#define OP_STACKING_HPP

// Needed for defines which specifies which operators
// should be compiled in and which not.
#ifdef ePLAYER
#include "../../eplayer3/production.hpp"
#endif

#include "demo.hpp"
#include "opinput.hpp"
#include "parameter.hpp"
#include "ioperator.hpp"
#include "demoscript.hpp"
#include "opmacros.hpp"
#include "bitmapops.hpp"
#include "meshops.hpp"
#include "modelops.hpp"
#include "sequencerops.hpp"
#include "effectops.hpp"
#include "miscops.hpp"
#include "pathops.hpp"
#include "oppage.hpp"
#include "demodata.hpp"

class eOpStacking
{
public:
    static void initialize()
    {
    }

#ifdef eEDITOR
    static void shutdown()
    {
        eDemoData::free();
        eDemo::shutdown();
    }
#endif
};

#endif // OP_STACKING_HPP