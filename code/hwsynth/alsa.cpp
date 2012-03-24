
#include "hwsynth.hpp"

const eU32 BUFFERSIZE = 2048;
const eU32 PERIODSIZE = 64;

tfAlsa::tfAlsa() : m_handle(eNULL)
{
}

tfAlsa::~tfAlsa()
{
	closeDevice();
}

eBool tfAlsa::openDevice(const eChar *device, eU32 freq, eU32 bits, eU32 channels)
{
	eInt err = 0;

	if (device == eNULL)
		return eFALSE;

	printf ("opening device '%s' for playback\n", device);

	if ((err = snd_pcm_open (&m_handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) 
	{
		fprintf (stderr, "cannot open audio device '%s' for playback(%s)\n", device, snd_strerror (err));
		return eFALSE;
	}

	snd_pcm_hw_params_t *hw_params;
	snd_pcm_format_t format;
	snd_pcm_uframes_t buffer_size = BUFFERSIZE;
	snd_pcm_uframes_t period_size = PERIODSIZE;

	printf("setting device to %i hz %i bits %i channels\n", freq, bits, channels);

	if (bits == 16)
		format = SND_PCM_FORMAT_S16_LE;
	else
		format = SND_PCM_FORMAT_S8;

    	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) 
	{
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n", snd_strerror (err));
		return false;
	}

	if ((err = snd_pcm_hw_params_any (m_handle, hw_params)) < 0) 
	{
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n", snd_strerror (err));
		return false;
	}

	if ((err = snd_pcm_hw_params_set_access (m_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) 
	{
		fprintf (stderr, "cannot set access type (%s)\n", snd_strerror (err));
		return false;
	}

	if ((err = snd_pcm_hw_params_set_format (m_handle, hw_params, format)) < 0) 
	{
		fprintf (stderr, "cannot set sample format (%s)\n", snd_strerror (err));
		return false;
	}

	if ((err = snd_pcm_hw_params_set_rate_near (m_handle, hw_params, (unsigned int *)&freq, 0)) < 0) 
	{
		fprintf (stderr, "cannot set sample rate (%s)\n", snd_strerror (err));
		return false;
	}

	if ((err = snd_pcm_hw_params_set_channels (m_handle, hw_params, channels)) < 0) 
	{
		fprintf (stderr, "cannot set channel count (%s)\n", snd_strerror (err));
		return false;
	}

	if ((err = snd_pcm_hw_params_set_buffer_size_near (m_handle, hw_params, &buffer_size)) < 0) 
	{
		fprintf (stderr, "cannot set buffer size (%s)\n", snd_strerror (err));
		return false;
	}

	if ((err = snd_pcm_hw_params_set_period_size_near (m_handle, hw_params, &period_size, NULL)) < 0) 
	{
		fprintf (stderr, "cannot set period size (%s)\n", snd_strerror (err));
		return false;
	}

	if ((err = snd_pcm_hw_params (m_handle, hw_params)) < 0) 
	{
		fprintf (stderr, "cannot set parameters (%s)\n", snd_strerror (err));
		return false;
	}

	snd_pcm_hw_params_free (hw_params);

	return eTRUE;
}

void tfAlsa::closeDevice()
{
	if (m_handle == NULL)
		return;

    	snd_pcm_close (m_handle);
    	m_handle = NULL;
    	printf ("playback audio interface freed.\n");
}

eBool tfAlsa::play()
{
	eInt err = 0;

	if (m_handle == NULL)
		return eFALSE;

	if ((err = snd_pcm_prepare (m_handle)) < 0) 
	{
		fprintf (stderr, "cannot prepare playback audio interface for use (%s)\n", snd_strerror (err));
		return eFALSE;
	}

	if ((err = snd_pcm_wait (m_handle, 1000)) < 0) 
	{
		fprintf (stderr, "playback poll failed (%s)\n", strerror (errno));
		return eFALSE;
	}	 

	printf ("playback audio interface ready for use\n");
	return eTRUE;
}

eU32 tfAlsa::getPlayBufferlen()
{
	if (m_handle == NULL)
		return 0;

	eU32 len = snd_pcm_avail_update(m_handle);
	//printf("%i\n", len);
	return len;
}


eBool tfAlsa::output(const eU8* data, const eU32 length)
{
	eS32 err = 0;

	if (m_handle == NULL)
		return eFALSE;
			
	while(true)
	{
		err = snd_pcm_writei (m_handle, data, length); 
		if (err != length)
		{
			if (err == -32) // broken pipe
			{
				fprintf (stderr, "\nbroken pipe in playback audio interface. fixing...\n");
				snd_pcm_prepare(m_handle);
			}
			else
			{
				fprintf (stderr, "write to audio interface failed (%i) (%s)\n", err, snd_strerror (err));
				return eFALSE;
			}
		}
		else
		{
			//printf(".");
			fflush(stdout);
			return eTRUE;
		}
	}
}


