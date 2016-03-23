#include <obs-module.h>

#define SOURCE_NAME "Test Tri3D"

struct source_context {
	gs_effect_t *effect;
	uint32_t cx;
	uint32_t cy;
	gs_vertbuffer_t  *vb;
	gs_indexbuffer_t *ib;
};

typedef struct source_context source_context_t;

static const char *source_get_name(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return SOURCE_NAME;
}

static void cleanup_buffers(source_context_t *ctx)
{
	if (!ctx)
		return;

	obs_enter_graphics();

	if (ctx->vb) {
		gs_vertexbuffer_destroy(ctx->vb);
		ctx->vb = NULL;
	}

	if (ctx->ib) {
		gs_indexbuffer_destroy(ctx->ib);
		ctx->ib = NULL;
	}

	obs_leave_graphics();
}

static void build_buffers(source_context_t *ctx)
{
	cleanup_buffers(ctx);

	struct vec3 *vertices = bzalloc(sizeof(struct vec3) * 3);
	uint32_t *indexes = bzalloc(sizeof(uint32_t) * 3);
	uint32_t *colors = bzalloc(sizeof(uint32_t) * 3);

	struct vec3 v;

	vec3_set(&v, 0.0f, 0.0f, 10.0f); // ---> vertex 1
	memcpy(vertices, &v, sizeof(struct vec3));

	vec3_set(&v, (float)ctx->cx, (float)ctx->cy, 50.0f); // ---> vertex 2
	memcpy(vertices + 1, &v, sizeof(struct vec3));

	vec3_set(&v, 0.0f, (float)ctx->cy, 25.0f); // ---> vertex 3
	memcpy(vertices + 2, &v, sizeof(struct vec3));

	indexes[0] = 0;
	indexes[1] = 1;
	indexes[2] = 2;

	colors[0] = 0xFF0000FF;
	colors[1] = 0xFF00FF00;
	colors[2] = 0xFFFF0000;

	struct gs_vb_data *vbd = gs_vbdata_create();

	vbd->num_tex = 0;
	vbd->tvarray = NULL;
	vbd->num = 3;
	vbd->points = vertices;
	vbd->colors = colors;
	vbd->normals = NULL;
	vbd->tangents = NULL;

	obs_enter_graphics();
	
	ctx->vb = gs_vertexbuffer_create(vbd, GS_DYNAMIC);
	ctx->ib =
	    gs_indexbuffer_create(GS_UNSIGNED_LONG, indexes, 3, GS_DYNAMIC);

	obs_leave_graphics();

}


static void source_update(void *data, obs_data_t *settings)
{
	UNUSED_PARAMETER(data);
	UNUSED_PARAMETER(settings);
}

static void *source_create(obs_data_t *settings, obs_source_t *source)
{
	source_context_t *ctx;

	ctx = bzalloc(sizeof(source_context_t));

	ctx->cx = 1280;
	ctx->cy = 720;

	char *file = obs_module_file("tri3d.effect");

	obs_enter_graphics();
	gs_effect_t *effect = gs_effect_create_from_file(file, NULL);
	obs_leave_graphics();

	bfree(file);

	ctx->effect = effect;

	//source_update(ctx, settings);

	build_buffers(ctx);

	return ctx;
}

static void source_destroy(void *data)
{
	source_context_t *ctx = data;
	cleanup_buffers(ctx);
	bfree(ctx);
}

static uint32_t source_getwidth(void *data)
{
	source_context_t *ctx = data;
	return ctx->cx;
}

static uint32_t source_getheight(void *data)
{
	source_context_t *ctx = data;
	return ctx->cy;
}

static void source_tick(void *data, float seconds)
{
	UNUSED_PARAMETER(data);
	UNUSED_PARAMETER(seconds);
}

static void source_render(void *data, gs_effect_t *effect)
{
	source_context_t *ctx = (source_context_t *)data;

	gs_vertbuffer_t *vb = ctx->vb;
	gs_indexbuffer_t *ib = ctx->ib;

	if (!vb || !ib) return;

	gs_vertexbuffer_flush(vb);
	gs_load_vertexbuffer(vb);

	gs_indexbuffer_flush(ib);
	gs_load_indexbuffer(ib);

	gs_projection_push();

	gs_frustum(0.0f, (float)ctx->cx, 0.0f, (float)ctx->cy, 10.0f, 50.0f);
	
	while (gs_effect_loop(ctx->effect, "Draw"))
		gs_draw(GS_TRIS, 0, 0);

	gs_projection_pop();

	gs_load_indexbuffer(NULL);
	gs_load_vertexbuffer(NULL);
}

struct obs_source_info tri3D_source = {
	.id = "tri3D_source",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW,
	.get_name = source_get_name,
	.create = source_create,
	.destroy = source_destroy,
	//.update = source_update,
	.get_width = source_getwidth,
	.get_height = source_getheight,
	.video_tick = source_tick,
	.video_render = source_render,
};


OBS_DECLARE_MODULE()

OBS_MODULE_USE_DEFAULT_LOCALE("tri3d", "en-US")

extern struct obs_source_info spectrum_source;


bool obs_module_load(void)
{
	obs_register_source(&tri3D_source);
	return true;
}