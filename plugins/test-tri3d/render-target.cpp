#include <obs-module.h>
#include "render-target.hpp"

RenderTarget::RenderTarget()
	: render_target(nullptr),
	  width(0),
	  height(0)
{
}

RenderTarget::~RenderTarget()
{
	if (render_target)
		Destroy();
}

void RenderTarget::Create(uint32_t w, uint32_t h, enum gs_color_format format, enum gs_zstencil_format zsformat)
{
	width = w;
	height = h;

	obs_enter_graphics();
	render_target = gs_texrender_create(format, zsformat);
	obs_leave_graphics();
}

bool RenderTarget::Enable()
{
	if (render_target) {
		return gs_texrender_begin(render_target, width, height);
	}
	return false;
}

void RenderTarget::Disable()
{
	if (render_target) {
		gs_texrender_end(render_target);
	}
}

void RenderTarget::Reset()
{
	if (render_target)
		gs_texrender_reset(render_target);
}

gs_texture_t * RenderTarget::GetTexture()
{
	if (render_target)
		return gs_texrender_get_texture(render_target);
	return nullptr;
}

void RenderTarget::Destroy()
{
	if (render_target) {
		obs_enter_graphics();
		gs_texrender_destroy(render_target);
		obs_leave_graphics();
	}
}
