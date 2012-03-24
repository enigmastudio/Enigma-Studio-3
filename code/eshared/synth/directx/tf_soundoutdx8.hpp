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

#ifndef TF_SOUND_OUT_DX8_HPP
#define TF_SOUND_OUT_DX8_HPP

struct IDirectSound8;
struct IDirectSoundBuffer8;

class tfSoundOutDx8 : public tfISoundOut
{
public:
    tfSoundOutDx8(eU32 latency, eU32 sampleRate);
    virtual ~tfSoundOutDx8();

    virtual eBool           initialize(eU32 latency, eU32 sampleRate);
    virtual void            shutdown();

    virtual void            play();
    virtual void            stop();
    virtual void            clear();
    virtual void            fill(const eU8 *data, eU32 count);

    virtual eBool           isFilled() const;
    virtual eU32            getSampleRate() const;

private:
    IDirectSound8 *         m_ds;
    IDirectSoundBuffer8 *   m_dsb;
    eU32                    m_nextWriteOffset;
    eU32                    m_bufferSize;
    eU32                    m_sampleRate;
};

#endif // TF_SOUND_OUT_DX8_HPP