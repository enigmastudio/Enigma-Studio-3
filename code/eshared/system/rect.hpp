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

#ifndef RECT_HPP
#define RECT_HPP

class eIXYXY
{
public:
    union
    {
        struct
        {
            eInt    left;
            eInt    top;
            eInt    right;
            eInt    bottom;
        };

        struct
        {
            eInt    x0;
            eInt    y0;
            eInt    x1;
            eInt    y1;
        };
    };
};

// Defines the coordinates of the upper-left
// and lower-right corners of a rectangle.
class eRect : public eIXYXY
{
public:
    eRect()
    {
        set(0, 0, 0, 0);
    }

    eRect(const eIXYXY &ixyxy)
    {
        set(ixyxy.x0, ixyxy.y0, ixyxy.x1, ixyxy.y1);
    }

    eRect(eInt nx0, eInt ny0, eInt nx1, eInt ny1)
    {
        set(nx0, ny0, nx1, ny1);
    }

    eRect(const ePoint &upperLeft, const ePoint &bottomRight)
    {
        set(upperLeft.x, upperLeft.y, bottomRight.x, bottomRight.y);
    }

    void set(eInt nx0, eInt ny0, eInt nx1, eInt ny1)
    {
        x0 = nx0;
        y0 = ny0;
        x1 = nx1;
        y1 = ny1;
    }

    // Translates (moves) this rectangle
    // by the given amount.
    void translate(eInt transX, eInt transY)
    {
        x0 += transX;
        x1 += transX;
        y0 += transY;
        y1 += transY;
    }

    // Can be negative if rect isn't normalized.
    eInt getWidth() const
    {
        return x1-x0;
    }

    // Can be negative if rect isn't normalized.
    eInt getHeight() const
    {
        return y1-y0;
    }

    // Returns center (middle point) of rectangle.
    ePoint getCenter() const
    {
        return ePoint((x0+x1)/2, (y0+y1)/2);
    }

    ePoint getUpperLeft() const
    {
        return ePoint(x0, y0);
    }

    ePoint getBottomRight() const
    {
        return ePoint(x1, y1);
    }

    // Returns width and height of rectangle
    // (rectangle isn't normalized before).
    eSize getDimension() const
    {
        return eSize(getWidth(), getHeight());
    }

    // Makes sure, left < right and top < bottom
    // => width and height are not negative.
    void normalize()
    {
        if (x0 > x1)
        {
            eSwap(x0, x1);
        }

        if (y0 > y1)
        {
            eSwap(y0, y1);
        }
    }

    eBool pointInRect(const ePoint &p) const
    {
        return (p.x >= x0 &&
                p.x <= x1 &&
                p.y >= y0 &&
                p.y <= y1);
    }

    eBool rectInRect(const eRect &r) const
    {
        return (r.x0 >= x0 &&
                r.x1 <= x1 &&
                r.y0 >= y0 &&
                r.y1 <= y1);
    }

    eBool intersect(const eRect &r) const
    {
        return (pointInRect(r.getUpperLeft()) ||
                pointInRect(r.getBottomRight()));
    }

    eInt operator [] (eInt index) const
    {
        eASSERT(index < 4);
        return ((eInt *)this)[index];
    }

    eInt & operator [] (eInt index)
    {
        eASSERT(index < 4);
        return ((eInt *)this)[index];
    }

    eBool operator == (const eRect &r) const
    {
        return (r.x0 == x0 &&
                r.y0 == y0 &&
                r.x1 == x1 &&
                r.y1 == y1);
    }

    eBool operator != (const eRect &r) const
    {
        return !(*this == r);
    }
};

#endif // RECT_HPP