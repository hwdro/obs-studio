#pragma once
#include <graphics/graphics.h>

class RenderTarget {
private:
	gs_texture_render *render_target;
	uint32_t width;
	uint32_t height;
public:
	RenderTarget();
	~RenderTarget();
	void Create(uint32_t w, uint32_t h, enum gs_color_format format, enum gs_zstencil_format zsformat);
	bool Enable();
	void Disable();
	void Reset();
	gs_texture_t * GetTexture();
	void Destroy();
};