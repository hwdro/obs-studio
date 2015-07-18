#include "vis-scope.hpp"
#include <vector>

using namespace std;

VisScope::VisScope(uint32_t cx_, uint32_t cy_, uint32_t sampleRate_,
	uint32_t channels_, size_t size_)
	: Visual(cx_, cy_)
{
	frameBuffer = static_cast<uint8_t *>(bzalloc(cx * cy * 4));

	obs_enter_graphics();
	texture = gs_texture_create(cx, cy, GS_RGBA, 1, nullptr, GS_DYNAMIC);
	obs_leave_graphics();

	audioData = new AudioData(sampleRate_, channels_, size_);
}

VisScope::~VisScope()
{
	obs_enter_graphics();
	gs_texture_destroy(texture);
	obs_leave_graphics();

	bfree(frameBuffer);

	delete audioData;
}
void VisScope::Render(gs_effect_t *effect)
{
	gs_reset_blend_state();

	gs_effect_set_texture(gs_effect_get_param_by_name(effect, "image"),
		texture);
	gs_draw_sprite(texture, 0, cx, cy);
}
void VisScope::Tick(float seconds)
{
	uint32_t x, y;
	float *data = audioData->GetBuffers()[0];
	memset(frameBuffer, 0, cx * cy * 4);
	for (x = 0; x < cx; x++) {
		y = static_cast<uint32_t>(cy / 2.0f + cy * data[x] / 2.0f);
		uint32_t *fb = reinterpret_cast<uint32_t *>(frameBuffer);
		fb[x + y*cx] = 0xFFFFFFFF;
	}
	obs_enter_graphics();
	gs_texture_set_image(texture, frameBuffer, cx * 4, false);
	obs_leave_graphics();
}

void VisScope::Update(AudioData *data)
{
	for (uint32_t i = 0; i < data->GetChannels(); i++) {
		float *src  = data->GetBuffers()[i];
		float *dest = audioData->GetBuffers()[i];
		memcpy(dest, src, data->GetSize() * sizeof(float));
	}
	
}