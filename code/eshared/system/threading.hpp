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

#ifndef THREADING_HPP
#define THREADING_HPP

void    eSleep(eU32 ms);

ePtr    eThreadStart(void (*func)(ePtr arg), ePtr arg, eBool critical);
void    eThreadEnd(ePtr handle, eBool wait);

ePtr    eCriticalSectionCreate();
void    eCriticalSectionDelete(ePtr handle);
void    eCriticalSectionEnter(ePtr handle);
void    eCriticalSectionLeave(ePtr handle);

class eCriticalSection
{
public:
    eCriticalSection() :
        m_handle(eCriticalSectionCreate())
    {
    }

    ~eCriticalSection()
    {
        eCriticalSectionDelete(m_handle);
    }

    void enter() const
    {
        eCriticalSectionEnter(m_handle);
    }

    void leave() const
    {
        eCriticalSectionLeave(m_handle);
    }

private:
    ePtr m_handle;
};

#endif // THREADING_HPP