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

eString::eString()
{
    m_data.append('\0');
}

eString::eString(const eChar *str)
{
    eASSERT(str != eNULL);

    m_data.resize(eStrLength(str)+1);
    eMemCopy(&m_data[0], str, m_data.size());
}

eString::eString(const eChar *str, eU32 length)
{
    eASSERT(str != eNULL);
    eASSERT(length > 0);
    eASSERT(eStrLength(str) >= length);

    m_data.resize(length+1);
    eMemCopy(&m_data[0], str, length);
}

eString::eString(const eString &str)
{
    *this = str;
}

eU32 eString::length() const
{
    return m_data.size()-1;
}

eBool eString::equals(const eString &str, eU32 count) const
{
    eASSERT(count <= str.length());

    for (eU32 i=0; i<eMin(count, length()); i++)
    {
        if (str[i] != m_data[i])
        {
            return eFALSE;
        }
    }

    return eTRUE;
}

void eString::makeUpper()
{
    eStrUpper(&m_data[0]);
}

// Returns wether or not the given token was
// found inside the string or not.
eBool eString::split(eChar token, eString &left, eString &right) const
{
    left = "";
    right = "";

    eBool found = eFALSE;
    eString *dst = &left;

    for (eU32 i=0; i<length(); i++)
    {
        const eChar c = at(i);

        if (c == token)
        {
            dst = &right;
            found = eTRUE;
        }
        else
        {
            *dst += c;
        }
    }

    return found;
}

eString eString::subStr(eU32 startIndex, eU32 endIndex) const
{
    eASSERT((eInt)startIndex < (eInt)m_data.size()-1);
    eASSERT((eInt)endIndex < (eInt)m_data.size()-1);
    eASSERT(startIndex <= endIndex);

    const eChar *str = (*this);

    return eString(str+startIndex, endIndex-startIndex);
}

// Returns a string with no spaces in the beginning
// and end and all white space sequences in the
// inside of the string are replaced by just one
// single space.
eString eString::simplified() const
{
    // Remove spaces in the beginning.
    eString ts = *this;

    while (ts.length() > 0 && ts[0] == ' ')
    {
        ts.removeAt(0);
    }

    // Remove spaces in the end.
    while (ts.length() > 0 && ts[ts.length()-1] == ' ')
    {
        ts.removeAt(ts.length()-1);
    }

    // Remove inner spaces.
    for (eInt i=ts.length()-1; i>0; i--)
    {
        if (ts[i] == ' ' && ts[i-1] == ' ')
        {
            ts.removeAt(i);
        }
    }

    return ts;
}

// Removes a sub-string start and ending
// at the given indices.
void eString::remove(eU32 startIndex, eU32 endIndex)
{
    eASSERT((eInt)startIndex < (eInt)m_data.size()-1);
    eASSERT((eInt)endIndex < (eInt)m_data.size()-1);
    eASSERT(startIndex <= endIndex);

    for (eU32 i=endIndex; i>=startIndex; i--)
    {
        m_data.removeAt(i);
    }
}

void eString::removeAt(eU32 index)
{
    eASSERT((eInt)index < (eInt)m_data.size()-1);
    remove(index, index);
}

eString eString::operator + (const eString &str) const
{
    eString res = *this;

    for (eU32 i=0; i<str.length(); i++)
    {
        res += str[i];
    }

    return res;
}

eString & eString::operator += (eChar c)
{
    m_data.insert(m_data.size()-1, c);
    return *this;
}

eString & eString::operator += (const eString &str)
{
    for (eU32 i=0; i<str.length(); i++)
    {
        (*this) += str[i];
    }

    return *this;
}

eString & eString::operator = (const eChar *str)
{
    m_data.resize(eStrLength(str)+1);
    eMemCopy(&m_data[0], str, m_data.size());

    return *this;
}

eString::operator const eChar * () const
{
    return &m_data[0];
}

const eChar & eString::at(eU32 index) const
{
    eASSERT(index < m_data.size()-1);
    return m_data[index];
}

eChar & eString::at(eU32 index)
{
    eASSERT(index < m_data.size()-1);
    return m_data[index];
}

const eChar & eString::operator [] (eInt index) const
{
    eASSERT(index < (eInt)m_data.size()-1);
    return m_data[index];
}

eChar & eString::operator [] (eInt index)
{
    eASSERT(index < (eInt)m_data.size()-1);
    return m_data[index];
}

eBool eString::operator == (const eString &str) const
{
    return (eStrCompare(*this, str) == 0);
}

eBool eString::operator == (const eChar *str) const
{
    return (*this == eString(str));
}

eBool eString::operator != (const eString &str) const
{
    return !(*this == str);
}

eBool eString::operator != (const eChar *str) const
{
    return !(*this == str);
}