/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   This file is part of
 *
 *    ------  /   /  /\   /  /---  -----  /  ----  /   /
 *       /   /   /  /  \ /  /---   -/-   /   \--  /---/   version 3
 *      /    \---  /    /  /---    /    /  ----/ /   /.
 *
 *       t i n y   m u s i c   s y n t h e s i z e r
 *
 *   Copyright © 2003-2010 Brain Control, all rights reserved.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef TF_ISOUND_OUT_HPP
#define TF_ISOUND_OUT_HPP

class tfISoundOut 
{
public:
    virtual eBool   initialize(eU32 latency, eU32 sampleRate) = 0;
    virtual void    shutdown() = 0;

    virtual void    play() = 0;
    virtual void    stop() = 0;
    virtual void    clear() = 0;
    virtual void    fill(const eU8 *data, eU32 count) = 0;

    virtual eBool   isFilled() const = 0;
    virtual eU32    getSampleRate() const = 0;
};

#endif // TF_ISOUND_OUT_HPP