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

#include "../system/system.hpp"
#include "../math/math.hpp"
#include "tunefish3.hpp"

tfSong::Pattern::Pattern(eU32 rowCount, eU32 trackCount, eU32 number) :
    m_patternNum(number),
    m_rowCount(rowCount)
{
    eASSERT(rowCount > 0);
    eASSERT(rowCount%4 == 0);

    setTrackCount(trackCount);
}

tfSong::Pattern::~Pattern()
{
    // Destructor has to be called explicetly.
    for (eU32 i=0; i<m_tracks.size(); i++)
    {
        m_tracks[i].~eArray();
    }
}

void tfSong::Pattern::setRowCount(eU32 rowCount)
{
    eASSERT(rowCount > 0);
    eASSERT(rowCount%4 == 0);

    if (rowCount == m_rowCount)
    {
        return;
    }

    m_rowCount = rowCount;

    for (eU32 i=0; i<m_tracks.size(); i++)
    {
        m_tracks[i].resize(rowCount);

        for (eU32 j=0;j<rowCount;j++)
        {
            m_tracks[i][j].effect = 0;
            m_tracks[i][j].instrument = -1;
            m_tracks[i][j].noteOct = 0;
            m_tracks[i][j].velocity = -1;
        }
    }
}

void tfSong::Pattern::setTrackCount(eU32 trackCount)
{
    eASSERT(trackCount > 0);

    if (trackCount != m_tracks.size())
    {
        if (trackCount < m_tracks.size())
        {
            m_tracks.resize(trackCount);
        }
        else
        {
            const eU32 oldTrackCount = m_tracks.size();

            for (eU32 i=oldTrackCount; i<trackCount; i++)
            {
                m_tracks.append(Track());
                m_tracks[i].resize(m_rowCount);

                for (eU32 j=0; j<m_rowCount; j++)
                {
                    m_tracks[i][j].effect = 0;
                    m_tracks[i][j].instrument = -1;
                    m_tracks[i][j].noteOct = 0;
                    m_tracks[i][j].velocity = -1;
                }
            }
        }
    }
}

eU32 tfSong::Pattern::getRowCount() const
{
    return m_rowCount;
}

eU32 tfSong::Pattern::getTrackCount() const
{
    return m_tracks.size();
}

tfSong::Track & tfSong::Pattern::getTrack(eU32 index)
{
    eASSERT(index < m_tracks.size());
    return m_tracks[index];
}

const tfSong::Track & tfSong::Pattern::getTrack(eU32 index) const
{
    eASSERT(index < m_tracks.size());
    return m_tracks[index];
}

eU32 tfSong::Pattern::getPatternNumber() const
{
    return m_patternNum;
}

tfSong::Pattern * tfSong::Pattern::copy() const
{
    return copy(0, 0, m_tracks.size(), m_rowCount);
}

tfSong::Pattern * tfSong::Pattern::copy(eU32 track, eU32 row, eU32 trackCount, eU32 rowCount) const
{
    tfSong::Pattern *result = new tfSong::Pattern(rowCount, trackCount, 0);
    eASSERT(result != eNULL);

    for (eU32 i=track; i<track+trackCount; i++)
    {
        if (i >= m_tracks.size())
        {
            break;
        }

        tfSong::Track &dstTrack = result->getTrack(i);
        const tfSong::Track &srcTrack = getTrack(i);

        for (eU32 j=row; j<row+rowCount; j++)
        {
            if (j >= m_rowCount)
            {
                break;
            }

            dstTrack[j].copyFrom(srcTrack[j]);
        }
    }

    return result;
}

void tfSong::Pattern::paste(const Pattern *p)
{
    paste(0, 0, p);
}

void tfSong::Pattern::paste(eU32 track, eU32 row, const Pattern *p)
{
    eU32 trackCount = p->getTrackCount(); 
    eU32 rowCount = p->getRowCount();

    for (eU32 i=track; i<track+trackCount; i++)
    {
        if (i >= m_tracks.size())
        {
            break;
        }

        const tfSong::Track &srcTrack = p->getTrack(i);
        tfSong::Track &dstTrack = getTrack(i);

        for(eU32 j=row; j<row+rowCount; j++)
        {
            if (j >= m_rowCount)
            {
                break;
            }

            dstTrack[j].copyFrom(srcTrack[j]);
        }
    }
}

void tfSong::Pattern::clear()
{
    clear(0, 0, m_tracks.size(), m_rowCount);
}

void tfSong::Pattern::clear(eU32 track, eU32 row, eU32 trackCount, eU32 rowCount)
{
    for (eU32 i=track; i<track+trackCount; i++)
    {
        if (i >= m_tracks.size())
        {
            break;
        }

        tfSong::Track &track = (tfSong::Track &)getTrack(i);

        for (eU32 j=row; j<row+rowCount; j++)
        {
            if (j >= m_rowCount)
            {
                break;
            }

            track[j].clear();
        }
    }
}

eU32 tfSong::m_idCounter = 0;

tfSong::tfSong(eID songId) :
    m_bpm(125),
    m_id(songId != eNOID ? songId : ++m_idCounter)
{
#ifdef eEDITOR
    for (eU32 i=0; i<tfSong::MAX_SEQ_TRACKS; i++)
    {
        m_muted[i] = eFALSE;
    }

    eStrCopy(m_userName, "New song");
#endif
}

tfSong::~tfSong()
{
#ifdef eEDITOR
    clearAll();
#endif
}

#ifdef ePLAYER
void tfSong::load(eDataStream &stream)
{
    m_id = stream.readDword();
    m_bpm = stream.readWord();
    
    eString un = stream.readString();
    eStrNCopy(m_userName, un, eMAX_NAME_LENGTH);

    const eU32 patternCount = stream.readWord();

    for (eU32 i=0; i<patternCount; i++)
    {
        const eU32 rowCount = stream.readByte();
        const eU32 trackCount = stream.readByte();
        const eU32 patternId = stream.readWord();

        Pattern &p = newPattern(rowCount, trackCount, patternId);

        for (eU32 j=0; j<trackCount; j++)
        {
            Track &track = p.getTrack(j);

            for (eU32 k=0; k<track.size(); k++)
            {
                NoteEvent &ne = track[k];

                ne.noteOct = stream.readByte();
                ne.instrument = stream.readByte();
                ne.velocity = stream.readByte();
                ne.effect = stream.readWord();
            }
        }
    }

    const eU32 patInstCount = stream.readWord();

    for (eU32 i=0; i<patInstCount; i++)
    {
        const eU32 rowOffset = stream.readWord();
        const eU32 seqTrack = stream.readByte();
        const eU32 patternId = stream.readByte();

        Pattern *p = eNULL;

        for (eU32 j=0; j<m_patterns.size(); j++)
        {
            if (patternId == m_patterns[j]->getPatternNumber())
            {
                p = m_patterns[j];
                break;
            }
        }

        eASSERT(p != eNULL);
        PatternInstance &pi = newPatternInstance(*p, rowOffset, seqTrack);
    }
}
#endif

#ifdef eEDITOR
void tfSong::store(eDataStream &stream) const
{
    stream.writeDword(m_id);
    stream.writeWord(m_bpm);
    stream.writeString(m_userName);

    eU32 patternCount = 0;
    for (eU32 i=0; i<m_patterns.size(); i++)
    {
        const Pattern *p = m_patterns[i];
        eASSERT(p != eNULL);

        eBool used = eFALSE;
        for (eU32 i=0; i<m_patternInsts.size(); i++)
        {
            const PatternInstance *pi = m_patternInsts[i];
            eASSERT(pi != eNULL);

            if (pi->pattern == p)
                used = eTRUE;
        }

        if (used)
            patternCount++;
    }
    stream.writeWord(patternCount);

    for (eU32 i=0; i<m_patterns.size(); i++)
    {
        const Pattern *p = m_patterns[i];
        eASSERT(p != eNULL);

        eBool used = eFALSE;
        for (eU32 i=0; i<m_patternInsts.size(); i++)
        {
            const PatternInstance *pi = m_patternInsts[i];
            eASSERT(pi != eNULL);

            if (pi->pattern == p)
                used = eTRUE;
        }

        if (used)
        {
            stream.writeByte(p->getRowCount());
            stream.writeByte(p->getTrackCount());
            stream.writeWord(p->getPatternNumber());

            for (eU32 j=0; j<p->getTrackCount(); j++)
            {
                const Track &track = p->getTrack(j);

                for (eU32 k=0; k<track.size(); k++)
                {
                    const NoteEvent &ne = track[k];

                    stream.writeByte(ne.noteOct);
                    stream.writeByte(ne.instrument);
                    stream.writeByte(ne.velocity);
                    stream.writeWord(ne.effect);
                }
            }
        }
    }

    stream.writeWord(m_patternInsts.size());

    for (eU32 i=0; i<m_patternInsts.size(); i++)
    {
        const PatternInstance *pi = m_patternInsts[i];
        eASSERT(pi != eNULL);

        stream.writeWord(pi->rowOffset);
        stream.writeByte(pi->seqTrack);

        for (eU32 j=0; j<m_patterns.size(); j++)
        {
            if (pi->pattern == m_patterns[j])
            {
                eASSERT(j < 256);
                stream.writeByte(j);
                break;
            }
        }
    }
}
#endif

tfSong::Pattern & tfSong::newPattern(eU32 rowCount, eU32 trackCount, eInt number)
{
    if (number < 0)
    {
        for (eU32 i=0; i<m_patterns.size(); i++)
        {
            if ((eInt)m_patterns[i]->getPatternNumber() > number)
            {
                number = (eInt)m_patterns[i]->getPatternNumber();
            }
        }

        number++;
    }

    m_patterns.append(new Pattern(rowCount, trackCount, number));
    return *m_patterns[m_patterns.size()-1];
}

tfSong::PatternInstance & tfSong::newPatternInstance(Pattern &pattern, eU32 rowOffset, eU32 seqTrack)
{
    m_patternInsts.append(new PatternInstance(*this, pattern, rowOffset, seqTrack));
    return *m_patternInsts[m_patternInsts.size()-1];
}

void tfSong::removePattern(eU32 index)
{
    eASSERT(index < m_patterns.size());

    // Remove all pattern instances, referencing
    // pattern with given index.
    for (eInt i=m_patternInsts.size()-1; i>=0; i--)
    {
        if (m_patternInsts[i]->pattern == m_patterns[index])
        {
            m_patternInsts.removeAt(i);
        }
    }

    // Remove pattern it-self.
    eSAFE_DELETE(m_patterns[index]);
    m_patterns.removeAt(index);
}

void tfSong::removePatternInstance(eU32 index)
{
    eASSERT(index < m_patternInsts.size());
    eSAFE_DELETE(m_patternInsts[index]);
    m_patternInsts.removeAt(index);
}

void tfSong::clearUnusedPatterns()
{
    for (eInt i=m_patterns.size()-1; i>=0; i--)
    {
        Pattern *p = m_patterns[i];
        eU32 refCount = 0;

        for (eU32 j=0; j<m_patternInsts.size(); j++)
        {
            if (m_patternInsts[j]->pattern == p)
                refCount++;
        }

        if (refCount == 0)
        {
            if (i == m_patterns.size()-1)
            {
                // it's the last pattern. we can delete it
                m_patterns.removeAt(i);
                eSAFE_DELETE(p);
            }
            else
            {
                // we have a pattern in the middle. just clear it
                p->clear();
            }
        }
    }
}

void tfSong::clearAll()
{
    for (eU32 i=0; i<m_patternInsts.size(); i++)
    {
        eSAFE_DELETE(m_patternInsts[i]);
    }

    for (eU32 i=0; i<m_patterns.size(); i++)
    {
        eSAFE_DELETE(m_patterns[i]);
    }
}

eU32 tfSong::timeToRow(eF32 time) const
{
    eASSERT(time >= 0.0f);

    return eFtoL(time*(eF32)m_bpm/60.0f*4.0f);
}

eF32 tfSong::rowToTime(eU32 row) const
{
    return ((eF32)row*60.0f/4.0f/(eF32)m_bpm);
}

#ifdef eEDITOR

void tfSong::setMuted(eU32 track, eBool muted)
{
    eASSERT(track < MAX_SEQ_TRACKS);
    m_muted[track] = muted;
}

eBool tfSong::getMuted(eU32 track) const
{
    eASSERT(track < MAX_SEQ_TRACKS);
    return m_muted[track];
}

#endif

void tfSong::setBpm(const eU32 bpm)
{
    eASSERT(bpm >= 1 && bpm <= 300);
    m_bpm = bpm;
}

void tfSong::setUserName(const eChar *userName)
{
    eASSERT(userName != eNULL);
    eStrNCopy(m_userName, userName, eMAX_NAME_LENGTH);
}

eU32 tfSong::getId() const
{
    return m_id;
}

eU32 tfSong::getBpm() const
{
    return m_bpm;
}

eU32 tfSong::getLengthInRows() const
{
    eU32 length = 0;

    for (eU32 i=0; i<m_patternInsts.size(); i++)
    {
        const PatternInstance *pi = m_patternInsts[i];
        eASSERT(pi != eNULL);

        length = eMax(length, pi->rowOffset + pi->pattern->getRowCount());
    }

    return length;
}

eF32 tfSong::getLengthInSecs() const
{
    eASSERT(eFALSE);
    return -1.0f;
}

eU32 tfSong::getSeqTrackCount() const
{
    eU32 count = 0;

    for (eU32 i=0; i<m_patternInsts.size(); i++)
    {
        count = eMax(count, m_patternInsts[i]->seqTrack);
    }

    return count+1;
}

eU32 tfSong::getPatternInstanceCount() const
{
    return m_patternInsts.size();
}

eU32 tfSong::getPatternCount() const
{
    return m_patterns.size();
}

tfSong::Pattern & tfSong::getPattern(eU32 index)
{
    eASSERT(index < m_patterns.size());
    return *m_patterns[index];
}

const tfSong::Pattern & tfSong::getPattern(eU32 index) const
{
    eASSERT(index < m_patterns.size());
    return *m_patterns[index];
}

tfSong::PatternInstance & tfSong::getPatternInstance(eU32 index)
{
    eASSERT(index < m_patternInsts.size());
    return *m_patternInsts[index];
}

const tfSong::PatternInstance & tfSong::getPatternInstance(eU32 index) const
{
    eASSERT(index < m_patternInsts.size());
    return *m_patternInsts[index];
}

const tfSong::PatternInstance * tfSong::getPatternInstance(eU32 seqTrack, eU32 row) const
{
    eASSERT(seqTrack < MAX_SEQ_TRACKS);

    for (eU32 i=0; i<m_patternInsts.size(); i++)
    {
        const PatternInstance *pi = m_patternInsts[i];
        eASSERT(pi != eNULL);

        if (pi->seqTrack == seqTrack)
        {
            if (row >= pi->rowOffset && row < pi->rowOffset+pi->pattern->getRowCount())
            {
                return pi;
            }
        }
    }

    return eNULL;
}

const tfSong::NoteEvent * tfSong::getPreviousNoteEvent(eU32 seqTrack, eU32 track, eU32 row) const
{  
    const NoteEvent *ne = eNULL;

    if (row > 0)
    {
        const PatternInstance *pi = eNULL;

        row--;

        while (!ne)
        {
            pi = getPatternInstance(seqTrack, row);

            if (!pi)
            {
                return eNULL;
            }

            ne = getPreviousNoteEvent(pi, track, row);

            if (!ne)
            {
                if (pi->rowOffset == 0)
                {
                    return eNULL;
                }

                row = pi->rowOffset-1;
            }
        }
    }

    return ne;
}

const tfSong::NoteEvent * tfSong::getPreviousNoteEvent(const PatternInstance *pi, eU32 track, eU32 row) const
{
    eASSERT(pi != eNULL);

    const eU32 r = row-pi->rowOffset;
    const Track &t = pi->pattern->getTrack(track);

    for (eInt i=(eInt)r; i>=0; i--)
    {
        const NoteEvent &ne = t[(eU32)i];

        if (ne.noteOct != 0)
        {
            return &ne;
        }
    }

    return eNULL;
}

const eChar * tfSong::getUserName() const
{
    return m_userName;
}

#ifdef eEDITOR
eBool tfSong::isTrackFreeAt(eU32 seqTrack, eU32 rowOffset, eU32 rowCount, const PatternInstance *allowedPi) const
{
    eASSERT(seqTrack < MAX_SEQ_TRACKS);

    for (eU32 i=0; i<m_patternInsts.size(); i++)
    {
        const PatternInstance *pi = m_patternInsts[i];
        eASSERT(pi != eNULL);

        if (pi != allowedPi && pi->seqTrack == seqTrack)
        {
            if (rowOffset <= pi->rowOffset && rowOffset+rowCount > pi->rowOffset)
            {
                return eFALSE;
            }
            else if (rowOffset >= pi->rowOffset && rowOffset < pi->rowOffset + pi->pattern->getRowCount())
            {
                return eFALSE;
            }
        }
    }

    return eTRUE;
}
#endif