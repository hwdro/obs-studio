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

#include <obs-module.h>
#include <string>
#include <util/threading.h>

#include "visualiser.hpp"


#define SETTINGS_AUDIO_SOURCE  "avis.audiosource"
#define TEXT_AUDIO_SOURCE_DESC obs_module_text("avis.audiosource.desc")

AudioVisSource::AudioVisSource(obs_data_t *settings, obs_source_t *source_)
	: source(source_),
	  audioSource(nullptr),
	  visual(nullptr)
{
	cx = 512;
	cy = 256;
	Update(settings);
}
AudioVisSource::~AudioVisSource()
{
	delete audioSource;
	delete visual;
}

void AudioVisSource::Update(obs_data_t *settings)
{
	struct obs_audio_info oai;
	string audioSourceName = obs_data_get_string(settings,
		SETTINGS_AUDIO_SOURCE);
	
	obs_get_audio_info(&oai);
	
	delete audioSource;
	delete visual;

	uint32_t sr, ch;
	size_t ws;

	sr = oai.samples_per_sec;
	ch = get_audio_channels(oai.speakers);
	ws = 512;

	audioSource = new AudioSource(audioSourceName,
		sr, ch, ws);
	
	visual = static_cast<Visual *>(new VisScope(cx, cy, sr, ch, ws));

}

uint32_t AudioVisSource::GetWidth()
{
	return cx;
}

uint32_t AudioVisSource::GetHeight()
{
	return cy;
}


bool AudioVisSource::EnumAudioSourcesCallback(void *param, 
	obs_source_t *source)
{
	obs_property_t *prop = static_cast<obs_property_t *>(param);
	string          name = obs_source_get_name(source);
	uint32_t       flags = obs_source_get_output_flags(source);

	if (flags & OBS_SOURCE_AUDIO)
		obs_property_list_add_string(prop, name.c_str(), name.c_str());
	
	return true;
}

static void GetAudioVisualiserDefaults(obs_data_t *settings)
{
	UNUSED_PARAMETER(settings);
}

obs_properties_t *AudioVisSource::Properties()
{
	obs_properties_t *props;
	obs_property_t   *prop;

	props = obs_properties_create();
	prop = obs_properties_add_list(props,
		SETTINGS_AUDIO_SOURCE, TEXT_AUDIO_SOURCE_DESC,
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);

	for (uint32_t i = 1; i <= 10; i++) {
		obs_source_t *source = obs_get_output_source(i);
		if (source) {
			EnumAudioSourcesCallback(prop, source);
			obs_source_release(source);
		}
	}

	obs_enum_sources(AudioVisSource::EnumAudioSourcesCallback, prop);
	return props;
}

void AudioVisSource::Tick(float seconds)
{
	if (audioSource)
		audioSource->Tick(seconds);
	if (visual) {
		if (audioSource) {
			bool acq = audioSource->IsSourceAcquired();
			if (acq) {
				if (audioSource->MutexTryLock() == EBUSY)
					goto NoAudioSource;
				visual->Update(audioSource->GetAudioData());
				audioSource->MutexUnlock();
			}
			
		}
NoAudioSource:
		visual->Tick(seconds);
	}
}

void AudioVisSource::Render(gs_effect_t *effect)
{
	if (visual)
		visual->Render(effect);
}

static void *CreateAudioVisualiserSource(obs_data_t *settings,
	obs_source_t *source)
{
	try {
		return new AudioVisSource(settings, source);
	} catch (const char *error) {
		blog(LOG_ERROR, "[CreateAudioVisSource] %s", error);
	}
	return nullptr;
}

static void DestroyAudioVisualiserSource(void *obj)
{
	delete static_cast<AudioVisSource *>(obj);
}

static void UpdateAudioVisualiserSource(void *obj, obs_data_t *settings)
{
	static_cast<AudioVisSource *>(obj)->Update(settings);

}

static obs_properties_t *GetAudioVisualiserSourceProperties(void *obj)
{
	return static_cast<AudioVisSource *>(obj)->Properties();
}

static const char *GetAudioVisualiserSourceName(void)
{
	return obs_module_text("avis.plugin.name");
}

static uint32_t GetAudioVisualiserWidth(void *obj)
{
	return static_cast<AudioVisSource*>(obj)->GetWidth();
}

static uint32_t GetAudioVisualiserHeight(void *obj)
{
	return static_cast<AudioVisSource*>(obj)->GetHeight();
}

static void AudioVisualiserSourceTick(void *obj, float seconds)
{
	static_cast<AudioVisSource *>(obj)->Tick(seconds);
}

static void AudioVisualiserSourceRender(void *obj, gs_effect *effect)
{
	static_cast<AudioVisSource *>(obj)->Render(effect);
}


void RegisterAudioVisualiserSource()
{
	obs_source_info info = {};
	info.id             = "audio_visualiser";
	info.type           = OBS_SOURCE_TYPE_INPUT;
	info.output_flags   = OBS_SOURCE_VIDEO;
	info.get_name       = GetAudioVisualiserSourceName;
	info.create         = CreateAudioVisualiserSource;
	info.destroy        = DestroyAudioVisualiserSource;
	info.update         = UpdateAudioVisualiserSource;
	info.get_defaults   = GetAudioVisualiserDefaults;
	info.get_properties = GetAudioVisualiserSourceProperties;
	info.get_height     = GetAudioVisualiserHeight;
	info.get_width      = GetAudioVisualiserWidth;
	info.video_tick     = AudioVisualiserSourceTick;
	info.video_render   = AudioVisualiserSourceRender;
	obs_register_source(&info);
}
