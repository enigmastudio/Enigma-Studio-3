
#ifndef TF_ALSA_HPP
#define TF_ALSA_HPP

class tfAlsa
{
public:
	tfAlsa();
	~tfAlsa();

	eBool 	openDevice(const eChar *device, eU32 freq, eU32 bits, eU32 channels);
	void 	closeDevice();
	eBool	play();
	eU32	getPlayBufferlen();
	eBool 	output(const eU8* data, const eU32 length);

private:
	snd_pcm_t * m_handle;
};

#endif
