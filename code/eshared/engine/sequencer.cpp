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
#include "engine.hpp"

eSequencer::eSequencer() :
    m_endTime(0.0f),
    m_aspectRatio(4.0f/3.0f),
	m_rotationStart(0),
	m_rotationEnd(0),
	m_rotationCurrent(0),
    m_tempTarget(eNULL),
    m_copyTarget(eNULL),
    m_vsQuad(eShaderManager::loadVertexShader(eVS(quad))),
    m_psMergeFx(eShaderManager::loadPixelShader(ePS(fx_merge)))
{
    eASSERT(m_vsQuad != eNULL);
    eASSERT(m_psMergeFx != eNULL);
}

eSequencer::~eSequencer()
{
    _freeTargets();
}

// Returns true if time > end time of last entry,
// else false is returned.
eBool eSequencer::run(eF32 time, eITexture2d *target, eIRenderer *renderer) 
{
    eASSERT(time >= 0.0f);
    eASSERT(renderer != eNULL);
    eASSERT(target != eNULL);
    eASSERT(target->isRenderTarget() == eTRUE);

    eGraphicsApiDx9 *gfx = renderer->getGraphicsApi();
    eASSERT(gfx != eNULL);

    _setupTargets(gfx, target->getSize());

    // Clear final target.
    eStateManager::bindRenderTarget(0, target);
    eStateManager::setTextureAddressMode(0, eTEXADDRMODE_CLAMP);
    eStateManager::apply();
    gfx->clear(eCLEAR_ALLBUFFERS, eColor::BLACK);

    // Are there still entries to play?
    // Used for indicating player to exit.
    if (time > m_endTime)
    {
        return eTRUE;
    }

    // Render all active entries.
    for (eU32 i=0; i<MAX_TRACKS; i++)
    {
		if (m_rotationStart != m_rotationEnd) {
			if (i != m_rotationCurrent && i >= m_rotationStart && i <= m_rotationEnd)
				continue;
		}

        const Entry *entry = _getEntryForTrack(time, i);
        
        if (entry)
        {
            // Render entry to temporary target.
            _renderEntry(*entry, time, m_tempTarget, renderer);

            // Copy final target (needed, because a texture
            // can't be bound as texture and render target
            // at the same time).
            eStateManager::push();
            eStateManager::bindRenderTarget(0, m_copyTarget);
            eStateManager::setTextureAddressMode(0, eTEXADDRMODE_CLAMP);
            renderer->renderTexturedQuad(eRect(0, 0, target->getWidth(), target->getHeight()), target->getSize(), target);

            // Merge temporary and copied render-targets.
            if (entry->type == Entry::TYPE_SCENE || entry->type == Entry::TYPE_OVERLAY)
            {
                eStateManager::setCullingMode(eCULLING_NONE);
                eStateManager::setCap(eCAP_ZBUFFER, eFALSE);
                eStateManager::setCap(eCAP_BLENDING, eFALSE);
                eStateManager::bindVertexShader(m_vsQuad);
                eStateManager::bindPixelShader(m_psMergeFx);
                eStateManager::bindTexture(0, m_copyTarget);
                eStateManager::bindTexture(1, m_tempTarget);
                eStateManager::bindRenderTarget(0, target);
                eStateManager::setTextureAddressMode(0, eTEXADDRMODE_CLAMP);
                eStateManager::setTextureAddressMode(1, eTEXADDRMODE_CLAMP);
                gfx->setPsConst(0, eVector3((eF32)entry->blendMode, entry->blendRatios.x, entry->blendRatios.y));
                renderer->renderQuad(eRect(0, 0, target->getWidth(), target->getHeight()), target->getSize());
            }

            eStateManager::pop();
        }
    }

    return eFALSE;
}

void eSequencer::addEntry(const Entry &entry, eU32 track)
{
    eASSERT(track < MAX_TRACKS);

    m_entries[track].append(entry);
    m_endTime = eMax(m_endTime, entry.startTime+entry.duration);
}

void eSequencer::merge(const eSequencer &seq)
{
    for (eU32 i=0; i<MAX_TRACKS; i++)
    {
        for (eU32 j=0; j<seq.m_entries[i].size(); j++)
        {
            const Entry &entry = seq.m_entries[i][j];

            m_entries[i].append(entry);
            m_endTime = eMax(m_endTime, entry.startTime+entry.duration);
        }
    }
}

void eSequencer::clear()
{
    for (eU32 i=0; i<MAX_TRACKS; i++)
    {
        m_entries[i].clear();
    }

    m_endTime = 0.0f;
}

void eSequencer::setAspectRatio(eF32 aspectRatio)
{
    eASSERT(aspectRatio > 0.0f);
    m_aspectRatio = aspectRatio;
}

void eSequencer::setRotation(eU32 rotationStart, eU32 rotationEnd, eU32 rotationCurrent)
{
	m_rotationStart = rotationStart;
	m_rotationEnd = rotationEnd;
	m_rotationCurrent = rotationCurrent;
}

void eSequencer::getRotation(eU32 &rotationStart, eU32 &rotationEnd, eU32 &rotationCurrent) const
{
    rotationStart = m_rotationStart;
    rotationEnd = m_rotationEnd;
    rotationCurrent = m_rotationCurrent;
}

const eSequencer::EntryArray & eSequencer::getEntriesOfTrack(eU32 track) const
{
    eASSERT(track < MAX_TRACKS);
    return m_entries[track];
}

eF32 eSequencer::getAspectRatio() const
{
    return m_aspectRatio;
}

const eSequencer::Entry * eSequencer::_getEntryForTrack(eF32 time, eU32 track) const
{
    eASSERT(track < MAX_TRACKS);
    eASSERT(time >= 0.0f);

    for (eU32 i=0; i<m_entries[track].size(); i++)
    {
        const Entry *entry = &m_entries[track][i];
        eASSERT(entry != eNULL);

        if (entry->startTime <= time && entry->startTime+entry->duration > time)
        {
            return entry;
        }
    }

    return eNULL;
}

void eSequencer::_setupTargets(eGraphicsApiDx9 *gfx, const eSize &size)
{
    eASSERT(gfx != eNULL);
    eASSERT(size.width > 0);
    eASSERT(size.height > 0);

    if (m_tempTarget == eNULL || m_tempTarget->getSize() != size)
    {
        _freeTargets();

        m_tempTarget = gfx->createTexture2d(size.width, size.height, eTRUE, eFALSE, eFALSE, eFORMAT_ARGB8);
        m_copyTarget = gfx->createTexture2d(size.width, size.height, eTRUE, eFALSE, eFALSE, eFORMAT_ARGB8);
    }
}

void eSequencer::_freeTargets()
{
    eSAFE_DELETE(m_copyTarget);
    eSAFE_DELETE(m_tempTarget);
}

void eSequencer::_renderEntry(const Entry &entry, eF32 time, eITexture2d *target, eIRenderer *renderer) const
{
    switch (entry.type)
    {
        case Entry::TYPE_SCENE:
        {
            entry.scene.effect->run((time-entry.startTime) * entry.scene.timeScale + entry.scene.timeOffset, target, renderer);
            break;
        }

        case Entry::TYPE_OVERLAY:
        {
            const Overlay &ol = entry.overlay;

            const eVector2 ul(ol.rect.x, 1.0f-ol.rect.y);
            const eVector2 br(ol.rect.z, 1.0f-ol.rect.w);

            const eRect r(eFtoL(ul.x*(eF32)target->getWidth()),
                          eFtoL(br.y*(eF32)target->getHeight()),
                          eFtoL(br.x*(eF32)target->getWidth()),
                          eFtoL(ul.y*(eF32)target->getHeight()));

            eStateManager::push();
            eStateManager::bindRenderTarget(0, target);
            eStateManager::setTextureFilter(0, (entry.overlay.filtered ? eTEXFILTER_BILINEAR : eTEXFILTER_NEAREST));
            eStateManager::setTextureAddressMode(0, entry.overlay.uvAddrMode);
            eStateManager::apply();
            
            renderer->getGraphicsApi()->clear(eCLEAR_COLORBUFFER, eColor(0, 0, 0, 0));
            renderer->renderTexturedQuad(r, target->getSize(), ol.texture, ol.tileUv, ol.scrollUv);
            
            eStateManager::pop();
            break;
        }
    }
}