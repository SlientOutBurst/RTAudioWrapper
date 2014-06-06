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
	buffer_frames_ = BUFF_FRAMES;
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
	memcpy(audio_data.data, input_buffer, AUDIOBUFFERLEN*2);
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

struct WAVHeader
{	
	char riff_data[4]; // 'riff' 4
	int file_length_8; // 4
	char wave_data[4]; // 'wave' // 4
	char fmt_data[4]; // 4
	int fmtlength; // Length of the fmt data (16 bytes) // 4
	short fmt_tag;  // Format tag: 1 = PCM // 2
	short channels;     // Channels: 1 = mono, 2 = stereo // 2
	int sample_rate;  // Samples per second: e.g., 44100 // 4
	int bytes_per_second; // sample rate * block align // 4
	short block_align;  // channels * bits/sample / 8 //2 
	short bits_per_sample;  // 8 or 16 // 2
	char data_data[4]; // 'data' // 4
	int data_block_len; // 4
};

void RtAudioWrapper::generate_wav_header(char *data, const int len)
{
	WAVHeader wav_header;
	printf("%d\n", sizeof(wav_header));
	wav_header.riff_data[0] = 'R';
	wav_header.riff_data[1] = 'I';
	wav_header.riff_data[2] = 'F';
	wav_header.riff_data[3] = 'F';

	wav_header.wave_data[0] = 'W';
	wav_header.wave_data[1] = 'A';
	wav_header.wave_data[2] = 'V';
	wav_header.wave_data[3] = 'E';

	wav_header.fmt_data[0] = 'f';
	wav_header.fmt_data[1] = 'm';
	wav_header.fmt_data[2] = 't';
	wav_header.fmt_data[3] = ' ';

	wav_header.fmtlength = 0x00000010;
	wav_header.fmt_tag = 1;
	wav_header.channels = 2;
	wav_header.sample_rate = 44100;
	wav_header.bits_per_sample = (short)16;
	wav_header.block_align = (short)(wav_header.channels * wav_header.bits_per_sample / 8);
	wav_header.bytes_per_second = wav_header.sample_rate * wav_header.block_align;

	wav_header.data_data[0] = 'd';
	wav_header.data_data[1] = 'a';
	wav_header.data_data[2] = 't';
	wav_header.data_data[3] = 'a';

	wav_header.data_block_len = wav_header.bytes_per_second * 5;
	wav_header.file_length_8 = wav_header.data_block_len + 44 - 8;
	
	memcpy((char*)data, (char*)&wav_header, len);
}