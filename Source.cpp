#include "RtAudioWrapper.h"

#include <iostream>
#include <cstdlib>
#include <cstring>

int main()
{
	RtAudioWrapper rtaudio;
	//rtaudio.list_devices
	if (!rtaudio.open(0))
	{
		printf("open %d error\n", 0);
		exit(-1);
	}
	while (true)
	{
		if (!rtaudio.grab())
		{
			continue;
		}
		AudioData data;
		if (rtaudio.retrieve(data))
		{
			printf("%lf\n", data.stream_time);
		}
	}
	system("pause");
	return 0;
}