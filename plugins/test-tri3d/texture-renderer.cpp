#include "texture-renderer.hpp"

void TextureRenderer::SetupEffect(const char *file_name)
{
	effect.Load(file_name);
	ep_texture.Load(effect, "image");
}

void TextureRenderer::Render(gs_texture_t *texture) {
	ep_texture.Set(texture);
	
	while (gs_effect_loop(effect.GetEffect(), "Draw"))
		gs_draw_sprite(texture, 0, 1280, 720);
}