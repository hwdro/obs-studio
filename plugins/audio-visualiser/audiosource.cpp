/******************************************************************************
Copyright (C) 2015 by HomeWorld <homeworld@gmail.com>
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "audiosource.hpp"

#define ACQ_RETRY_TIMEOUT 1.0f

AudioSource::AudioSource(const string &sourceName_, uint32_t sampleRate_,
	uint32_t channels_, size_t windowSize_)
	: audioData(nullptr),
	  source(nullptr),
	  retryTimeout(ACQ_RETRY_TIMEOUT)
{
	pthread_mutex_init(&mutex, nullptr);
	Update(sourceName_, sampleRate_, channels_, windowSize_);
}

AudioSource::~AudioSource()
{
	if (source)
		ReleaseAudioSource();

	if (audioData)
		delete audioData;

	pthread_mutex_destroy(&mutex);
}

void AudioSource::Update(const string &sourceName_, uint32_t sampleRate_,
	uint32_t channels_, size_t windowSize_)
{
	sourceName = sourceName_;
	sampleRate = sampleRate_;
	windowSize = windowSize_;
	channels = channels_;

	if (audioData)
		delete audioData;

	audioData = new AudioData(sampleRate, channels, windowSize);
	AcquireAudioSource();
}


obs_source_t * AudioSource::GetAudioSource()
{
	return source;
}

AudioData * AudioSource::GetAudioData()
{
	return audioData;
}

void AudioSource::Tick(float seconds)
{
	if (!source) {
		retryTimeout -= seconds;
		if (retryTimeout < 0.0f) {
			AcquireAudioSource();
			retryTimeout = ACQ_RETRY_TIMEOUT;
		}
	}
}

int AudioSource::MutexTryLock()
{
	return pthread_mutex_trylock(&mutex);
}

int AudioSource::MutexLock()
{
	return pthread_mutex_lock(&mutex);
}

int AudioSource::MutexUnlock()
{
	return pthread_mutex_unlock(&mutex);
}

void AudioSource::SourceDataRecievedSignal(void *obj, calldata_t *calldata)
{
	AudioSource *audioSource = static_cast<AudioSource *>(obj);
	struct audio_data *data  = static_cast<audio_data *>
		(calldata_ptr(calldata, "data"));

	if (!audioSource)
		return;

	uint32_t channels;
	size_t   windowSize, frames, offset;
	
	if (audioSource->MutexTryLock() == EBUSY)
		return;

	frames     = data->frames;
	windowSize = audioSource->windowSize;
	offset     = windowSize - frames;
	
	audioSource->audioData->SetVolume(data->volume);

	if (frames > windowSize)
		return;

	channels = audioSource->GetAudioData()->GetChannels();

	for (uint32_t i = 0; i < channels; i++) {
		float *abuffer = audioSource->audioData->GetBuffers()[i];
		float *adata   = reinterpret_cast<float *>(data->data[i]);
		if (adata) {
			memmove(abuffer, abuffer + frames,
				offset * sizeof(float));

			memcpy(abuffer + offset, adata,
				frames * sizeof(float));
		}
	}
	audioSource->MutexUnlock();
}

void AudioSource::SourceRemovedSignal(void *obj, calldata_t *calldata)
{
	AudioSource *audioSource = static_cast<AudioSource *>(obj);
	if (!audioSource)
		return;
	audioSource->ReleaseAudioSource();
	UNUSED_PARAMETER(calldata);
}

void AudioSource::ReleaseAudioSource()
{
	if (!source)
		return;

	signal_handler_t *sh;

	sh = obs_source_get_signal_handler(source);

	MutexLock();
	signal_handler_disconnect(sh, "audio_data",
		AudioSource::SourceDataRecievedSignal, this);
	MutexUnlock();

	signal_handler_disconnect(sh, "remove",
		AudioSource::SourceRemovedSignal, this);

	obs_source_release(source);
	source = nullptr;
	sourceName.clear();
	//context->can_render = false;
	retryTimeout = ACQ_RETRY_TIMEOUT;
	

}

void AudioSource::AcquireAudioSource()
{
	bool global_source_found = false;
	obs_source_t *audio_source = nullptr;
	signal_handler_t *sh;

	for (uint32_t i = 1; i <= 10; i++) {
		obs_source_t *source = obs_get_output_source(i);
		if (source) {
			uint32_t flags = obs_source_get_output_flags(source);
			if (flags & OBS_SOURCE_AUDIO) {
				const char *name = obs_source_get_name(source);
				if (strcmp(name, sourceName.c_str()) == 0) {
					global_source_found = true;
					audio_source = source;
					break;
				}

			}
			obs_source_release(source);
		}
	}

	if (!global_source_found)
		audio_source = obs_get_source_by_name(sourceName.c_str());

	if (!audio_source)
		return;

	if (audio_source == source) {
		obs_source_release(audio_source);
		return;
	}

	ReleaseAudioSource();

	source = audio_source;

	sh = obs_source_get_signal_handler(audio_source);

	signal_handler_connect(sh, "audio_data",
		SourceDataRecievedSignal, this);
	signal_handler_connect(sh, "remove",
		SourceRemovedSignal, this);

}

bool AudioSource::IsSourceAcquired()
{
	return !!source;
}
