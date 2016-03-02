#include <obs-module.h>
#include "audiocapture.h"

#define SPECTRUM_NAME obs_module_text("Spectrum.Name")

#define SETTINGS_AUDIO_SOURCE_NAME "Audio.Source"

#define PROPERTIES_AUDIO_SOURCES_LIST_LABEL                                    \
	obs_module_text("Audio.Sources.List.Label")

struct source_context {
	audiocapture_t *audioc;
	uint32_t cx, cy;

	uint32_t channels;
	uint32_t sample_rate;
	uint32_t frame_size;

};
typedef struct source_context source_context_t;

static const char *source_get_name(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return SPECTRUM_NAME;
}

static void source_update(void *data, obs_data_t *settings)
{
	source_context_t *ctx = (source_context_t *)data;

	const char *audio_source_name = obs_data_get_string(settings,
		SETTINGS_AUDIO_SOURCE_NAME);
	
	struct obs_audio_info oai;
	obs_get_audio_info(&oai);

	ctx->channels    = get_audio_channels(oai.speakers);
	ctx->sample_rate = oai.samples_per_sec;
	ctx->frame_size  = 1024;

	if (ctx->audioc)
		audiocapture_destroy(ctx->audioc);

	audiocapture_create(audio_source_name, ctx->channels, ctx->frame_size);
}

static void *source_create(obs_data_t *settings, obs_source_t *source)
{
	source_context_t *ctx;

	ctx = bzalloc(sizeof(source_context_t));
	
	ctx->cx = 1280;
	ctx->cy = 720;

	source_update(ctx, settings);

	return ctx;
}

static void source_destroy(void *data)
{
	source_context_t *ctx = data;

	audiocapture_destroy(ctx->audioc);
	bfree(ctx);
}

static uint32_t source_getwidth(void *data)
{
	source_context_t *ctx = data;
	return ctx->cx;
}

static uint32_t source_getheight(void *data)
{
	source_context_t *ctx = data;
	return ctx->cy;
}


static void source_tick(void *data, float seconds)
{

}

static void source_render(void *data, gs_effect_t *effect)
{

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

static obs_properties_t *source_properties(void *data)
{
	struct audiovis_source *context = data;
	obs_properties_t *props;
	obs_property_t *prop;

	props = obs_properties_create();

	prop = obs_properties_add_list(
		props,
		SETTINGS_AUDIO_SOURCE_NAME,
		PROPERTIES_AUDIO_SOURCES_LIST_LABEL,
		OBS_COMBO_TYPE_LIST,
		OBS_COMBO_FORMAT_STRING);

	struct enum_sources_proc_params params = { prop, context };

	//params.prop = prop;
	//params.context = context;

	for (uint32_t i = 1; i <= 10; i++) {
		obs_source_t *source = obs_get_output_source(i);
		if (source) {
			enum_sources_proc(&params, source);
			obs_source_release(source);
		}
	}

	obs_enum_sources(enum_sources_proc, &params);

	//visual_get_properties(context->vis_info, props);

	return props;
}

static void source_defaults(obs_data_t *settings)
{
	obs_data_set_default_string(settings, SETTINGS_AUDIO_SOURCE_NAME, "");
}

struct obs_source_info spectrum_source = {
	.id = "av_spectrum_source",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_VIDEO,
	.get_name = source_get_name,
	.create = source_create,
	.destroy = source_destroy,
	.update = source_update,
	//.get_defaults = audiovis_source_defaults,
	//.show = audiovis_source_show,
	//.hide = audiovis_source_hide,
	//.activate = audiovis_source_activate,
	//.deactivate = audiovis_source_deactivate,
	.get_width = source_getwidth,
	.get_height = source_getheight,
	.video_tick = source_tick,
	.video_render = source_render,
	.get_properties = source_properties
};