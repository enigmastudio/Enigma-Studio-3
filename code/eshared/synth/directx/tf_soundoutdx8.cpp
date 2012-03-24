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

#include <windows.h>
#include <dsound.h>
#include <stdio.h>

#include "../../eshared.hpp"
#include "tf_soundoutdx8.hpp"

tfSoundOutDx8::tfSoundOutDx8(eU32 latency, eU32 sampleRate) :
    m_nextWriteOffset(0),
    m_bufferSize(0),
    m_ds(NULL),
    m_dsb(NULL),
    m_sampleRate(0)
{
    initialize(latency, sampleRate);
}

tfSoundOutDx8::~tfSoundOutDx8()
{
    shutdown();
}

eBool tfSoundOutDx8::initialize(eU32 latency, eU32 sampleRate)
{
    if (SUCCEEDED(DirectSoundCreate8(NULL, &m_ds, NULL)))
    {
        if (SUCCEEDED(m_ds->SetCooperativeLevel(GetForegroundWindow(), DSSCL_NORMAL)))
        {
            m_bufferSize = sampleRate*4*latency/1000; 

            PCMWAVEFORMAT pcmwf;
            eMemSet(&pcmwf, 0, sizeof(pcmwf));
            pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
            pcmwf.wf.nChannels = 2;
            pcmwf.wf.nSamplesPerSec = sampleRate;
            pcmwf.wf.nBlockAlign = 4;
            pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec*pcmwf.wf.nBlockAlign;
            pcmwf.wBitsPerSample = 16;

            DSBUFFERDESC dsbDesc;
            eMemSet(&dsbDesc, 0, sizeof(dsbDesc));
            dsbDesc.dwSize = sizeof(dsbDesc);
            dsbDesc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
            dsbDesc.dwBufferBytes = m_bufferSize;
            dsbDesc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;

            if (SUCCEEDED(m_ds->CreateSoundBuffer(&dsbDesc, (IDirectSoundBuffer **)&m_dsb, eNULL)))
            {
                m_sampleRate = sampleRate;
                return eTRUE;
            }
            else
            {
                eShowError("Couldn't create sound buffer!");
            }
        }
        else
        {
            eShowError("Couldn't set cooperative level for sound!");
        }
    }
    else
    {
        eShowError("Couldn't initialize DirectSound!");
    }


    return eFALSE;
}

void tfSoundOutDx8::shutdown()
{
    eSAFE_RELEASE_COM(m_dsb);
    eSAFE_RELEASE_COM(m_ds);
}

void tfSoundOutDx8::play()
{
    if (m_dsb == eNULL)
    {
        return;
    }

    HRESULT res = m_dsb->SetCurrentPosition(0);
    eASSERT(!FAILED(res));

    res = m_dsb->Play(0, 0, DSBPLAY_LOOPING);
    eASSERT(!FAILED(res));
}

void tfSoundOutDx8::stop()
{
    if (m_dsb)
    {
        const HRESULT res = m_dsb->Stop();
        eASSERT(!FAILED(res));
    }
}

void tfSoundOutDx8::clear()
{
    eASSERT(m_dsb != eNULL);

    ePtr write0, write1;
    DWORD length0, length1;

    eByteArray zeroData(m_bufferSize);
    eMemSet(&zeroData[0], 0, m_bufferSize);

    HRESULT hr = m_dsb->Lock(0, m_bufferSize, &write0, &length0, &write1, &length1, 0);

    if (hr == DSERR_BUFFERLOST)
    {
        hr = m_dsb->Restore();
        eASSERT(!FAILED(hr));
        hr = m_dsb->Lock(0, m_bufferSize, &write0, &length0, &write1, &length1, 0);
    }

    eASSERT(!FAILED(hr));
    eMemCopy(write0, &zeroData[0], length0);
    hr = m_dsb->Unlock(write0, length0, write1, length1);
    eASSERT(!FAILED(hr));
}

void tfSoundOutDx8::fill(const eU8 *data, eU32 count)
{
    eASSERT(m_dsb != eNULL);

    ePtr write0, write1;
    DWORD length0, length1;

    HRESULT hr = m_dsb->Lock(m_nextWriteOffset, count, &write0, &length0, &write1, &length1, 0);

    if (hr == DSERR_BUFFERLOST)
    {
        hr = m_dsb->Restore();
        eASSERT(!FAILED(hr));
        hr = m_dsb->Lock(0, count, &write0, &length0, &write1, &length1, 0);
    }

    eASSERT(!FAILED(hr));
    eMemCopy(write0, data, length0);

    if (write1)
    {
        eMemCopy(write1, data + length0, length1);
    }

    hr = m_dsb->Unlock(write0, length0, write1, length1);
    eASSERT(!FAILED(hr));

    m_nextWriteOffset += count;
    m_nextWriteOffset %= m_bufferSize;
}

eBool tfSoundOutDx8::isFilled() const
{
    eASSERT(m_dsb != eNULL);

    DWORD playCursor, writeCursor;
    m_dsb->GetCurrentPosition(&playCursor, &writeCursor);

    eU32 writeOffset = m_nextWriteOffset;
    
    if (writeOffset < playCursor)
    {
        writeOffset += m_bufferSize;
    }

    const eU32 minsize = m_bufferSize/8;
    const eU32 distance = writeOffset-playCursor;
    const eBool filled = (distance >= minsize);

    // START OF LOGGING CODE
    /*
    const eU32 DRAWLEN = 100;
    FILE *out = fopen("d:\\dev\\tf3audio.log", "ab");
    eU32 pc = playCursor * DRAWLEN / m_bufferSize;
    eU32 wc = m_nextWriteOffset * DRAWLEN / m_bufferSize;
    for(eU32 i=0;i<DRAWLEN;i++)
    {
        if (wc > pc)
        {
            if (i > pc && i < wc)
                fprintf(out, "#");
            else
                fprintf(out, ".");
        }
        else
        {
            if (i < wc || i > pc)
                fprintf(out, "#");
            else
                fprintf(out, ".");
        }
    }
        
    if (filled)
        fprintf(out, " OK ");
    else
        fprintf(out, "    ");

    fprintf(out, " %i / %i - W:%i - R:%i\n", distance, minsize, wc, pc);
    fclose(out);
    */
    // END OF LOGGING CODE 

    return filled;
}

eU32 tfSoundOutDx8::getSampleRate() const
{
    return m_sampleRate;
}