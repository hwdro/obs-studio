#pragma once

#include <obs-module.h>


class Effect {
private:
	gs_effect_t *effect;
public:
	gs_effect_t * GetEffect();
	void Load(const char *file_name);
	Effect();
	~Effect();
};

class EffectParam {
	gs_eparam_t *param;
public:
	EffectParam();
	void Load(Effect &effect, const char* name);
	template <typename T>
	void Set(T value);
};