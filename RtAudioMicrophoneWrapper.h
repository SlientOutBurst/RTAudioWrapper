#ifndef __RTAUDIOMICROPHONEWRAPER_H__
#define __RTAUDIOMICROPHONEWRAPER_H__

#include "RtAudio.h"

#include <iostream>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <assert.h>

typedef signed short MICROPHONE_MY_TYPE;
const unsigned int MICROPHONE_BUFF_FRAMES = 1024; // for WAV 16bit stream, this number should be 256, for AAC stream should be 1024
const int MICROPHONE_CHANNELS = 2;
const int MICROPHONE_AUDIOBUFFERLEN = MICROPHONE_BUFF_FRAMES * MICROPHONE_CHANNELS;
struct MicrophoneAudioData
{
	MICROPHONE_MY_TYPE data[MICROPHONE_AUDIOBUFFERLEN];
	double stream_time;
};

class RtAudioMicrophoneWrapper
{
public:
	RtAudioMicrophoneWrapper();
	RtAudioMicrophoneWrapper(int device_id, int num_channel, int sample_rate);
	~RtAudioMicrophoneWrapper();

	bool open(const int divice_id);
	void list_devices();
	bool grab();
	bool retrieve(MicrophoneAudioData &audio_data);
	void init();
	void generate_wav_header(char *data, const int len);

private:
	RtAudioMicrophoneWrapper(const RtAudioMicrophoneWrapper&) {};
	RtAudioMicrophoneWrapper& operator=(const RtAudioMicrophoneWrapper&) {};

	std::shared_ptr<RtAudio> rtaudio_;

	// audio data stuff
	std::deque<MicrophoneAudioData> v_audiodata_;
	MicrophoneAudioData audio_data_;
	std::mutex mutex_;
	std::condition_variable condition_variable_;
	bool cond_notify_;

	// audio information stuff
	int device_id_;
	int num_channels_;
	int sample_rate_;
	unsigned int buffer_frames_; 
	bool is_grabed_;
};

#endif // !__RTAUDIOWRAPER_H__
