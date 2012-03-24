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

// Initialize static members.
tfISoundOut *   eDemo::m_soundOut = new tfSoundOutDx8(512, 44100);
tfPlayer *      eDemo::m_player = new tfPlayer(eDemo::m_soundOut);

void eDemo::shutdown()
{
    m_player->stop();
    eSAFE_DELETE(m_player);
    eSAFE_DELETE(m_soundOut);
}

tfPlayer & eDemo::getSynth()
{
    return *m_player;
}

eDemo::eDemo() :
    m_target(eNULL)
{
}

#ifdef eEDITOR
eDemo::~eDemo()
{
    eSAFE_DELETE(m_target);
}
#endif

// Returns wether or not playing sequencer
// ended and no more entries are left.
eBool eDemo::render(eF32 time, eIRenderer *renderer)
{
    eASSERT(renderer != eNULL);
    eASSERT(time >= 0.0f);

    eGraphicsApiDx9 *gfx = renderer->getGraphicsApi();
    eASSERT(gfx != eNULL);

    _setupTarget(gfx, m_seq.getAspectRatio());

    // Clear the background.
    eStateManager::bindRenderTarget(0, eGraphicsApiDx9::TARGET_SCREEN);
    eStateManager::apply();
    gfx->clear(eCLEAR_ALLBUFFERS, eColor::BLACK);

    // Render next frame onto target and copy to screen.
    const eBool res = m_seq.run(time, m_target, renderer);
    eStateManager::bindRenderTarget(0, eGraphicsApiDx9::TARGET_SCREEN);
    renderer->renderTexturedQuad(m_renderRect, gfx->getWindowSize(), m_target);

    return res;
}

void eDemo::setSequencer(const eSequencer &seq)
{
    m_seq = seq;
}

void eDemo::setSong(tfSong *song)
{
    m_player->setSong(song);
}

const eSequencer & eDemo::getSequencer() const
{
    return m_seq;
}

eSequencer & eDemo::getSequencer()
{
    return m_seq;
}

void eDemo::_setupTarget(eGraphicsApiDx9 *gfx, eF32 aspectRatio)
{
    eASSERT(aspectRatio > 0.0f);

    // Calculate target size based on aspect ratio.
    const eU32 winWidth = gfx->getWindowWidth();
    const eU32 winHeight = gfx->getWindowHeight();

    eU32 targetWidth = winWidth;
    eU32 targetHeight = eFtoL(winWidth*(1.0f/aspectRatio));

    if (targetHeight > winHeight)
    {
        targetHeight = winHeight;
        targetWidth = eFtoL(winHeight*aspectRatio);
    }

    // Recreate target if needed.
    if (m_target == eNULL || m_target->getSize() != eSize(targetWidth, targetHeight))
    {
        eSAFE_DELETE(m_target);

        const eU32 halfDiffWidth = (winWidth-targetWidth)/2;
        const eU32 halfDiffHeight = (winHeight-targetHeight)/2;

        m_renderRect.set(halfDiffWidth, halfDiffHeight, winWidth-halfDiffWidth, winHeight-halfDiffHeight);
        m_target = gfx->createTexture2d(targetWidth, targetHeight, eTRUE, eFALSE, eFALSE, eFORMAT_ARGB8);
    }
}