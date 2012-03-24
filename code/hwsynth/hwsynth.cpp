
#include "hwsynth.hpp"
#include "eventmapping.hpp"

#define HAVE_UI false

const eU32 BLOCKSIZE = 256;

static eBool 		m_stop = eFALSE;
static eCriticalSection m_CS;
static eS16 *		m_outputFinal;
static eSignal * 	m_outputSignal[2];
static tfMidi 		m_Midi;
static tfInstrument 	m_TF;
static tfAlsa 		m_Snd;
static tfDisplay	m_Display;

void sighandler(int dum)
{
       	m_stop = eTRUE;
}

void audioThread(void *arg)
{
	m_Snd.play();

	while(!m_stop)
	{
		if (m_Snd.getPlayBufferlen() >= BLOCKSIZE)
		{    
			m_Snd.output((const eU8 *)m_outputFinal, BLOCKSIZE);
			
			eMemSet(m_outputSignal[0], 0, sizeof(eSignal) * BLOCKSIZE);
	    		eMemSet(m_outputSignal[1], 0, sizeof(eSignal) * BLOCKSIZE);

	    		m_CS.enter();
		    	m_TF.process(m_outputSignal, BLOCKSIZE);
	    		m_CS.leave();

			eSignalToS16(m_outputSignal, m_outputFinal, 20000.0f, BLOCKSIZE);

			if (HAVE_UI)
			    m_Display.setScopes(m_outputFinal, BLOCKSIZE);
		}
		else
			eSleep(10);
	}

	m_Snd.closeDevice();
}

void videoThread(void *arg)
{
	while(!m_stop)
	{
		if (!m_Display.process())
			m_stop = true;

		eSleep(1000);
	}
}

void loadInstrument(const eChar *file)
{
	eChar name[64];
	eChar param[64];

	printf("Loading program: %s\n", file);
	FILE *in = fopen(file, "r+");	
	if (!in)
	{
		printf("Could not open file!\n");
		return;
	}

	fscanf(in, "%s", name);
	printf("Program name: %s\n", name);

	while(!feof(in))
	{
		fscanf(in, "%s", param);

		for(eU32 i=0;i<strlen(param);i++)
		{
			if (param[i] == ';')
			{	
				param[i] = '\0';
				eF32 value = (eF32)atof(&param[i+1]);

				for(eU32 j=0;j<TF_PARAM_COUNT;j++)
				{
					if (strcmp(TF_NAMES[j], param) == 0)
					{
						m_TF.setParam(j, value);
					}
				}
			}			
		}
	}

	printf("\n");

	fclose(in);
}

int main(int argc,char** argv)
{
	if (!m_Snd.openDevice(argv[2], 44100, 16, 2))
	{
		fprintf(stderr,"Could not open sound device\n");
		return -1;
	}

	if (!m_Midi.openDevice(argv[1])) 
	{
		fprintf(stderr,"Could not open midi device\n");
		return -1;
	}

	if (HAVE_UI)
	{
	    if (!m_Display.openDisplay()) 
	    {
		fprintf(stderr,"Could not open display\n");
		return -1;
	    }
	}

        m_Display.setDisplayString("LP Cutoff");
        m_Display.setDisplayValue(0.5);

	loadInstrument(argv[3]);

	m_outputSignal[0] = new eSignal[BLOCKSIZE];
	m_outputSignal[1] = new eSignal[BLOCKSIZE];
	m_outputFinal = new eS16[BLOCKSIZE * 2];

	signal(SIGINT,sighandler);

	ePtr threadA = eThreadStart(audioThread, eNULL, eTRUE);
	
	ePtr threadV;
	if (HAVE_UI)
	    threadV = eThreadStart(videoThread, eNULL, eFALSE);

        while (!m_stop) 
	{
		tfMidi::Event ev = m_Midi.readEvent();
		
		switch(ev.type)
		{
			case tfMidi::TYPE_NOTEON:
				{
					printf("NoteOn: %02x Velocity: %02x\n", ev.data1, ev.data2/2+64);

					m_CS.enter();
					if (ev.data2 > 0)
						m_TF.noteOn(ev.data1, ev.data2, 0, 0);
					else
						m_TF.noteOff(ev.data1);
					m_CS.leave();
					break;
				}
			case tfMidi::TYPE_NOTEOFF:
				{
					printf("NoteOff: %02x Velocity: %02x\n", ev.data1, ev.data2);

					m_CS.enter();
					m_TF.noteOff(ev.data1);
					m_CS.leave();
					break;
				}
			case tfMidi::TYPE_CONTROL:
				{
					printf("Type: %x Channel: %x ", ev.type, ev.channel);
					printf("Control: %02x Value: %02x", ev.data1, ev.data2);
			
					eF32 fvalue = (eF32)ev.data2 / 127;

					eU32 i=0;
					while(true)
					{
						if (m_EvMapping[i].control == -1)
							break;

						if (m_EvMapping[i].control == ev.data1)
						{
							printf(" Target: %s", m_EvMapping[i].name);

							m_CS.enter();
							m_TF.setParam(m_EvMapping[i].param, fvalue);
							m_CS.leave();

							m_Display.setDisplayString(m_EvMapping[i].name);
							m_Display.setDisplayValue(fvalue);

							break;
						}

						i++;
					}

					printf("\n");
					break;
				}
		}

		eSleep(10);
        }

	eThreadEnd(threadA, eTRUE);
	
	if (HAVE_UI)
	{
	    eThreadEnd(threadV, eTRUE);
	    m_Display.closeDisplay();
	}
	
	m_Midi.closeDevice();

	eSAFE_DELETE_ARRAY(m_outputSignal[0]);
	eSAFE_DELETE_ARRAY(m_outputSignal[1]);
	eSAFE_DELETE_ARRAY(m_outputFinal);

	return 0;
}

