#include <obs-module.h>
#include <vector>

#include "object.hpp"
#include "camera.hpp"
#include "render-target.hpp"
#include "texture-renderer.hpp"

class Source {
private:
	uint32_t width, height;
	float global_time;
	Cube cube;
	Object object;
	Camera camera;
	RenderTarget render_target;
	TextureRenderer tex_renderer;

public:
	Source() = delete;
	Source(Source&) = delete;
	Source(obs_data_t *settings, obs_source_t *source);
	void UpdateSettings(obs_data_t *settings);
	obs_properties_t * Properties(void);
	uint32_t Width();
	uint32_t Height();
	void Tick(float seconds);
	void Render(gs_effect_t *eff);
	~Source();
};

Source::Source(obs_data_t *settings, obs_source_t *source)
    : width(1280), height(720), global_time(0.0f)
{
	//UNUSED_PARAMETER(settings);
	float aspect = (float)width / (float)height;

	cube.Set();
	object.AttachModel(&cube);
	object.transform.SetScale(2.0f, 2.0f, 2.0f);

	camera.Perspective(aspect, 70.0f, 0.1f, 1000.0f);
	object.AttachCamera(&camera);

	object.LoadEffect("rectangle.effect");

	render_target.Create(width, height, GS_RGBA, GS_Z32F);

	tex_renderer.SetupEffect("texture.effect");

	obs_source_update(source, settings);
}

Source::~Source()
{

}

void Source::UpdateSettings(obs_data_t *settings)
{
	UNUSED_PARAMETER(settings);
}
obs_properties_t * Source::Properties(void)
{
	return nullptr;
}

uint32_t Source::Width()
{
	return width;
}

uint32_t Source::Height()
{
	return height;
}

void Source::Tick(float seconds)
{
	global_time += seconds;
	vec3 campos = { 5*cosf(global_time), 3*sinf(global_time), 5*sinf(global_time) };
	vec3 target = { 0.0f, 0.0f, 0.0f };
	camera.LookAt(&campos, &target);
	
}

void Source::Render(gs_effect_t *eff)
{
	UNUSED_PARAMETER(eff);

	render_target.Enable();

	object.BeginRendering();
	object.Render();
	object.EndRendering();
	render_target.Disable();

	tex_renderer.Render(render_target.GetTexture());
	render_target.Reset();
}


static const char * GetSourceName(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return obs_module_text("Test3D");
}

static void * CreateSource(obs_data_t *settings, obs_source_t *source)
{
	Source *src = new Source(settings, source);
	return src;
}

static void DestroySource(void *data)
{
	Source *src = static_cast<Source *>(data);
	delete src;
}

static void UpdateSource(void *data, obs_data_t *settings)
{
	Source *src = static_cast<Source *>(data);
	src->UpdateSettings(settings);
}

static void GetDefaults(obs_data_t *settings)
{
	UNUSED_PARAMETER(settings);
}

static obs_properties_t * GetSourceProperties(void *data)
{
	Source *src = static_cast<Source *>(data);
	return src->Properties();
}

static uint32_t GetWidth(void *data)
{
	Source *src = static_cast<Source *>(data);
	return src->Width();
}

static uint32_t GetHeight(void *data)
{
	Source *src = static_cast<Source *>(data);
	return src->Height();
}

static void SourceTick(void *data, float seconds)
{
	Source *src = static_cast<Source *>(data);
	src->Tick(seconds);
}

static void SourceRender(void *data, gs_effect_t *effect)
{
	Source *src = static_cast<Source *>(data);
	src->Render(effect);
}


void RegisterSource()
{
	obs_source_info info = {0};
	info.id = "test3drendering";
	info.type = OBS_SOURCE_TYPE_INPUT;
	info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;
	info.get_name = GetSourceName;
	info.create = CreateSource;
	info.destroy = DestroySource;
	info.update = UpdateSource;
	info.get_defaults = GetDefaults;
	info.get_properties = GetSourceProperties;
	info.get_height = GetHeight;
	info.get_width = GetWidth;
	info.video_tick = SourceTick;
	info.video_render = SourceRender;
	obs_register_source(&info);
}
