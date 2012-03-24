
#ifndef TF_MIDI_HPP
#define TF_MIDI_HPP

class tfMidi
{
public:
	enum EventType
	{
		TYPE_NOTEOFF = 8,
		TYPE_NOTEON = 9,
		TYPE_KEY_PRESSURE_AFTERTOUCH = 10,
		TYPE_CONTROL = 11,
		TYPE_PROGCHANGE = 12,
		TYPE_CHANNEL_PRESSURE_AFTERTOUCH = 13,
		TYPE_PITCHWHEEL = 14,
		TYPE_SYSTEM = 15
	};

	struct Event
	{
		EventType type;
		eU8 channel;
		eU8 data1;
		eU8 data2;
	};

	tfMidi();
	~tfMidi();

	eBool 	openDevice(const eChar *device);
	void	closeDevice();
	Event	readEvent();

private:
	int m_handle;
};

#endif


