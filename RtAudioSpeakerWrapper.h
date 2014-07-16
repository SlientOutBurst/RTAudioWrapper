#ifndef __RTAUDIOWSPEAKERRAPER_H__
#define __RTAUDIOWSPEAKERRAPER_H__

#include "RtAudio.h"

#include <iostream>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <assert.h>

typedef unsigned short SPERKER_MY_TYPE;
const unsigned int SPERKER_BUFF_FRAMES = 1024; // for WAV 16bit stream, this number should be 256, for AAC stream should be 1024
const int SPERKER_CHANNELS = 2;
const int SPERKER_AUDIOBUFFERLEN = SPERKER_BUFF_FRAMES * SPERKER_CHANNELS;
struct SpeakerAudioData
{
	SPERKER_MY_TYPE data[SPERKER_AUDIOBUFFERLEN];
	double stream_time;
};

class RtAudioSpeakerWrapper
{
public:
	RtAudioSpeakerWrapper();
	RtAudioSpeakerWrapper(int device_id, int num_channel, int sample_rate);
	~RtAudioSpeakerWrapper();

	bool open(const int divice_id);
	void list_devices();
	bool play(SpeakerAudioData &audio_data, int len);
	void init();

private:
	RtAudioSpeakerWrapper(const RtAudioSpeakerWrapper&) {};
	RtAudioSpeakerWrapper& operator=(const RtAudioSpeakerWrapper&) {};

	std::shared_ptr<RtAudio> rtaudio_;

	// audio data stuff
	std::deque<SpeakerAudioData> v_audiodata_;
	SpeakerAudioData audio_data_;
	std::mutex mutex_;
	std::condition_variable condition_variable_;
	bool cond_notify_;

	// audio information stuff
	int device_id_;
	int num_channels_;
	int sample_rate_;
	unsigned int buffer_frames_; 
	bool is_played_;
};

#endif // !__RTAUDIOWRAPER_H__
