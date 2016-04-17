#pragma once
#include <obs-module.h>
#include "effect.hpp"

class TextureRenderer {
private:
	Effect effect;
	EffectParam ep_texture;
public:
	void SetupEffect(const char *file_name);
	void Render(gs_texture_t *texture);
};