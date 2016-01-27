#include <obs-module.h>

struct fade_tofrom_info {
	obs_source_t *source;

	gs_effect_t *effect;
	gs_eparam_t *a_param;
	gs_eparam_t *b_param;
	gs_eparam_t *progress;
	gs_eparam_t *color_param;
	gs_eparam_t *switch_point_param;
	struct vec4 color;
	float switch_point;
};

/*
	TODO(HomeWorld): Add properties for color and switch_point when UI
	will support it. For now this transition fades to/from black, the switch
	happens at 1/2 of duration (switch point is specified in the [0 - 1] 
	interval)
*/

static const char *fade_tofrom_get_name(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return obs_module_text("FadeToFromTransition");
}

void *fade_tofrom_create(obs_data_t *settings, obs_source_t *source)
{
	struct fade_tofrom_info *fade;
	char *file = obs_module_file("fade_tofrom_transition.effect");
	gs_effect_t *effect;

	obs_enter_graphics();
	effect = gs_effect_create_from_file(file, NULL);
	obs_leave_graphics();
	bfree(file);

	if (!effect) {
		blog(LOG_ERROR, "Could not find fade_tofrom_transition.effect");
		return NULL;
	}

	fade = bmalloc(sizeof(*fade));
	fade->source = source;
	fade->effect = effect;
	fade->a_param = gs_effect_get_param_by_name(effect, "tex_a");
	fade->b_param = gs_effect_get_param_by_name(effect, "tex_b");
	fade->progress = gs_effect_get_param_by_name(effect, "progress");
	fade->color_param = 
		gs_effect_get_param_by_name(effect,"color");
	fade->switch_point_param = 
		gs_effect_get_param_by_name(effect, "switch_point");
	
	vec4_from_rgba(&fade->color, 0xFF000000);
	fade->switch_point = 0.5f;

	UNUSED_PARAMETER(settings);
	return fade;
}

void fade_tofrom_destroy(void *data)
{
	struct fade_tofrom_info *fade = data;
	bfree(fade);
}

static void fade_tofrom_callback(void *data, gs_texture_t *a, gs_texture_t *b,
	float t, uint32_t cx, uint32_t cy)
{
	struct fade_tofrom_info *fade = data;

	gs_effect_set_texture(fade->a_param, a);
	gs_effect_set_texture(fade->b_param, b);
	gs_effect_set_float(fade->progress, t);
	gs_effect_set_vec4(fade->color_param, &fade->color);
	gs_effect_set_float(fade->switch_point_param, fade->switch_point);

	while (gs_effect_loop(fade->effect, "FadeToFrom"))
		gs_draw_sprite(NULL, 0, cx, cy);
}

void fade_tofrom_video_render(void *data, gs_effect_t *effect)
{
	struct fade_tofrom_info *fade = data;
	obs_transition_video_render(fade->source, fade_tofrom_callback);
	UNUSED_PARAMETER(effect);
}

static float lerp(float a, float b, float x)
{
	return (1.0f - x) * a + x * b;
}

static float clamp(float x, float min, float max)
{
	if (x < min)
		return min;
	else if (x > max)
		return max;
	return x;
}

static float smoothstep(float min, float max, float x)
{
	x = clamp((x - min) / (max - min), 0.0f, 1.0f);
	return x*x*(3 - 2 * x);
}

static float mix_a(void *data, float t)
{
	struct fade_tofrom_info *fade = data;
	float sp = fade->switch_point;

	return lerp(1.0f - t , 0.0f, smoothstep(0.0f, sp, t));
}

static float mix_b(void *data, float t)
{
	struct fade_tofrom_info *fade = data;
	float sp = fade->switch_point;

	return lerp(0.0f, t, smoothstep(sp, 1.0f, t));
}

bool fade_tofrom_audio_render(void *data, uint64_t *ts_out,
		struct obs_source_audio_mix *audio, uint32_t mixers,
		size_t channels, size_t sample_rate)
{
	struct fade_tofrom_info *fade = data;
	return obs_transition_audio_render(fade->source, ts_out,
		audio, mixers, channels, sample_rate, mix_a, mix_b);
}

struct obs_source_info fade_tofrom_transition = {
	.id = "fade_tofrom_transition",
	.type = OBS_SOURCE_TYPE_TRANSITION,
	.get_name = fade_tofrom_get_name,
	.create = fade_tofrom_create,
	.destroy = fade_tofrom_destroy,
	.video_render = fade_tofrom_video_render,
	.audio_render = fade_tofrom_audio_render
};
