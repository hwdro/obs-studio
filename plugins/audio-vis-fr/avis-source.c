#include <obs-module.h>
#include "avis-capture.h"

#define SOURCE_NAME obs_module_text("avis.name")

#define SETTINGS_INPUT_SOURCE "avis.source"

#define PROPERTIES_SOURCES_LIST_LABEL                                    \
	obs_module_text("avis.properties.list.name")

struct avis_source {
	uint32_t width, height;
	uint32_t sample_rate;
	uint32_t channels;
	uint32_t frame_size;
	avis_capture_t *audio_input;
};
typedef struct avis_source avis_source_t;


static const char *source_get_name(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return SOURCE_NAME;
}

static void source_update(void *data, obs_data_t *settings)
{
	avis_source_t *ctx = data;
	struct obs_audio_info oai;
	
	obs_get_audio_info(&oai);

	ctx->channels = get_audio_channels(oai.speakers);
	ctx->sample_rate = oai.samples_per_sec;
	ctx->frame_size = 1024;

	const char *audio_source_name = obs_data_get_string(settings,
		SETTINGS_INPUT_SOURCE);
	ctx->audio_input = avis_capture_create(audio_source_name, ctx->channels,
		ctx->frame_size);
}


static void *source_create(obs_data_t *settings, obs_source_t *source)
{
	avis_source_t *ctx;

	ctx = bzalloc(sizeof(avis_source_t));

	ctx->width = 640;
	ctx->height = 360;

	obs_source_update(source, settings);

	return ctx;
}

static void source_destroy(void *data)
{
	avis_source_t *ctx = data;
	avis_capture_destroy(ctx->audio_input);
	bfree(ctx);
}

static uint32_t source_get_width(void *data)
{
	avis_source_t *ctx = data;
	return ctx->width;
}

static uint32_t source_get_height(void *data)
{
	avis_source_t *ctx = data;
	return ctx->height;
}

static void source_get_defaults(obs_data_t *settings)
{
	obs_data_set_default_string(settings, SETTINGS_INPUT_SOURCE, "");
}


struct enum_source_params {
	obs_property_t *list;
	avis_source_t *ctx;
};

static bool enum_sources_proc(void *data, obs_source_t *source)
{
	struct enum_source_params *params = data;
	avis_source_t *ctx = params->ctx;
	obs_property_t *list = params->list;

	const char *name = obs_source_get_name(source);
	uint32_t flags = obs_source_get_output_flags(source);

	if (flags & OBS_SOURCE_AUDIO)
		obs_property_list_add_string(list, name, name);

	return true;
}

static obs_properties_t *source_get_properties(void *data)
{
	avis_source_t *ctx = data;
	obs_properties_t *props = obs_properties_create();
	obs_property_t *sources_list = obs_properties_add_list(props,
		SETTINGS_INPUT_SOURCE, PROPERTIES_SOURCES_LIST_LABEL,
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);

	struct enum_source_params enum_params = { sources_list, ctx };
	obs_enum_sources(enum_sources_proc, &enum_params);

	return props;
}


static void source_video_tick(void *data, float seconds)
{
	UNUSED_PARAMETER(data);
	UNUSED_PARAMETER(seconds);
}

static void source_video_render(void *data, gs_effect_t *effect)
{
	UNUSED_PARAMETER(data);
	UNUSED_PARAMETER(effect);
}

struct obs_source_info avis_source_info = {
	.id = "avis_source",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_VIDEO,
	.get_name = source_get_name,
	.create = source_create,
	.update = source_update,
	.destroy = source_destroy,
	.get_width = source_get_width,
	.get_height = source_get_height,
	.get_defaults = source_get_defaults,
	.get_properties = source_get_properties,
	.video_tick = source_video_tick,
	.video_render = source_video_render,
};
