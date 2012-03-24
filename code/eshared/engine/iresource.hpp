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

#ifndef IRESOURCE_HPP
#define IRESOURCE_HPP

class eIResource
{
public:
    enum Type
    {
        TYPE_VOLATILE,
        TYPE_NONVOLATILE
    };

public:
    eIResource();
    virtual ~eIResource();

    virtual Type            getType() = 0;

    virtual eBool           upload() = 0;
    virtual eBool           unload() = 0;
};

class eIVolatileResource : public eIResource
{
public:
    virtual Type            getType();
};

class eINonVolatileResource : public eIResource
{
public:
    virtual Type            getType();
};

#endif // IRESOURCE_HPP