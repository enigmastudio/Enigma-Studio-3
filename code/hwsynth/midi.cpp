
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "../eshared/system/system.hpp"
#include "midi.hpp"

tfMidi::tfMidi() : m_handle(-1)
{
}

tfMidi::~tfMidi()
{
   	closeDevice();
}

eBool tfMidi::openDevice(const eChar *device)
{
	printf ("opening midi interface.\n");
	m_handle = open(device, O_RDONLY);
	return m_handle >= 0;
}

void tfMidi::closeDevice()
{
	if (m_handle >= 0)
	{
		close(m_handle);
		m_handle = -1;
		printf ("midi interface freed.\n");
	}
}

tfMidi::Event tfMidi::readEvent()
{
	tfMidi::Event ev;

	if (m_handle < 0)
		return ev;

	eU8 data;
	read(m_handle, &data, 1);

	ev.type = (EventType)(data >> 4);
	ev.channel = data & 0xf;

	switch(ev.type)
	{
		case TYPE_PROGCHANGE:
		case TYPE_CHANNEL_PRESSURE_AFTERTOUCH:
			{
				read(m_handle, &ev.data1, 1);
				ev.data2 = 0;
				break;
			}
		case TYPE_NOTEON:
		case TYPE_NOTEOFF:
		case TYPE_CONTROL:
		case TYPE_KEY_PRESSURE_AFTERTOUCH:
		case TYPE_PITCHWHEEL:
			{
				read(m_handle, &ev.data1, 1);
				read(m_handle, &ev.data2, 1);	
				break;
			}
		case TYPE_SYSTEM:
			{
				switch (ev.channel)
				{
					case 0: 	// custom message
					{
						data = 0;
						while (data != 247)
						{
							read(m_handle, &data, 1);
						}
						break;
					}
					case 2: 	// song position
					{
						read(m_handle, &ev.data1, 1);
						read(m_handle, &ev.data2, 1);	
						break;
					}
					case 3: 	// song select
					{
						read(m_handle, &ev.data1, 1);
						break;
					}
					default:
						break;
				}
	
				break;
			}
	}

	return ev;
}



