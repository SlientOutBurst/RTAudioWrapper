#include "RtAudioMicrophoneWrapper.h"
#include "RtAudioSpeakerWrapper.h"

#include <iostream>
#include <cstdlib>
#include <cstring>

void record()
{
	RtAudioMicrophoneWrapper rtaudio;
	rtaudio.list_devices();
	if (!rtaudio.open(5))
	{
		printf("open %d error\n", 0);
		exit(-1);
	}

	char header[44] = {0};
	rtaudio.generate_wav_header(header, 44);
	FILE *p = fopen("test.wav", "wb");
	fwrite(header, sizeof(char), 44, p);
	int data_len = (*((int*)(header + 40))) / 2;
	printf("data_len %d\n", data_len);
	//int data_len = 5 * 44100 * 2;
	while (true)
	{
		if (!rtaudio.grab())
		{
			continue;
		}
		MicrophoneAudioData data;
		if (rtaudio.retrieve(data))
		{
			if (data_len < 0)
			{
				break;
			}
			if (data_len - MICROPHONE_AUDIOBUFFERLEN < 0)
			{
				fwrite((MICROPHONE_MY_TYPE*)data.data, sizeof(MICROPHONE_MY_TYPE), data_len, p);
			}
			else
			{
				fwrite((MICROPHONE_MY_TYPE*)data.data, sizeof(MICROPHONE_MY_TYPE), MICROPHONE_AUDIOBUFFERLEN, p);
			}
			data_len -= MICROPHONE_AUDIOBUFFERLEN;
		}
	}
	fclose(p);
}

void play()
{
	RtAudioSpeakerWrapper rtaudio;
	rtaudio.list_devices();
	if (!rtaudio.open(0))
	{
		printf("open %d error\n", 0);
		exit(-1);
	}
	FILE *p = fopen("apologize.wav", "rb");
	char header[44];
	fread(header, sizeof(char), 44, p);
	SpeakerAudioData data;
	bool done_flag = false;
	while (true)
	{
		int len = fread(data.data, sizeof(char), SPERKER_AUDIOBUFFERLEN * sizeof(SPERKER_MY_TYPE), p);
		if (rtaudio.play(data, len) == false)
		{
			break;
		}
	}

	fclose(p);
}

int main()
{
	record();
	//play();
	
	return 0;
}