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

#ifndef SINGLETON_HPP
#define SINGLETON_HPP

template<class T> class eSingleton
{
protected:
    eSingleton()
    {
    }

    virtual ~eSingleton()
    {
    }

private:
    eSingleton(const eSingleton &)
    {
    }

    eSingleton & operator = (const eSingleton &)
    {
    }

public:
    static T& get()
    {
        static T instance;
        return instance;
    }
};

#endif // SINGLETON_HPP