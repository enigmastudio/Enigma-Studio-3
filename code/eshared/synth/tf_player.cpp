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

#ifndef eHWSYNTH

#include "../eshared.hpp"
#include <stdio.h>

tfPlayer::tfPlayer(tfISoundOut *soundOut) :
    m_row(0),
    m_playing(eFALSE),
    m_joinRequest(eFALSE),
    m_threadHandle(0),
    m_song(eNULL),
    m_signalCount(0),
    m_sampleRate(44100),
    m_time(0),
    m_soundOut(soundOut),
    m_mute(eFALSE),
    m_volume(1.0f),
    m_masterPeak(0.0f)
{
    eASSERT(soundOut != eNULL);

    eMemSet(m_instruments, 0, TF_MAX_INPUTS * sizeof(tfInstrument *));
    eMemSet(m_peakInstrMemory, 0, TF_MAX_INPUTS * TF_PLAYER_PEAK_MEMORY * sizeof(eF32));
    eMemSet(m_masterPeakMemory, 0, TF_PLAYER_PEAK_MEMORY * sizeof(eF32));
    eMemSet(m_peakInstr, 0, TF_MAX_INPUTS * sizeof(eF32));
	eMemSet(m_peakTrack, 0, tfSong::MAX_SEQ_TRACKS * sizeof(eF32));
    eMemSet(m_muted, 0, tfSong::MAX_SEQ_TRACKS * sizeof(eBool));
	eMemSet(m_instrumentPatternTrack, 0, TF_MAX_INPUTS * sizeof(eU32));
	eMemSet(m_lastEvents, 0, tfSong::MAX_SEQ_TRACKS * tfSong::MAX_PATTERN_TRACKS * sizeof(tfSong::NoteEvent));

    m_outputSignal[0] = new eSignal[TF_BLOCKSIZE];
    m_outputSignal[1] = new eSignal[TF_BLOCKSIZE];
	m_tempSignal[0] = new eSignal[TF_BLOCKSIZE];
    m_tempSignal[1] = new eSignal[TF_BLOCKSIZE];
    m_outputFinal = new eS16[TF_BLOCKSIZE*2];

    _startThread();
    m_soundOut->play();
}

tfPlayer::~tfPlayer()
{
    m_soundOut->stop();
    _stopThread();

    clearInstruments();

    eSAFE_DELETE_ARRAY(m_outputSignal[0]);
    eSAFE_DELETE_ARRAY(m_outputSignal[1]);
	eSAFE_DELETE_ARRAY(m_tempSignal[0]);
    eSAFE_DELETE_ARRAY(m_tempSignal[1]);
    eSAFE_DELETE_ARRAY(m_outputFinal);
}

void tfPlayer::addInstrument(eU32 index)
{
    eASSERT(index >= 0 && index < TF_MAX_INPUTS);

    if (m_instruments[index] == eNULL)
    {
        tfInstrument *instr = new tfInstrument();
        eASSERT(instr != eNULL);
		
        m_instruments[index] = instr;
    }
}

void tfPlayer::removeInstrument(eU32 index)
{
    eASSERT(index >= 0 && index < TF_MAX_INPUTS);
    eSAFE_DELETE(m_instruments[index]);
}

tfInstrument * tfPlayer::getInstrument(eU32 index)
{
    eASSERT(index >= 0 && index < TF_MAX_INPUTS);
    return m_instruments[index];
}

#ifndef TF3_VSTI

void tfPlayer::loadInstruments(eDataStream &stream)
{
    clearInstruments();

    for (eU8 i=0; i<TF_MAX_INPUTS; i++)
    {
        if (stream.readByte())
        {
            tfInstrument *instr = new tfInstrument;
            eASSERT(instr != eNULL);
            m_instruments[i] = instr;
        }
    }

    for (eU32 j=0; j<TF_PARAM_COUNT; j++)
    {
        for (eU8 i=0; i<TF_MAX_INPUTS; i++)
        {
            if (m_instruments[i])
            {
                const eU8 value_range = TF_PARAM_VALUERANGE[j];
                eF32 p = 0.0f;

                if (value_range == 0)
                {
                    p = (eF32)stream.readByte()/100.0f;
                }     
                else
                {
                    p = (eF32)stream.readByte()/value_range;
                }

                m_instruments[i]->setParam(j, p);
            }
        }
    }

    for (eU8 i=0; i<TF_MAX_INPUTS; i++)
    {
        if (m_instruments[i])
            m_instruments[i]->updateAddSynth();
    }
    
}

void tfPlayer::storeInstruments(eDataStream &stream) const
{
    for (eU32 i=0; i<TF_MAX_INPUTS; i++)
    {
        stream.writeByte(m_instruments[i] == eNULL ? 0 : 1);
    }

    // Store the instruments.
    for (eU32 j=0; j<TF_PARAM_COUNT; j++)
    {
        for (eU32 i=0; i<TF_MAX_INPUTS; i++)
        {
            const tfInstrument *instr = m_instruments[i];

            if (instr)
            {
                const eU8 value_range = TF_PARAM_VALUERANGE[j];
                eU8 p = 0;

                if (value_range == 0)
                {
                    p = eFtoL(eRound(instr->getParam(j)*100.0f));
                }     
                else
                {
                    p = eFtoL(eRound(instr->getParam(j)*value_range));
                }

                stream.writeByte(p);
            }
        }
    }
}

#endif

void tfPlayer::clearInstruments()
{
	m_criticalSection.enter();

    for (eU32 i=0; i<TF_MAX_INPUTS; i++)
    {
        eSAFE_DELETE(m_instruments[i]);
    }

	m_criticalSection.leave();
}

void tfPlayer::play(eF32 time)
{
    m_playing = eTRUE;
    setTime(time);
    m_row = -1;
}

void tfPlayer::stop()
{
    m_playing = eFALSE;
    m_allNotesOff = eTRUE;
}

static FILE *s_out;

void tfPlayer::process()
{
	//s_out = fopen("c:\\dev\\tf3.raw", "wb");

    while (!m_joinRequest)
    {
        _processAudio();
        eSleep(1);
    }

	//fclose(s_out);
}

void tfPlayer::allNotesOff()
{
    m_criticalSection.enter();

    for(eU32 i=0; i<TF_MAX_INPUTS; i++)
    {
        tfInstrument *tf = m_instruments[i];

        if (tf)
        {
            tf->allNotesOff();
        }
    }

    m_criticalSection.leave();
}

void tfPlayer::setVolume(eF32 volume)
{
    m_volume = volume;
}

void tfPlayer::panic()
{
    m_criticalSection.enter();

    for (eU32 i=0; i<TF_MAX_INPUTS; i++)
    {
        tfInstrument *tf = m_instruments[i];

        if (tf)
        {
            tf->panic();
        }
    }

    m_criticalSection.leave();
}

void tfPlayer::mute(eBool on)
{
    m_mute = on;
}

void tfPlayer::noteEvent(eU32 instrument, eU32 note, eU32 velocity, eU32 modslot, eF32 mod)
{
    eASSERT(instrument >= 0 && instrument < TF_MAX_INPUTS);

    tfInstrument *instr = m_instruments[instrument];

    if (instr)
    {
        if (velocity > 0)
        {
            instr->noteOn(note, velocity, modslot, mod);
        }
        else
        {
            instr->noteOff(note);
        }
    }
}

void tfPlayer::setSong(tfSong *song)
{
    m_song = song;
}

tfSong * tfPlayer::getSong()
{
    return m_song;
}

void tfPlayer::setSampleRate(eU32 sampleRate)
{
    m_sampleRate = sampleRate;
}

void tfPlayer::setTime(eF32 time)
{
    eASSERT(time >= 0.0f);
    allNotesOff();
    m_time = eFtoL(time * m_soundOut->getSampleRate());
}

void tfPlayer::setPosition(eU32 row)
{
    m_row = row;
}

void tfPlayer::setLoop(eU32 startRow, eU32 endRow)
{
	m_loopStartRow = startRow;
	m_loopEndRow = endRow;
}

eU32 tfPlayer::getSampleRate() const
{
    return m_sampleRate;
}

eF32 tfPlayer::getTime() const
{
    return (eF32)m_time / m_soundOut->getSampleRate();
}

eU32 tfPlayer::getPosition(eU32 row) const
{
    return m_row;
}

eF32 tfPlayer::getPeakInstr(eU32 instr) const
{
    eASSERT(instr < TF_MAX_INPUTS);
    m_criticalSection.enter();
    const eF32 peak = m_peakInstr[instr];
    m_criticalSection.leave();
    return peak;
}

eF32 tfPlayer::getPeakTrack(eU32 track) const
{
    eASSERT(track < tfSong::MAX_SEQ_TRACKS);
    m_criticalSection.enter();
    const eF32 peak = m_peakTrack[track];
    m_criticalSection.leave();
    return peak;
}

eF32 tfPlayer::getMasterPeak() const
{
    m_criticalSection.enter();
    const eF32 peak = m_masterPeak;
    m_criticalSection.leave();
    return peak;
}

void tfPlayer::_processRow()
{
    for (eU32 i=0; i<m_song->getSeqTrackCount(); i++)
    {
#ifdef eEDITOR
        if (m_song->getMuted(i))
        {
            if (!m_muted[i])
            {
                const tfSong::PatternInstance *pi = m_song->getPatternInstance(i, m_row);
				if (pi)
				{
					const eU32 patternRow = m_row-pi->rowOffset;
					const tfSong::Pattern &pattern = *pi->pattern;

					for (eU32 j=0; j<pattern.getTrackCount(); j++)
					{
						const tfSong::NoteEvent *prevev = &m_lastEvents[i][j];

						if (prevev->noteOct)
						{
							tfInstrument *prev_instr = m_instruments[prevev->instrument];

							if (prev_instr)
							{
								prev_instr->noteOff(prevev->noteOct-1);
								m_lastEvents[i][j] = tfSong::NoteEvent();
							}
						}
					}
				}

                m_muted[i] = eTRUE;
            }

            continue;
        }

        m_muted[i] = eFALSE;
#endif

        const tfSong::PatternInstance *pi = m_song->getPatternInstance(i, m_row);
        
        if (pi)
        {
            const eU32 patternRow = m_row-pi->rowOffset;
            const tfSong::Pattern &pattern = *pi->pattern;

            for (eU32 j=0; j<pattern.getTrackCount(); j++)
            {
                const tfSong::Track &track = pattern.getTrack(j);
                const tfSong::NoteEvent &ev = track[patternRow];

                if (ev.noteOct && ev.instrument >= 0)
                {
                    tfInstrument *instr = m_instruments[ev.instrument];

                    if (instr)
                    {
                        const tfSong::NoteEvent *prevev = &m_lastEvents[i][j];

                        if (prevev->noteOct)
                        {
                            tfInstrument *prev_instr = m_instruments[prevev->instrument];

                            if (prev_instr)
                            {
                                prev_instr->noteOff(prevev->noteOct-1);
								m_lastEvents[i][j] = tfSong::NoteEvent();
                            }
                        }

                        if (ev.noteOct != 0x80)
                        {
                            const eU32 vel = (ev.velocity < 0 ? 128 : ev.velocity);
                            instr->noteOn(ev.noteOct-1, vel, ev.effectHi, (eF32)ev.effectLo/128.0f);
							
							m_lastEvents[i][j] = ev;
							m_instrumentPatternTrack[ev.instrument] = i;
                        }
                    }
                }
                else if (ev.noteOct == 0x80)
                {
                    const tfSong::NoteEvent *prevev = &m_lastEvents[i][j];

                    if (prevev->noteOct)
					{
						tfInstrument *prev_instr = m_instruments[prevev->instrument];

						if (prev_instr)
						{
							prev_instr->noteOff(prevev->noteOct-1);
							m_lastEvents[i][j] = tfSong::NoteEvent();
						}
					}
                }
            }
        }
    }
}

void tfPlayer::_processAudio()
{
    if (m_soundOut == eNULL)
    {
        return;
    }

    while(!m_soundOut->isFilled())
    {
        if (m_playing && m_song)
        {
            eF32 ftime = getTime();
            eU32 newRow = m_song->timeToRow(ftime) % m_song->getLengthInRows();

            if (newRow != m_row)
            {
                m_row = newRow;

				if (m_loopStartRow != m_loopEndRow)
				{
					if (m_row >= (eInt)m_loopEndRow)
					{
						const eF32 startTime = m_song->rowToTime(m_loopStartRow);
						const eF32 endTime = m_song->rowToTime(m_loopEndRow);

						m_row = m_loopStartRow;
 						ftime -= endTime - startTime;

						if (ftime < 0.0f)
							ftime = 0.0f;

                        m_time = eFtoL(ftime * m_soundOut->getSampleRate());
					}
				}

                m_criticalSection.enter();
                _processRow();
                m_criticalSection.leave();
            }
        }

        if (m_allNotesOff)
        {
            allNotesOff();
            m_allNotesOff = eFALSE;
        }

        eMemSet(m_outputSignal[0], 0, sizeof(eSignal)*TF_BLOCKSIZE);
        eMemSet(m_outputSignal[1], 0, sizeof(eSignal)*TF_BLOCKSIZE);

        m_criticalSection.enter();

        for(eU32 j=TF_PLAYER_PEAK_MEMORY-1; j>0; j--)
        {
            m_masterPeakMemory[j] = m_masterPeakMemory[j - 1];
        }
        m_masterPeakMemory[0] = 0.0f;

	    for (eU32 i=0; i<tfSong::MAX_SEQ_TRACKS; i++)
        {
		    m_peakTrack[i] = 0.0f;
        }

        for (eU32 i=0; i<TF_MAX_INPUTS; i++)
        {
            tfInstrument *instr = m_instruments[i];
		    eU32 track = m_instrumentPatternTrack[i];
		    eF32 peak = 0.0f;

            const eU32 peak_mem_offset = i * TF_PLAYER_PEAK_MEMORY;
            for(eU32 j=TF_PLAYER_PEAK_MEMORY-1; j>0; j--)
            {
                m_peakInstrMemory[peak_mem_offset + j] = m_peakInstrMemory[peak_mem_offset + j - 1];
            }
            m_peakInstrMemory[peak_mem_offset] = 0.0f;
            
            if (instr)
            {
				eMemSet(m_tempSignal[0], 0, sizeof(eSignal)*TF_BLOCKSIZE);
				eMemSet(m_tempSignal[1], 0, sizeof(eSignal)*TF_BLOCKSIZE);

                peak = instr->process(m_tempSignal, TF_BLOCKSIZE);

				eSignalMix(m_outputSignal, m_tempSignal, TF_BLOCKSIZE, 1.0f);
            }

            if (peak > m_masterPeakMemory[0])
                m_masterPeakMemory[0] = peak;

            m_peakInstrMemory[peak_mem_offset] = peak;
		    m_peakTrack[track] += peak;

            for(eU32 j=0; j<TF_PLAYER_PEAK_MEMORY; j++)
            {
                m_peakInstr[i] += m_peakInstrMemory[peak_mem_offset + j];
            }

            m_peakInstr[i] /= TF_PLAYER_PEAK_MEMORY;
        }

        m_masterPeak = 0.0f;
        for(eU32 j=0; j<TF_PLAYER_PEAK_MEMORY; j++)
        {
            m_masterPeak += m_masterPeakMemory[j];
        }

        m_masterPeak /= TF_PLAYER_PEAK_MEMORY;

        m_criticalSection.leave();

        if (m_mute)
        {
            eMemSet(m_outputFinal, 0, TF_BLOCKSIZE*2*sizeof(eS16));
        }
        else
        {
            eSignalToS16(m_outputSignal, m_outputFinal, 5000.0f*m_volume, TF_BLOCKSIZE);
        }

        m_soundOut->fill((const eU8 *)m_outputFinal, TF_BLOCKSIZE*2*sizeof(eS16));

		//fwrite(m_outputFinal, TF_BLOCKSIZE*2*sizeof(eS16), 1, s_out);

        if (m_playing && m_song)
        {
            m_time += TF_BLOCKSIZE;
        }
    }
}

void tfPlayer::_startThread()
{
    if (m_threadHandle)
    {
        return;
    }

    m_joinRequest = eFALSE;

    m_threadHandle = eThreadStart(_threadProc, this, eTRUE);
    eASSERT(m_threadHandle != eNULL);
}

void tfPlayer::_stopThread()
{
    m_allNotesOff = eTRUE;
    m_joinRequest = eTRUE;

    eThreadEnd(m_threadHandle, eTRUE);
    m_threadHandle = eNULL;
}

void tfPlayer::_threadProc(ePtr arg)
{
    tfPlayer *player = (tfPlayer *)arg;
    eASSERT(player != eNULL);

    player->process();
}

eCriticalSection * tfPlayer::getCriticalSection()
{
    return &m_criticalSection;
}

#endif