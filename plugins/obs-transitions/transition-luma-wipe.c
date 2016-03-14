#include <obs-module.h>
#include <graphics/image-file.h>

#define SETTINGS_LUMA_IMAGE "luma_image"
#define SETTINGS_LUMA_INVERT "luma_invert"

struct luma_wipe_info {
	obs_source_t *source;

	gs_effect_t *effect;
	gs_eparam_t *ep_a_tex;
	gs_eparam_t *ep_b_tex;
	gs_eparam_t *ep_l_tex;
	gs_eparam_t *ep_progress;
	gs_eparam_t *ep_invert;
	gs_eparam_t *ep_softness;

	gs_image_file_t luma_image;
	bool  invert_luma;
	float softness;
};

static const char *luma_wipe_get_name(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return obs_module_text("LumaWipeTransition");
}

static void luma_wipe_update(void *data, obs_data_t *settings)
{
	struct luma_wipe_info *lwipe = data;

	//char *path= obs_data_get_string(settings, SETTINGS_LUMA_IMAGE);
	//lwipe->invert_luma = obs_data_get_bool(settings, SETTINGS_LUMA_INVERT);

	char *path = obs_module_file("luma_wipes/parallel-zigzag-h.png");
	lwipe->invert_luma = false;
	lwipe->softness = 0.1f;

	// shader params : luma image l_tex , invert_luma invert , softness

	obs_enter_graphics();
	gs_image_file_free(&lwipe->luma_image);
	obs_leave_graphics();
	
	gs_image_file_init(&lwipe->luma_image, path);

	obs_enter_graphics();
	gs_image_file_init_texture(&lwipe->luma_image);
	obs_leave_graphics();
	
	UNUSED_PARAMETER(settings);
}

static void *luma_wipe_create(obs_data_t *settings, obs_source_t *source)
{
	struct luma_wipe_info *lwipe;
	gs_effect_t *effect;
	char *file = obs_module_file("luma_wipe_transition.effect");

	obs_enter_graphics();
	effect = gs_effect_create_from_file(file, NULL);
	obs_leave_graphics();

	if (!effect) {
		blog(LOG_ERROR,
		     "Could not create open luma_wipe_transition.effect");
		return NULL;
	}

	bfree(file);

	lwipe = bzalloc(sizeof(*lwipe));

	lwipe->effect      = effect;
	lwipe->ep_a_tex    = gs_effect_get_param_by_name(effect, "a_tex");
	lwipe->ep_b_tex    = gs_effect_get_param_by_name(effect, "b_tex");
	lwipe->ep_l_tex    = gs_effect_get_param_by_name(effect, "l_tex");
	lwipe->ep_progress = gs_effect_get_param_by_name(effect, "progress");
	lwipe->ep_invert   = gs_effect_get_param_by_name(effect, "invert");
	lwipe->ep_softness = gs_effect_get_param_by_name(effect, "softness");
	lwipe->source      = source;
	luma_wipe_update(lwipe, settings);
	
	return lwipe;
}

static void luma_wipe_destroy(void *data)
{
	struct luma_wipe_info *lwipe = data;
	
	obs_enter_graphics();
	gs_image_file_free(&lwipe->luma_image);
	obs_leave_graphics();
	bfree(lwipe);
}

static void luma_wipe_callback(void *data, gs_texture_t *a, gs_texture_t *b,
			       float t, uint32_t cx, uint32_t cy)
{
	struct luma_wipe_info *lwipe = data;

	gs_effect_set_texture(lwipe->ep_a_tex, a);
	gs_effect_set_texture(lwipe->ep_b_tex, b);
	gs_effect_set_texture(lwipe->ep_l_tex, lwipe->luma_image.texture);
	gs_effect_set_float(lwipe->ep_progress, t);

	gs_effect_set_bool(lwipe->ep_invert, lwipe->invert_luma);
	gs_effect_set_float(lwipe->ep_softness, lwipe->softness);

	while (gs_effect_loop(lwipe->effect, "LumaWipe"))
		gs_draw_sprite(NULL, 0, cx, cy);
}

void luma_wipe_video_render(void *data, gs_effect_t *effect)
{
	struct luma_wipe_info *lwipe = data;
	obs_transition_video_render(lwipe->source, luma_wipe_callback);
	UNUSED_PARAMETER(effect);
}

static float mix_a(void *data, float t)
{
	UNUSED_PARAMETER(data);
	return 1.0f - t;
}

static float mix_b(void *data, float t)
{
	UNUSED_PARAMETER(data);
	return t;
}

bool luma_wipe_audio_render(void *data, uint64_t *ts_out,
			    struct obs_source_audio_mix *audio, uint32_t mixers,
			    size_t channels, size_t sample_rate)
{
	struct luma_wipe_info *lwipe = data;
	return obs_transition_audio_render(lwipe->source, ts_out, audio, mixers,
					   channels, sample_rate, mix_a, mix_b);
}

struct obs_source_info luma_wipe_transition = {
	.id   = "wipe_transition",
	.type = OBS_SOURCE_TYPE_TRANSITION,
	.get_name     = luma_wipe_get_name,
	.create       = luma_wipe_create,
	.destroy      = luma_wipe_destroy,
	.video_render = luma_wipe_video_render,
	.audio_render = luma_wipe_audio_render
};
