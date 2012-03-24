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

#ifndef TF_SONG_HPP
#define TF_SONG_HPP

class tfSong
{
public:
    struct NoteEvent
    {
        NoteEvent()
        {
            clear();
        }

        void copyFrom(const NoteEvent &ne)
        {
            noteOct = ne.noteOct;
            instrument = ne.instrument;
            velocity = ne.velocity;
            effect = ne.effect;
        }

        void clear()
        {
            noteOct = 0;
            instrument = -1;
            velocity = -1;
            effect = 0;
        }

        eU8         noteOct;
        eS8         instrument;
        eS8         velocity;

        union
        {
            eU16    effect;

            struct
            {
                eU8 effectLo;
                eU8 effectHi;
            };
        };
    };

    typedef eArray<NoteEvent> Track;

    class Pattern
    {
    public:
        Pattern(eU32 rowCount, eU32 trackCount, eU32 number);
        ~Pattern();

        void                setRowCount(eU32 rowCount);
        void                setTrackCount(eU32 trackCount);

        Pattern *           copy() const;
        Pattern *           copy(eU32 track, eU32 row, eU32 trackCount, eU32 rowCount) const;
        void                paste(const Pattern *p);
        void                paste(eU32 track, eU32 row, const Pattern *p);
        void                clear();
        void                clear(eU32 track, eU32 row, eU32 trackCount, eU32 rowCount);

        eU32                getRowCount() const;
        eU32                getTrackCount() const;
        Track &             getTrack(eU32 index);
        const Track &       getTrack(eU32 index) const;
        eU32                getPatternNumber() const;

    private:
        eArray<Track>       m_tracks;
        eU32                m_rowCount;
        const eU32          m_patternNum;
    };

    typedef eArray<Pattern *> PatternPtrArray;

    struct PatternInstance
    {
        PatternInstance(tfSong &ownerSong, Pattern &p, eU32 rowOff, eU32 track) :
            song(ownerSong),
            pattern(&p),
            rowOffset(rowOff),
            seqTrack(track)
        {
        }

        void plus()
        {
            const eU32 num = pattern->getPatternNumber()+1;

            if (num >= song.getPatternCount())
            {
                pattern = &song.newPattern(pattern->getRowCount(), pattern->getTrackCount());
            }
            else
            {
                pattern = &song.getPattern(num);
            }
        }

        void minus()
        {
            const eU32 num = pattern->getPatternNumber();

            if (num > 0)
            {
                pattern = &song.getPattern(num-1);
            }
        }

        tfSong &            song;
        Pattern *           pattern;
        eU32                rowOffset;
        eU32                seqTrack;
    };

    typedef eArray<PatternInstance *> PatternInstPtrArray;

public:
    static const eU32       MAX_SEQ_TRACKS = 16;
	static const eU32		MAX_PATTERN_TRACKS = 32;

public:
    tfSong(eID songId=eNOID);
    ~tfSong();

#ifdef ePLAYER
    void                    load(eDataStream &stream);
#endif

#ifdef eEDITOR
    void                    store(eDataStream &stream) const;
#endif

    Pattern &               newPattern(eU32 rowCount, eU32 trackCount, eInt number=-1);
    PatternInstance &       newPatternInstance(Pattern &pattern, eU32 rowOffset, eU32 seqTrack);
    void                    removePattern(eU32 index);
    void                    removePatternInstance(eU32 index);
    void                    clearAll();
    void                    clearUnusedPatterns();

    eU32                    timeToRow(eF32 time) const;
    eF32                    rowToTime(eU32 row) const;

#ifdef eEDITOR
    void                    setMuted(eU32 track, eBool muted);
    eBool                   getMuted(eU32 track) const;
#endif

    void                    setBpm(const eU32 bpm);
    void                    setUserName(const eChar *userName);

    eU32                    getId() const;
    eU32                    getBpm() const;
    eU32                    getLengthInRows() const;
    eF32                    getLengthInSecs() const;
    eU32                    getSeqTrackCount() const;
    eU32                    getPatternCount() const;
    eU32                    getPatternInstanceCount() const;
    Pattern &               getPattern(eU32 index);
    const Pattern &         getPattern(eU32 index) const;
    PatternInstance &       getPatternInstance(eU32 index);
    const PatternInstance & getPatternInstance(eU32 index) const;
    const PatternInstance * getPatternInstance(eU32 seqTrack, eU32 row) const;
    const NoteEvent *       getPreviousNoteEvent(eU32 seqTrack, eU32 track, eU32 row) const;
    const NoteEvent *       getPreviousNoteEvent(const PatternInstance *pi, eU32 track, eU32 row) const;

    const eChar *           getUserName() const;

#ifdef eEDITOR
    eBool                   isTrackFreeAt(eU32 seqTrack, eU32 rowOffset, eU32 rowCount, const PatternInstance *allowedPi=eNULL) const;
#endif

private:
    PatternPtrArray         m_patterns;
    PatternInstPtrArray     m_patternInsts;
    eU32                    m_bpm;
    eID                     m_id;
    eChar                   m_userName[eMAX_NAME_LENGTH];
#ifdef eEDITOR
    eBool                   m_muted[MAX_SEQ_TRACKS];
#endif

private:
    static eU32             m_idCounter;
};

typedef eArray<tfSong *> tfSongPtrArray;

#endif // TF_SONG_HPP