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

#ifndef SEQUENCER_HPP
#define SEQUENCER_HPP

class eSequencer
{
public:
    struct Scene
    {
        eIEffect *          effect;
        eF32                timeOffset;
        eF32                timeScale;
    };

    struct Overlay
    {
        eITexture2d *       texture;
        eBool               filtered;
        eFXYZW              rect;
        eFXY                scrollUv;
        eFXY                tileUv;
        eTextureAddressMode uvAddrMode; 
    };

    struct Entry
    {
        enum Type
        {
            TYPE_SCENE,
            TYPE_OVERLAY,
        };

        enum BlendMode
        {
            BLEND_ADD,
            BLEND_SUB,
            BLEND_MUL,
            BLEND_BRIGHTER,
            BLEND_DARKER,
            BLEND_NONE,
        };
     
        union
        {
            Scene           scene;
            Overlay         overlay;
        };

        Type                type;
        eF32                startTime;
        eF32                duration;
        BlendMode           blendMode;
        eVector2            blendRatios;
    };

private:
    typedef eArray<Entry> EntryArray;

public:
    eSequencer();
    ~eSequencer();
    
    eBool                   run(eF32 time, eITexture2d *target, eIRenderer *renderer);

    void                    addEntry(const Entry &entry, eU32 track);
    void                    merge(const eSequencer &seq);
    void                    clear();

    void                    setAspectRatio(eF32 aspectRatio);
	void					setRotation(eU32 rotationStart, eU32 rotationEnd, eU32 rotationCurrent);
    void                    getRotation(eU32 &rotationStart, eU32 &rotationEnd, eU32 &rotationCurrent) const;

    const EntryArray &      getEntriesOfTrack(eU32 track) const;
    eF32                    getAspectRatio() const;
    
private:
    const Entry *           _getEntryForTrack(eF32 time, eU32 track) const;
    void                    _setupTargets(eGraphicsApiDx9 *gfx, const eSize &size);
    void                    _freeTargets();
    void                    _renderEntry(const Entry &entry, eF32 time, eITexture2d *target, eIRenderer *renderer) const;
    void                    _mergeTargets(eITexture2d *target, eIRenderer *renderer) const;

public:
    static const eInt       MAX_TRACKS = 16;

private:
    eF32                    m_endTime;
    eF32                    m_aspectRatio;
	eU32					m_rotationStart;
	eU32					m_rotationEnd;
	eU32					m_rotationCurrent;
    eITexture2d *           m_tempTarget;
    eITexture2d *           m_copyTarget;
    EntryArray              m_entries[MAX_TRACKS];
    eIVertexShader *        m_vsQuad;
    eIPixelShader *         m_psMergeFx;
};

#endif // SEQUENCER_HPP