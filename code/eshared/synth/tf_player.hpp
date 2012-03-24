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

#ifndef TF_PLAYER_HPP
#define TF_PLAYER_HPP

class tfPlayer
{
public:
    tfPlayer(tfISoundOut *soundOut);
    ~tfPlayer();

    void                addInstrument(eU32 index);
    void                removeInstrument(eU32 index);

#ifndef TF3_VSTI
    void                loadInstruments(eDataStream &stream);
    void                storeInstruments(eDataStream &stream) const;
#endif
    void                clearInstruments();

    void                play(eF32 time);
    void                stop();
    void                process();
    void                allNotesOff();
    void                panic();
    void                setVolume(eF32 volume);
    void                mute(eBool on);
	void				setLoop(eU32 startRow, eU32 endRow);

    void                noteEvent(eU32 instrument, eU32 note, eU32 velocity, eU32 modslot, eF32 mod);

    void                setSong(tfSong *song);
    void                setSampleRate(eU32 sampleRate);
    void                setTime(eF32 time);
    void                setPosition(eU32 row);

	tfSong *			getSong();
    tfInstrument *      getInstrument(eU32 index);
    eU32                getSampleRate() const;
    eF32                getTime() const;
    eU32                getPosition(eU32 row) const;
    eF32                getPeakInstr(eU32 instr) const;
	eF32                getPeakTrack(eU32 track) const;
    eF32                getMasterPeak() const;

    eCriticalSection *  getCriticalSection();

private:
    void                _processRow();
    void                _processAudio();

    void                _startThread();
    void                _stopThread();

private:
    static void         _threadProc(ePtr arg);

private:
    tfSong *            m_song;
    tfISoundOut *       m_soundOut;
    tfInstrument *      m_instruments[TF_MAX_INPUTS];
	eU32				m_instrumentPatternTrack[TF_MAX_INPUTS];
	tfSong::NoteEvent   m_lastEvents[tfSong::MAX_SEQ_TRACKS][tfSong::MAX_PATTERN_TRACKS];
    eU32                m_sampleRate;
    eBool               m_muted[tfSong::MAX_SEQ_TRACKS];

    eF32                m_peakInstrMemory[TF_MAX_INPUTS * TF_PLAYER_PEAK_MEMORY];
    eF32                m_peakInstr[TF_MAX_INPUTS];
	eF32                m_peakTrack[tfSong::MAX_SEQ_TRACKS];
    eF32                m_masterPeakMemory[TF_PLAYER_PEAK_MEMORY];
    eF32                m_masterPeak;
    eSignal *           m_outputSignal[2];
	eSignal *           m_tempSignal[2];
    eS16 *              m_outputFinal;
    eU32                m_signalCount;

    eBool               m_mute;
    eF32                m_volume;
    eS32                m_row;
    eU32                m_time;
    eBool               m_playing;
    eBool               m_joinRequest;
    eBool               m_allNotesOff;
    eCriticalSection    m_criticalSection;
    ePtr                m_threadHandle;
	eU32				m_loopStartRow;
	eU32				m_loopEndRow;
};

#endif // TF_PLAYER_HPP