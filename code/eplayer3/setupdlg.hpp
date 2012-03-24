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

#ifndef SETUP_DLG_HPP
#define SETUP_DLG_HPP

struct eSetup
{
    eSetup(eU32 resWidth, eU32 resHeight, eBool fullScr) :
        fullScreen(fullScr)
    {
        res.width = resWidth;
        res.height = resHeight;
    }

    eSize   res;
    eBool   fullScreen;
};

// Returns false, if config dialog was canceled,
// else true is returned (=> start demo).
eBool eShowSetupDialog(eSetup &setup, const eEngine &engine);

#endif // SETUP_DLG_HPP