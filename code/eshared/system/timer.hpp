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

#ifndef TIMER_HPP
#define TIMER_HPP

class eTimer
{
public:
    eTimer();

    void            restart();
    eU32            getElapsedMs() const;

public:
    static eU32     getTimeMs();
    static eU64     getTickCount();
    static eU64     getFrequency();

private:
    static void     _initialize();

private:
    static eU64     m_freq;
    static eU64     m_correction;
    static eBool    m_inited;

private:
    eU64            m_startTime;
};

#endif // TIMER_HPP