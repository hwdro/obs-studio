#include <obs-module.h>
#include "audiocapture.h"
#include "vg.h"

#define SPECTRUM_NAME obs_module_text("Spectrum.Name")

#define SETTINGS_AUDIO_SOURCE_NAME "Audio.Source"

#define PROPERTIES_AUDIO_SOURCES_LIST_LABEL                                    \
	obs_module_text("Audio.Sources.List.Label")

struct source_context {
	audiocapture_t *audioc;
	audiobuf_t audio_data;
	vg_context_t vg;
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
	ctx->frame_size  = ctx->cx;

	audiobuf_free(&ctx->audio_data);
	audiobuf_init(&ctx->audio_data, ctx->channels, ctx->frame_size);

	if (ctx->audioc)
		audiocapture_destroy(ctx->audioc);

	ctx->audioc = audiocapture_create(audio_source_name, ctx->channels,
					  ctx->frame_size);
	
	vg_context_init(&ctx->vg);
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
	vg_context_free(&ctx->vg);
	audiobuf_free(&ctx->audio_data);
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

static void draw_scopes(source_context_t *ctx)
{
	vg_context_t *pc = &ctx->vg;
	uint32_t cx, cy;

	cx = ctx->cx;
	cy = ctx->cy;
	
	vg_begin_paint(pc);
	
	float *audio[2];
	
	audio[0] = audiobuf_get_buffer(&ctx->audio_data, 0);
	audio[1] = audiobuf_get_buffer(&ctx->audio_data, 1);

	size_t frame_size = audiobuf_get_frame_size(&ctx->audio_data);

	struct vec4 color;
	vec4_from_rgba(&color, 0xFFFFFFFF);

	for (int i = 0; i < frame_size; i++) {
		vg_draw_rectangle_f(pc, (float)i, (float)cy / 4,
			(float)i + 1.0f,
			(float)cy / 4 *(2 - audio[0][i]), &color);

		vg_draw_rectangle_f(pc, (float)i, (float)cy * 3 / 4,
			(float)i + 1.0f,
			(float)cy / 4 * (4 - audio[1][i]), &color);
	}
	
	vg_end_paint(pc);
}

static void source_tick(void *data, float seconds)
{
	source_context_t *ctx = (source_context_t *)data;
	audiocapture_copy_buffers(ctx->audioc, &ctx->audio_data);
	draw_scopes(ctx);
}

static void source_render(void *data, gs_effect_t *effect)
{

}

struct enum_sources_proc_params
{
	obs_property_t *prop;
	source_context_t *context;
};


bool enum_sources_proc(void *param, obs_source_t *source)
{
	obs_property_t *prop = param;

	const char *name = obs_source_get_name(source);
	uint32_t flags   = obs_source_get_output_flags(source);

	if (flags & OBS_SOURCE_AUDIO) {
		obs_property_list_add_string(prop, name, name);
		blog(LOG_WARNING, "Asrc: %s ", name);
	}
	return true;
}

static obs_properties_t *source_properties(void *data)
{
	source_context_t *context = data;
	obs_properties_t *props;
	obs_property_t *prop;

	props = obs_properties_create();

	prop = obs_properties_add_list(props, SETTINGS_AUDIO_SOURCE_NAME,
				       PROPERTIES_AUDIO_SOURCES_LIST_LABEL,
				       OBS_COMBO_TYPE_LIST,
				       OBS_COMBO_FORMAT_STRING);

	obs_enum_sources(enum_sources_proc, prop);

	return props;
}

static void source_defaults(obs_data_t *settings)
{
	obs_data_set_default_string(settings, SETTINGS_AUDIO_SOURCE_NAME, "");
}

struct obs_source_info spectrum_source = {
	.id = "av_spectrum_source",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW,
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