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

#ifndef COLOR_HPP
#define COLOR_HPP

struct eFloatColor
{
    eF32    r;
    eF32    g;
    eF32    b;
    eF32    a;
};

// Four channel floating point color. Channel
// components aren't clamped while doing arithmetic
// operations (+, -, *) on it.
class eColor
{
public:
    eColor() : m_red(0), m_green(0), m_blue(0), m_alpha(255)
    {
    }

    eColor(const eU32 &argb) : m_argb(argb)
    {
    }

    eColor(const eFloatColor &argb)
    {
        // Call assignment operator to copy color.
        *this = argb;
    }

    // Sets r, g, b and a to value.
    eColor(eU8 value) :
        m_red(value),
        m_green(value),
        m_blue(value),
        m_alpha(value)
    {
    }

    eColor(eU8 red, eU8 green, eU8 blue, eU8 alpha=255) :
        m_red(red),
        m_green(green),
        m_blue(blue),
        m_alpha(alpha)
    {
    }

    void set(eU8 red, eU8 green, eU8 blue)
    {
        m_red   = red;
        m_green = green;
        m_blue  = blue;
    }

    void set(eU8 red, eU8 green, eU8 blue, eU8 alpha)
    {
        set(red, green, blue);
        m_alpha = alpha;
    }

    void set(eU32 channel, eU8 color)
    {
        eASSERT(channel < 4);
        m_channels[channel] = color;
    }

    void setRed(eU8 red)
    {
        m_red = red;
    }

    void setGreen(eU8 green)
    {
        m_green = green;
    }

    void setBlue(eU8 blue)
    {
        m_blue = blue;
    }

    void setAlpha(eU8 alpha)
    {
        m_alpha = alpha;
    }

    void setRedF(eF32 red)
    {
        eASSERT(red >= 0.0f && red <= 1.0f);
        m_red = eFtoL(red*255.0f);
    }

    void setGreenF(eF32 green)
    {
        eASSERT(green >= 0.0f && green <= 1.0f);
        m_green = eFtoL(green*255.0f);
    }

    void setBlueF(eF32 blue)
    {
        eASSERT(blue >= 0.0f && blue <= 1.0f);
        m_blue = eFtoL(blue*255.0f);
    }

    void setAlphaF(eF32 alpha)
    {
        eASSERT(alpha >= 0.0f && alpha <= 1.0f);
        m_alpha = eFtoL(alpha*255.0f);
    }

    eU8 get(eU32 index) const
    {
        eASSERT(index < 4);
        return m_channels[index];
    }

    eU8 red() const
    {
        return m_red;
    }

    eU8 green() const
    {
        return m_green;
    }

    eU8 blue() const
    {
        return m_blue;
    }

    eU8 alpha() const
    {
        return m_alpha;
    }

    eF32 getF(eU32 index) const
    {
        eASSERT(index < 4);
        return (eF32)m_channels[index]/255.0f;
    }

    eF32 redF() const
    {
        return (eF32)m_red/255.0f;
    }

    eF32 greenF() const
    {
        return (eF32)m_green/255.0f;
    }

    eF32 blueF() const
    {
        return (eF32)m_blue/255.0f;
    }

    eF32 alphaF() const
    {
        return (eF32)m_alpha/255.0f;
    }

    // Returns the gray scaled color value
    // of this color.
    eU8 grayScale() const
    {
        return (m_red*11+m_green*16+m_blue*5)/32;
    }

    void toGrayScale()
    {
        const eU8 gray = grayScale();

        m_red   = gray;
        m_green = gray;
        m_blue  = gray;
    }

    eColor operator + (const eColor &c) const
    {
        static eColor res;

        res.m_red   = eMin((eInt)(m_red+c.m_red), 255);
        res.m_green = eMin((eInt)(m_green+c.m_green), 255);
        res.m_blue  = eMin((eInt)(m_blue+c.m_blue), 255);
        res.m_alpha = eMin((eInt)(m_alpha+c.m_alpha), 255);

        return res;
    }

    eColor operator - (const eColor &c) const
    {
        static eColor res;

        res.m_red   = eMax(0, (eInt)(m_red-c.m_red));
        res.m_green = eMax(0, (eInt)(m_green-c.m_green));
        res.m_blue  = eMax(0, (eInt)(m_blue-c.m_blue));
        res.m_alpha = eMax(0, (eInt)(m_alpha-c.m_alpha));

        return res;
    }

    eColor operator * (const eColor &c) const
    {
        static eColor res;

        res.m_red   = (m_red*c.m_red)/255;
        res.m_green = (m_green*c.m_green)/255;
        res.m_blue  = (m_blue*c.m_blue)/255;
        res.m_alpha = (m_alpha*c.m_alpha)/255;

        return res;
    }

    eColor operator * (eF32 s) const
    {
        eASSERT(s >= 0.0f);

        static eColor res;

        res.m_red   = eMin(eFtoL((eF32)m_red*s), 255);
        res.m_green = eMin(eFtoL((eF32)m_green*s), 255);
        res.m_blue  = eMin(eFtoL((eF32)m_blue*s), 255);
        res.m_alpha = eMin(eFtoL((eF32)m_alpha*s), 255);

        return res;
    }

    friend eColor operator * (eF32 s, const eColor &c)
    {
        return c*s;
    }

    eColor & operator += (const eColor &c)
    {
        *this = *this+c;
        return *this;
    }

    eColor & operator -= (const eColor &c)
    {
        *this = *this-c;
        return *this;
    }

    eColor & operator *= (const eColor &c)
    {
        *this = *this*c;
        return *this;
    }

    eColor & operator *= (eF32 s)
    {
        *this = *this*s;
        return *this;
    }

    // Some advanced color arithmetic operations.

    void minimum(const eColor &c)
    {
        m_red   = eMin(m_red, c.m_red);
        m_green = eMin(m_green, c.m_green);
        m_blue  = eMin(m_blue, c.m_blue);
        m_alpha = eMin(m_alpha, c.m_alpha);
    }

    void maximum(const eColor &c)
    {
        m_red   = eMax(m_red, c.m_red);
        m_green = eMax(m_green, c.m_green);
        m_blue  = eMax(m_blue, c.m_blue);
        m_alpha = eMax(m_alpha, c.m_alpha);
    }

    void average(const eColor &c)
    {
        m_red   = (m_red+c.m_red)/2;
        m_green = (m_green+c.m_green)/2;
        m_blue  = (m_blue+c.m_blue)/2;
        m_alpha = (m_alpha+c.m_alpha)/2;
    }

    void difference(const eColor &c)
    {
        m_red   = eAbs(m_red-c.m_red);
        m_green = eAbs(m_green-c.m_green);
        m_blue  = eAbs(m_blue-c.m_blue);
        m_alpha = eAbs(m_alpha-c.m_alpha);
    }

    // Linear interpolation (0 <= t <= 1).
    eColor lerp(const eColor &to, eF32 t) const
    {
        eASSERT(t >= 0.0f && t <= 1.0f);

        eColor res;

        res.m_red   = eFtoL(eLerp((eF32)m_red,   (eF32)to.m_red,   t));
        res.m_green = eFtoL(eLerp((eF32)m_green, (eF32)to.m_green, t));
        res.m_blue  = eFtoL(eLerp((eF32)m_blue,  (eF32)to.m_blue,  t));
        res.m_alpha = eFtoL(eLerp((eF32)m_alpha, (eF32)to.m_alpha, t));

        return res;
    }

    eU32 toArgb() const
    {
        return m_argb;
    }

    eColor & operator = (const eU32 &argb)
    {
        m_argb = argb;
        return *this;
    }

    eColor & operator = (const eFloatColor &col)
    {
        eASSERT(col.r <= 1.0f);
        eASSERT(col.g <= 1.0f);
        eASSERT(col.b <= 1.0f);
        eASSERT(col.a <= 1.0f);
        eASSERT(col.r >= 0.0f);
        eASSERT(col.g >= 0.0f);
        eASSERT(col.b >= 0.0f);
        eASSERT(col.a >= 0.0f);

        set(eFtoL(col.r*255.0f),
            eFtoL(col.g*255.0f),
            eFtoL(col.b*255.0f),
            eFtoL(col.a*255.0f));

        return *this;
    }

    eU8 operator [] (eInt index) const
    {
        eASSERT(index < 4);
        return m_channels[index];
    }

    operator const eF32 * () const
    {
        static eF32 flt[4];

        flt[0] = redF();
        flt[1] = greenF();
        flt[2] = blueF();
        flt[3] = alphaF();

        return flt;
    }

public:
    // Bigger, non inlinable functions.
    void toHsv(eInt &h, eInt &s, eInt &v);
    void fromHsv(eInt h, eInt s, eInt v);

public:
    static const eColor RED;
    static const eColor GREEN;
    static const eColor BLUE;
    static const eColor ORANGE;
    static const eColor YELLOW;
    static const eColor PURPLE;
    static const eColor CYAN;
    static const eColor PINK;

    static const eColor WHITE;
    static const eColor BLACK;
    static const eColor GRAY;
    static const eColor DARKGRAY;
    static const eColor LIGHTGRAY;

private:
    union
    {
        struct
        {
            eU8    m_blue;
            eU8    m_green;
            eU8    m_red;
            eU8    m_alpha;
        };

        eU32       m_argb;
        eU8        m_channels[4];
    };
};

#endif // COLOR_HPP