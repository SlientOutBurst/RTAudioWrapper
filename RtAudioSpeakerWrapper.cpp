#include "RtAudioSpeakerWrapper.h"

struct SpeakerCallbackData {
	RtAudioSpeakerWrapper::lockfreequeue_type *v_audiodata;
};
static SpeakerCallbackData callback_data_;

RtAudioSpeakerWrapper::RtAudioSpeakerWrapper() {
	rtaudio_ = std::shared_ptr<RtAudio>(new RtAudio);
	device_id_ = rtaudio_->getDefaultOutputDevice();
	num_channels_ = 2;
	sample_rate_ = 44100;
	init();
}

RtAudioSpeakerWrapper::~RtAudioSpeakerWrapper() {
	while (!v_audiodata_.empty()) {
		continue;
	}
	if (rtaudio_) {
		rtaudio_->stopStream();
		if (rtaudio_->isStreamOpen()) {
			rtaudio_->closeStream();
		}
	}
}

void RtAudioSpeakerWrapper::init() {
	buffer_frames_ = SPERKER_BUFF_FRAMES;
	callback_data_.v_audiodata = &v_audiodata_;
}

RtAudioSpeakerWrapper::RtAudioSpeakerWrapper(int device_id, int num_channel, int sample_rate)
{
	rtaudio_ = std::shared_ptr<RtAudio>(new RtAudio);
	device_id_ = device_id;
	num_channels_ = num_channel;
	sample_rate_ = sample_rate;
	init();
}

bool RtAudioSpeakerWrapper::play(SpeakerAudioData &_audio_data, int len)
{
	if (!rtaudio_->isStreamOpen()) {
		return false;
	}

	if (len > 0) {
		SpeakerAudioData audio_data = _audio_data;
		while (!v_audiodata_.push(audio_data)) {
			continue;
		}
		return true;
	}
	else
	{
		if (v_audiodata_.empty() == true) {
			return false;
		}
	}
	return true;
}

static int callback_func(void *output_buffer, void *input_buffer, unsigned int num_bufferframes, 
						 double stream_time, RtAudioStreamStatus status, void *user_data)
{
	SpeakerCallbackData *c_data = (SpeakerCallbackData*)user_data;
	RtAudioSpeakerWrapper::lockfreequeue_type *v_audiodata = c_data->v_audiodata;
	SpeakerAudioData audio_data;
	while (!v_audiodata->pop(audio_data))
	{
		continue;
	}
	memcpy(output_buffer, audio_data.data, SPERKER_AUDIOBUFFERLEN*2);
	return 0;
}

bool RtAudioSpeakerWrapper::open(const int divice_id) {
	assert(rtaudio_);
	RtAudio::StreamParameters parameters;
	parameters.deviceId = divice_id;
	parameters.nChannels = num_channels_;
	parameters.firstChannel = 0;
	rtaudio_->openStream(&parameters, NULL, RTAUDIO_SINT16, sample_rate_, &buffer_frames_, &callback_func, (void*)&callback_data_);
	if (rtaudio_->isStreamOpen()) {
		rtaudio_->startStream();
		return true;
	}
	return false;
}

void RtAudioSpeakerWrapper::list_devices()
{
	if (!rtaudio_) {
		return;
	}
	RtAudio::DeviceInfo info;

	unsigned int devices = rtaudio_->getDeviceCount();
	std::cout << "\nFound " << devices << " device(s) ...\n";

	for (unsigned int i = 0; i < devices; i++) 
	{
		info = rtaudio_->getDeviceInfo(i);

		std::cout << "\nDevice Name = " << info.name << '\n';
		if ( info.probed == false )
			std::cout << "Probe Status = UNsuccessful\n";
		else {
			std::cout << "Probe Status = Successful\n";
			std::cout << "Output Channels = " << info.outputChannels << '\n';
			std::cout << "Input Channels = " << info.inputChannels << '\n';
			std::cout << "Duplex Channels = " << info.duplexChannels << '\n';
			if ( info.isDefaultOutput ) std::cout << "This is the default output device.\n";
			else std::cout << "This is NOT the default output device.\n";
			if ( info.isDefaultInput ) std::cout << "This is the default input device.\n";
			else std::cout << "This is NOT the default input device.\n";
			if ( info.nativeFormats == 0 )
				std::cout << "No natively supported data formats(?)!";
			else {
				std::cout << "Natively supported data formats:\n";
				if ( info.nativeFormats & RTAUDIO_SINT8 )
					std::cout << "  8-bit int\n";
				if ( info.nativeFormats & RTAUDIO_SINT16 )
					std::cout << "  16-bit int\n";
				if ( info.nativeFormats & RTAUDIO_SINT24 )
					std::cout << "  24-bit int\n";
				if ( info.nativeFormats & RTAUDIO_SINT32 )
					std::cout << "  32-bit int\n";
				if ( info.nativeFormats & RTAUDIO_FLOAT32 )
					std::cout << "  32-bit float\n";
				if ( info.nativeFormats & RTAUDIO_FLOAT64 )
					std::cout << "  64-bit float\n";
			}
			if ( info.sampleRates.size() < 1 )
				std::cout << "No supported sample rates found!";
			else {
				std::cout << "Supported sample rates = ";
				for (unsigned int j=0; j<info.sampleRates.size(); j++)
					std::cout << info.sampleRates[j] << " ";
			}
			std::cout << std::endl;
		}
	}
	std::cout << std::endl;
}
