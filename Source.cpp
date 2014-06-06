#include "RtAudioWrapper.h"

#include <iostream>
#include <cstdlib>
#include <cstring>

int main()
{
	RtAudioWrapper rtaudio;
	rtaudio.list_devices();
	if (!rtaudio.open(0))
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
		AudioData data;
		if (rtaudio.retrieve(data))
		{
			if (data_len < 0)
			{
				break;
			}
			if (data_len - AUDIOBUFFERLEN < 0)
			{
				fwrite((MY_TYPE*)data.data, sizeof(MY_TYPE), data_len, p);
			}
			else
			{
				fwrite((MY_TYPE*)data.data, sizeof(MY_TYPE), AUDIOBUFFERLEN, p);
			}
			data_len -= AUDIOBUFFERLEN;
// 			printf("%d\n", data_len);
// 			printf("%lf\n", data.stream_time);
		}
	}
	fclose(p);
	system("pause");
	return 0;
}