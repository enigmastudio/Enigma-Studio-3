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

#ifndef CONFIG_INFO_HPP
#define CONFIG_INFO_HPP

// Current version of Enigma 3 (editor and player).
const eChar eENIGMA3_VERSION[] = "3.11b";

// Current configuration of including project.
#ifdef eDEBUG
    const eChar eENIGMA3_CONFIG[] = "Debug";
#else
    const eChar eENIGMA3_CONFIG[] = "Release";
#endif

#endif // CONFIG_INFO_HPP