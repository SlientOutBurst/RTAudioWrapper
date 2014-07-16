#ifndef __RTAUDIOMICROPHONEWRAPER_H__
#define __RTAUDIOMICROPHONEWRAPER_H__

#include <assert.h>
#include <iostream>
#include <memory>

#include "RtAudio.h"
#include <boost/lockfree/spsc_queue.hpp>

typedef signed short MICROPHONE_MY_TYPE;
const unsigned int MICROPHONE_BUFF_FRAMES = 1024; // for WAV 16bit stream, this number should be 256, for AAC stream should be 1024
const int MICROPHONE_CHANNELS = 2;
const int MICROPHONE_AUDIOBUFFERLEN = MICROPHONE_BUFF_FRAMES * MICROPHONE_CHANNELS;
struct MicrophoneAudioData {
	MICROPHONE_MY_TYPE data[MICROPHONE_AUDIOBUFFERLEN];
	double stream_time;
};

class RtAudioMicrophoneWrapper {
public:
	typedef boost::lockfree::spsc_queue<MicrophoneAudioData, boost::lockfree::capacity<16>> lockfreequeue_type;
	/// \brief Constructor of RtAudioMicrophoneWrapper
	/// \return no return
	RtAudioMicrophoneWrapper();

	/// \brief Constructor of RtAudioMicrophoneWrapper
	/// \param device_id microphone device id
	/// \param num_channel sound channels
	/// \param sample_rate sound sample per seconds 44100 etc.
	/// \return no return
	RtAudioMicrophoneWrapper(int device_id, int num_channel, int sample_rate);

	/// \brief Destructor of RtAudioMicrophoneWrapper
	/// \return no return
	~RtAudioMicrophoneWrapper();

	/// \brief Open microphone device
	/// \param device_id device id
	/// \return true for success, false for fail
	bool open(const int divice_id);

	/// \brief list available devices
	/// \return no return
	void list_devices();

	/// \brief grab a sound sample from device
	/// \return true for success, false for fail
	bool grab();

	/// \brief retrieve a sound sample to audio data
	/// \param audio_data audio data content
	/// \return true for success, false for fail
	bool retrieve(MicrophoneAudioData &audio_data);

	/// \brief generate wav header for writing audio data to file
	/// \param data audio audio data
	/// \param len audio data length
	/// \return no return
	void generate_wav_header(char *data, const int len);

private:
	void init();
	RtAudioMicrophoneWrapper(const RtAudioMicrophoneWrapper&) {};
	RtAudioMicrophoneWrapper& operator=(const RtAudioMicrophoneWrapper&) {};

	std::shared_ptr<RtAudio> rtaudio_;

	// audio data stuff
	lockfreequeue_type v_audiodata_;
	MicrophoneAudioData audio_data_;

	// audio information stuff
	int device_id_;
	int num_channels_;
	int sample_rate_;
	unsigned int buffer_frames_; 
	bool is_grabed_;
};

#endif // !__RTAUDIOWRAPER_H__
