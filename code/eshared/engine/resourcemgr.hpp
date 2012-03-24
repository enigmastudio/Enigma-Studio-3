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

#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

class eResourceManager
{
    friend eIResource;

public:
    static void             setGraphics(eGraphicsApiDx9 *gfx);

    static eBool            uploadAll();
    static eBool            unloadAll();
    static eBool            reloadAll();

protected:
    static eBool            addResource(eIResource *res);
    static eBool            removeResource(eIResource *res);

private:
    static void             _sortResourcesByType();

private:
    typedef eArray<eIResource *> IResPtrArray;

private:
    static IResPtrArray     m_resources;
    static eGraphicsApiDx9 *  m_gfx;
};

#endif // RESOURCE_MANAGER_HPP