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

#include "system.hpp"
#include "color.hpp"

#include "../math/math.hpp"

// Initialize static members.
const eColor eColor::RED(255, 0, 0);
const eColor eColor::GREEN(0, 255, 0);
const eColor eColor::BLUE(0, 0, 255);
const eColor eColor::ORANGE(255, 128, 0);
const eColor eColor::YELLOW(255, 255, 0);
const eColor eColor::PURPLE(255, 0, 255);
const eColor eColor::CYAN(0, 255, 255);
const eColor eColor::PINK(255, 0, 128);
const eColor eColor::WHITE(255, 255, 255);
const eColor eColor::BLACK(0, 0, 0);
const eColor eColor::GRAY(128, 128, 128);
const eColor eColor::DARKGRAY(64, 64, 64);
const eColor eColor::LIGHTGRAY(204, 204, 204);

// Converts from RGB to HSV color space. The hue
// defines the color. Its range is 0..359 if the
// color is chromatic and -1 if the color is
// achromatic. The saturation and value both vary
// between 0 and 255 inclusive.

void eColor::toHsv(eInt &h, eInt &s, eInt &v)
{
    const eInt r = red();
    const eInt g = green();
    const eInt b = blue();

    // Find maximum channel.
    eInt max = r;
    eInt whatMax = 0; // r => 0, g => 1, b => 2

    if (g > max)
    {
        max = g;
        whatMax = 1;
    }

    if (b > max)
    {
        max = b;
        whatMax = 2;
    }

    // Find minimum channel.
    const eInt min = eMin(r, eMin(g, b));
    const eInt delta = max-min;

    // Calculate HSV values.
    v = max;
    s = max ? (510*delta+max)/(2*max) : 0;

    if (s == 0)
    {
        h = -1; // Hue is undefined.
    }
    else
    {
        switch ( whatMax )
        {
            case 0: // Red is max component.
            {
                if (g >= b)
                {
                    h = (120*(g-b)+delta)/(2*delta);
                }
                else
                {
                    h = (120*(g-b+delta)+delta)/(2*delta)+300;
                }

                break;
            }

            case 1: // Green is max component.
            {
                if (b > r)
                {
                    h = 120+(120*(b-r)+delta)/(2*delta);
                }
                else
                {
                    h = 60+(120*(b-r+delta)+delta)/(2*delta);
                }

                break;
            }

            case 2: // Blue is max component.
            {
                if (r > g)
                {
                    h = 240+(120*(r-g)+delta)/(2*delta);
                }
                else
                {
                    h = 180+(120*(r-g+delta)+delta)/(2*delta);
                }

                break;
            }
        }
    }
}

void eColor::fromHsv(eInt h, eInt s, eInt v)
{
    if (h < -1 || (eU32)s > 255 || (eU32)v > 255)
    {
        return;
    }

    if (s == 0 || h == -1)
    {
        // Ignore achromatic case.
        set(v, v, v);
    }
    else
    {
        // Much more complicated chromatic case.
        if ((eU32)h >= 360)
        {
            h %= 360;
        }

        const eU32 f = h%60;
        h /= 60;
        const eU32 p = (eU32)(2*v*(255-s)+255)/510;

        if (h&1)
        {
            const eU32 q = (eU32)(2*v*(15300-s*f)+15300)/30600;

            switch (h)
            {
                case 1:
                {
                    set(q, v, p);
                    break;
                }

                case 3:
                {
                    set(p, q, v);
                    break;
                }

                case 5:
                {
                    set(v, p, q);
                    break;
                }
            }
        }
        else
        {
            const eU32 t = (eU32)(2*v*(15300-(s*(60-f)))+15300)/30600;

            switch (h)
            {
                case 0:
                {
                    set(v, t, p);
                    break;
                }

                case 2:
                {
                    set(p, v, t);
                    break;
                }

                case 4:
                {
                    set(t, p, v);
                    break;
                }
            }
        }
    }
}