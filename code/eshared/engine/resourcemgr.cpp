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

#include "../system/system.hpp"
#include "../math/math.hpp"
#include "engine.hpp"

eGraphicsApiDx9 *                 eResourceManager::m_gfx = eNULL;
eResourceManager::IResPtrArray  eResourceManager::m_resources;

void eResourceManager::setGraphics(eGraphicsApiDx9 *gfx)
{
    eASSERT(gfx != eNULL);
    m_gfx = gfx;
}

eBool eResourceManager::uploadAll()
{
    for (eU32 i=0; i<m_resources.size(); i++)
    {
        if (!m_resources[i]->upload())
        {
            return eFALSE;
        }
    }

    return eTRUE;
}

eBool eResourceManager::unloadAll()
{
    _sortResourcesByType();

    for (eInt i=(eInt)m_resources.size()-1; i>=0; i--)
    {
        if (!m_resources[i]->unload())
        {
            return eFALSE;
        }
    }

    return eTRUE;
}

eBool eResourceManager::reloadAll()
{
    return (unloadAll() && reloadAll());
}

eBool eResourceManager::addResource(eIResource *res)
{
    eASSERT(res != eNULL);
    eASSERT(m_resources.exists(res) == -1);

    m_resources.append(res);
    return eTRUE;
}

eBool eResourceManager::removeResource(eIResource *res)
{
    eASSERT(res != eNULL);

    const eInt index = m_resources.exists(res);
    eASSERT(index != -1);
    m_resources.removeAt(index);

    return eTRUE;
}

void eResourceManager::_sortResourcesByType()
{
    eBool swapped;
    eU32 i = 0;

    do
    {
        swapped = eFALSE;
        const eU32 top = m_resources.size()-i;

        for (eU32 j=1; j<top; j++)
        {
            if (m_resources[j-1]->getType() > m_resources[j]->getType())
            {
                eSwap(m_resources[j-1], m_resources[j]);
                swapped = eTRUE;
            }
        }

        i++;
    }
    while (swapped);
}