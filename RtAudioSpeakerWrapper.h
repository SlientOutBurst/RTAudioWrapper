#ifndef __RTAUDIOWSPEAKERRAPER_H__
#define __RTAUDIOWSPEAKERRAPER_H__

#include <assert.h>
#include <iostream>
#include <memory>

#include "RtAudio.h"
#include <boost/lockfree/spsc_queue.hpp>

typedef unsigned short SPERKER_MY_TYPE;
const unsigned int SPERKER_BUFF_FRAMES = 1024; // for WAV 16bit stream, this number should be 256, for AAC stream should be 1024
const int SPERKER_CHANNELS = 2;
const int SPERKER_AUDIOBUFFERLEN = SPERKER_BUFF_FRAMES * SPERKER_CHANNELS;
struct SpeakerAudioData {
	SPERKER_MY_TYPE data[SPERKER_AUDIOBUFFERLEN];
	double stream_time;
};

class RtAudioSpeakerWrapper
{
public:
	typedef boost::lockfree::spsc_queue<SpeakerAudioData, boost::lockfree::capacity<16>> lockfreequeue_type;
	/// \brief Constructor of RtAudioSpeakerWrapper
	/// \return no return
	RtAudioSpeakerWrapper();
	/// \brief Constructor of RtAudioSpeakerWrapper
	/// \param device_id microphone device id
	/// \param num_channel sound channels
	/// \param sample_rate sound sample per seconds 44100 etc.
	/// \return no return
	RtAudioSpeakerWrapper(int device_id, int num_channel, int sample_rate);

	/// \brief Destructor of RtAudioSpeakerWrapper
	/// \return no return
	~RtAudioSpeakerWrapper();

	/// \brief Open speaker device
	/// \param device_id device id
	/// \return true for success, false for fail
	bool open(const int divice_id);

	/// \brief list available devices
	/// \return no return
	void list_devices();

	/// \brief send audio data to speaker
	/// \param audio_data audio data
	/// \param len audio data len
	/// \return true for success, false for fail
	bool play(SpeakerAudioData &audio_data, int len);

private:
	void init();
	RtAudioSpeakerWrapper(const RtAudioSpeakerWrapper&) {};
	RtAudioSpeakerWrapper& operator=(const RtAudioSpeakerWrapper&) {};

	std::shared_ptr<RtAudio> rtaudio_;

	// audio data stuff
	lockfreequeue_type v_audiodata_;

	// audio information stuff
	int device_id_;
	int num_channels_;
	int sample_rate_;
	unsigned int buffer_frames_; 
	bool is_played_;
};

#endif // !__RTAUDIOWRAPER_H__
