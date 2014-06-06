#include "RtAudioWrapper.h"

struct CallbackData 
{
	std::deque<AudioData> *v_audiodata;
	std::mutex *mutex;
	std::condition_variable *condition_variable;
	bool *cond_notify;
};
CallbackData callback_data_;

RtAudioWrapper::RtAudioWrapper()
{
	rtaudio_ = std::shared_ptr<RtAudio>(new RtAudio);
	device_id_ = rtaudio_->getDefaultInputDevice();
	num_channels_ = 2;
	sample_rate_ = 44100;
	init();
}

RtAudioWrapper::~RtAudioWrapper()
{
	if (rtaudio_ != NULL)
	{
		rtaudio_->stopStream();
		if (rtaudio_->isStreamOpen()) 
			rtaudio_->closeStream();
	}
}

void RtAudioWrapper::init()
{
	cond_notify_ = false;
	buffer_frames_ = AUDIOBUFFERLEN;
	callback_data_.v_audiodata = &v_audiodata_;
	callback_data_.mutex = &mutex_;
	callback_data_.condition_variable = &condition_variable_;
	callback_data_.cond_notify = &cond_notify_;
}

RtAudioWrapper::RtAudioWrapper(int device_id, int num_channel, int sample_rate)
{
	rtaudio_ = std::shared_ptr<RtAudio>(new RtAudio);
	device_id_ = device_id;
	num_channels_ = num_channel;
	sample_rate_ = sample_rate;
	init();
}

bool RtAudioWrapper::grab()
{
	if (!rtaudio_->isStreamOpen())
	{
		return false;
	}
	{
		std::unique_lock<std::mutex> lock(mutex_);
		while(!cond_notify_) // used to avoid spurious wakeups 
		{
			condition_variable_.wait(lock);
		}
	}
	std::unique_lock<std::mutex> lock(mutex_);
	if (v_audiodata_.empty())
	{
		is_grabed_ = false;
		cond_notify_ = false;
		return is_grabed_;
	}
	audio_data_ = v_audiodata_.front();
	v_audiodata_.pop_front();
	is_grabed_ = true;
	cond_notify_ = false;
	return is_grabed_;
}

bool RtAudioWrapper::retrieve(AudioData &audio_data)
{
	if (!rtaudio_->isStreamOpen())
	{
		return false;
	}
	if (is_grabed_)
	{
		audio_data = audio_data_;
		return is_grabed_;
	}
	return is_grabed_;
}

static int callback_func(void *output_buffer, void *input_buffer, unsigned int num_bufferframes, 
		double stream_time, RtAudioStreamStatus status, void *user_data)
{
	CallbackData *c_data = (CallbackData*)user_data;
	std::unique_lock<std::mutex> lock(*(c_data->mutex));
	std::deque<AudioData> *v_audiodata = c_data->v_audiodata;
	std::condition_variable *condition_variable = c_data->condition_variable;
	bool* cond_notify = c_data->cond_notify;
	AudioData audio_data;
	memcpy(audio_data.data, input_buffer, sizeof(unsigned short) * num_bufferframes);
	audio_data.stream_time = stream_time;
	v_audiodata->push_back(audio_data);
	*cond_notify = true;
	condition_variable->notify_one();
	return 0;
}

bool RtAudioWrapper::open(const int divice_id)
{
	assert(rtaudio_ != NULL);
	RtAudio::StreamParameters parameters;
	parameters.deviceId = divice_id;
	parameters.nChannels = num_channels_;
	parameters.firstChannel = 0;
	rtaudio_->openStream(NULL, &parameters, RTAUDIO_SINT16, sample_rate_, &buffer_frames_, &callback_func, (void*)&callback_data_);
	if (rtaudio_->isStreamOpen())
	{
		rtaudio_->startStream();
		return true;
	}
	return false;
}

void RtAudioWrapper::list_devices()
{
	if (rtaudio_ == NULL)
	{
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