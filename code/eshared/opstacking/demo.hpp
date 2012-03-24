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

#ifndef DEMO_HPP
#define DEMO_HPP

class eDemo
{
public:
    static void             shutdown();
    static tfPlayer &       getSynth();

public:
    eDemo();
#ifdef eEDITOR
    ~eDemo();
#endif

    eBool                   render(eF32 time, eIRenderer *renderer);

    void                    setSequencer(const eSequencer &seq);
    void                    setSong(tfSong *song);

    const eSequencer &      getSequencer() const;
    eSequencer &            getSequencer();

private:
    void                    _setupTarget(eGraphicsApiDx9 *gfx, eF32 aspectRatio);

public:
    static const eU32       MAX_RUNNING_TIME_MINS = 7;

private:
    static tfPlayer *       m_player;
    static tfISoundOut *    m_soundOut;

private:
    eSequencer              m_seq;
    eITexture2d *           m_target;
    eRect                   m_renderRect;
};

#endif // DEMO_HPP