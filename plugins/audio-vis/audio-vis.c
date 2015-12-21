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
#include <util/threading.h>
#include <util/dstr.h>

#include "avis-monitor.h"
#include "vis-spectrum.h"

#define blog(log_level, format, ...) \
	blog(log_level, "[AVIS Source: '%s'] " format, \
			obs_source_get_name(context->source), ##__VA_ARGS__)

#define debug(format, ...) \
	blog(LOG_DEBUG, format, ##__VA_ARGS__)
#define info(format, ...) \
	blog(LOG_INFO, format, ##__VA_ARGS__)
#define warn(format, ...) \
	blog(LOG_WARNING, format, ##__VA_ARGS__)

#define AVIS_PLUGIN_NAME  obs_module_text("avis.plugin.name")

#define SETTINGS_AUDIO_SOURCES  "avis.audio.sources"
#define TEXT_AUDIO_SOURCES_DESC obs_module_text("avis.audio.sources.desc")

#define ACQ_RETRY_TIMEOUT_S 1.0f

struct audiovis_source {
	obs_source_t    *source;
	float           acq_retry_timeout;

	avis_monitor_t *monitor;
	visual_info_t   *vis_info;
};

static const char *audiovis_source_get_name(void)
{
	return AVIS_PLUGIN_NAME;
}


static void audiovis_source_update(void *data, obs_data_t *settings)
{
	struct audiovis_source *context = data;
	uint32_t channels, sample_rate;
	struct obs_audio_info  oai;
	size_t   frame_size;
	visual_info_t *vi;

	if (!context) return;

	const char *audio_source_name = obs_data_get_string(settings,
		SETTINGS_AUDIO_SOURCES);

	obs_get_audio_info(&oai);

	channels    = get_audio_channels(oai.speakers);
	sample_rate = oai.samples_per_sec;
	
	vi = context->vis_info;

	visual_destroy(vi);
	visual_create(vi, settings, sample_rate, channels);

	frame_size = visual_frame_size(vi);

	if (context->monitor)
		avis_monitor_destroy(context->monitor);

	context->monitor = avis_monitor_create(audio_source_name,
		sample_rate, channels, frame_size);
}

static visual_t * register_visuals(void)
{
	return get_spectrum_visualisation();
}

static void *audiovis_source_create(obs_data_t *settings, obs_source_t *source)
{
	struct audiovis_source *context;

	context = bzalloc(sizeof(struct audiovis_source));
	
	context->source = source;
	
	context->acq_retry_timeout = ACQ_RETRY_TIMEOUT_S;

	context->vis_info = vi_create(register_visuals());
	
	audiovis_source_update(context, settings);

	return context;
}

static void audiovis_source_destroy(void *data)
{
	struct audiovis_source *context = data;
	if (!context) return;

	avis_monitor_destroy(context->monitor);
	visual_destroy(context->vis_info);
	vi_destroy(context->vis_info);
	bfree(context);
}


static uint32_t audiovis_source_getwidth(void *data)
{
	struct audiovis_source *context = data;
	if (!context) return 0; 
	
	return visual_get_width(context->vis_info);
}

static uint32_t audiovis_source_getheight(void *data)
{
	struct audiovis_source *context = data;
	if (!context) return 0;
	
	return visual_get_height(context->vis_info);
}

static void audiovis_source_show(void *data)
{
	struct audiovis_source *context = data;
	if (!context) return;
}

static void audiovis_source_hide(void *data)
{
	struct audiovis_source *context = data;
	if (!context) return;
}

static void audiovis_source_activate(void *data)
{
	struct audiovis_source *context = data;
	if (!context) return;
}

static void audiovis_source_deactivate(void *data)
{
	struct audiovis_source *context = data;
	if (!context) return;
}

static void audiovis_source_render(void *data, gs_effect_t *effect)
{
	struct audiovis_source *context = data;

	if (!context) return;
	if (!context->monitor->source) return;
	
	visual_render(context->vis_info, effect);
}

static void audiovis_source_tick(void *data, float seconds)
{
	struct audiovis_source *context = data;
	
	if (!context) return;
	if (!context->monitor) return;

	if (!context->monitor->source) {
		context->acq_retry_timeout -= seconds;
		if (context->acq_retry_timeout < 0.0f) {
			avis_monitor_acquire_obs_source(context->monitor);
			context->acq_retry_timeout = ACQ_RETRY_TIMEOUT_S;
		}
	}

	if (pthread_mutex_trylock(&context->monitor->data_mutex))
		return;
	
	visual_process_audio(context->vis_info, context->monitor->data);
	pthread_mutex_unlock(&context->monitor->data_mutex);
	visual_tick(context->vis_info, seconds);
}

struct enum_sources_proc_params
{
	obs_property_t *prop;
	struct audiovis_source *context;
};


bool enum_sources_proc(void *param, obs_source_t *source)
{
	struct enum_sources_proc_params *params = param;
	struct audiovis_source *context = params->context;
	obs_property_t *prop = params->prop;
	const char *name = obs_source_get_name(source);
	uint32_t flags = obs_source_get_output_flags(source);

	if (flags & OBS_SOURCE_AUDIO)
		obs_property_list_add_string(prop, name, name);

	return true;
}

static obs_properties_t *audiovis_source_properties(void *data)
{
	struct audiovis_source *context = data;
	struct enum_sources_proc_params params;
	obs_properties_t *props;
	obs_property_t *prop;

	props = obs_properties_create();

	prop = obs_properties_add_list(
		props,
		SETTINGS_AUDIO_SOURCES,
		TEXT_AUDIO_SOURCES_DESC,
		OBS_COMBO_TYPE_LIST,
		OBS_COMBO_FORMAT_STRING);


	params.prop = prop;
	params.context = context;

	for (uint32_t i = 1; i <= 10; i++) {
		obs_source_t *source = obs_get_output_source(i);
		if (source) {
			enum_sources_proc(&params, source);
			obs_source_release(source);
		}
	}

	obs_enum_sources(enum_sources_proc, &params);

	visual_get_properties(context->vis_info, props);

	return props;
}

static void audiovis_source_defaults(obs_data_t *settings)
{
	obs_data_set_default_string(settings, SETTINGS_AUDIO_SOURCES, "");
	spectrum_get_defaults(settings);
}

static struct obs_source_info audiovis_source_info = {
	.id             = "audiovis_source",
	.type           = OBS_SOURCE_TYPE_INPUT,
	.output_flags   = OBS_SOURCE_VIDEO,
	.get_name       = audiovis_source_get_name,
	.create         = audiovis_source_create,
	.destroy        = audiovis_source_destroy,
	.update         = audiovis_source_update,
	.get_defaults   = audiovis_source_defaults,
	.show           = audiovis_source_show,
	.hide           = audiovis_source_hide,
	.activate       = audiovis_source_activate,
	.deactivate     = audiovis_source_deactivate,
	.get_width      = audiovis_source_getwidth,
	.get_height     = audiovis_source_getheight,
	.video_tick     = audiovis_source_tick,
	.video_render   = audiovis_source_render,
	.get_properties = audiovis_source_properties,
};

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("audiovis-source", "en-US")

bool obs_module_load(void)
{
	obs_register_source(&audiovis_source_info);
	return true;
}
