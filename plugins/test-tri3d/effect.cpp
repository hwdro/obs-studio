#include "effect.hpp"


gs_effect * Effect::GetEffect()
{
	return effect;
}

void Effect::Load(const char *file_name)
{
	char *file = obs_module_file(file_name);
	
	obs_enter_graphics();
	effect = gs_effect_create_from_file(file, nullptr);
	obs_leave_graphics();

	bfree(file);

	if (!effect)
		blog(LOG_ERROR, "Unable to load %s", file_name);
}

Effect::Effect()
	:effect(nullptr)
{

}

Effect::~Effect()
{
}

EffectParam::EffectParam()
{
	param = nullptr;
}

void EffectParam::Load(Effect &effect, const char *name)
{
	gs_effect_t *eff = effect.GetEffect();
	if (eff) {
		param = gs_effect_get_param_by_name(eff, name);
	}
}

template <>
void EffectParam::Set(gs_texture_t *val)
{
	if (param) {
		gs_effect_set_texture(param, val);
	}
}

template <>
void EffectParam::Set(int val)
{
	if (param) {
		gs_effect_set_int(param, val);
	}
}

template <>
void EffectParam::Set(float val)
{
	if (param)
		gs_effect_set_float(param, val);
}

template <>
void EffectParam::Set(vec3 *val)
{
	if (param)
		gs_effect_set_vec3(param, val);
}

template <>
void EffectParam::Set(vec2 *val)
{
	if (param)
		gs_effect_set_vec2(param, val);
}

template <>
void EffectParam::Set(bool val)
{
	if (param)
		gs_effect_set_bool(param, val);
}

template <>
void EffectParam::Set(matrix4 *val)
{
	if (param)
		gs_effect_set_matrix4(param, val);
}

template <>
void EffectParam::Set(vec4 *val)
{
	if (param)
		gs_effect_set_vec4(param, val);
}

