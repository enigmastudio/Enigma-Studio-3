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

#ifndef STRING_HPP
#define STRING_HPP

// Simple string class.
class eString
{
public:
    eString();
    eString(const eChar *str);
    eString(const eChar *str, eU32 length);
    eString(const eString &str);

    eU32            length() const;
    eBool           equals(const eString &str, eU32 count) const;

    void            makeUpper();
    eBool           split(eChar token, eString &left, eString &right) const;
    eString         subStr(eU32 startIndex, eU32 endIndex) const;
    eString         simplified() const;
    void            remove(eU32 startIndex, eU32 endIndex);
    void            removeAt(eU32 index);

    eString         operator + (const eString &str) const;
    eString &       operator += (eChar c);
    eString &       operator += (const eString &str);
    eString &       operator = (const eChar *str);

    const eChar &   at(eU32 index) const;
    eChar &         at(eU32 index);
    const eChar &   operator [] (eInt index) const;
    eChar &         operator [] (eInt index);

    eBool           operator == (const eString &str) const;
    eBool           operator == (const eChar *str) const;
    eBool           operator != (const eString &str) const;
    eBool           operator != (const eChar *str) const;

    operator const eChar * () const;

private:
    eCharArray      m_data;
};

typedef eArray<eString *> eStringPtrArray;

#endif // STRING_HPP