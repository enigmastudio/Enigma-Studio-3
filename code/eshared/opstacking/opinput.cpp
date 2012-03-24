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

#include "../eshared.hpp"

#ifdef eEDITOR
eAllowedOpInput::eAllowedOpInput(const eString &config)
{
    parseConfig(config);
}

void eAllowedOpInput::parseConfig(const eString &config)
{
    _clear();

    const eU32 configLen = config.length();
    eString buffer;

    for (eU32 i=0; i<configLen; i++)
    {
        buffer = "";

        while (i < configLen && config[i] != '|')
        {
            buffer += config[i++];
        }

        eString left, right;

        if (!buffer.split(',', left, right))
        {
            eSwap(left, right);
        }

        WhereWhat *ww = new WhereWhat;
        eASSERT(ww != eNULL);
        ww->index = (left == "" ? -1 : eStrToInt(left));
        ww->what = right;

        m_allowed.append(ww);
        m_allAllowed.append(new eString(right));
    }
}

eAllowedOpInput::~eAllowedOpInput()
{
    _clear();
}

eBool eAllowedOpInput::isAllowed(const eString &what) const
{
    for (eU32 i=0; i<m_allowed.size(); i++)
    {
        const eString &w = m_allowed[i]->what;

        if (w == what || w == "All")
        {
            return eTRUE;
        }
    }

    return eFALSE;
}

eBool eAllowedOpInput::isAllowed(const eIOperator *op) const
{
    eASSERT(op != eNULL);
    return (isAllowed(op->getCategory()) || isAllowed(op->getType()));
}

eBool eAllowedOpInput::isAllowedAt(eU32 index, const eString &what) const
{
    for (eU32 i=0; i<m_allowed.size(); i++)
    {
        const WhereWhat &ww = *m_allowed[i];

        if ((ww.index == index && ww.what == what)  ||
            (ww.index == -1    && ww.what == what)  ||
            (ww.index == index && ww.what == "All") ||
            (ww.index == -1    && ww.what == "All"))
        {
            return eTRUE;
        }
    }

    return eFALSE;
}

eBool eAllowedOpInput::isAllowedAt(eU32 index, const eIOperator *op) const
{
    eASSERT(op != eNULL);
    return (isAllowedAt(index, op->getCategory()) || isAllowedAt(index, op->getType()));
}

eStringPtrArray eAllowedOpInput::getAllAllowed() const
{
    return m_allAllowed;
}

void eAllowedOpInput::_clear()
{
    for (eU32 i=0; i<m_allowed.size(); i++)
    {
        eSAFE_DELETE(m_allowed[i]);
    }

    m_allowed.clear();

    for (eU32 i=0; i<m_allAllowed.size(); i++)
    {
        eSAFE_DELETE(m_allAllowed[i]);
    }

    m_allAllowed.clear();
}
#endif