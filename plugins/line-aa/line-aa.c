#include <obs-module.h>
#include "audio-window.h"

#define LINEAA_PLUGIN_NAME  obs_module_text("lineaa.plugin.name")

struct line_info{
	struct vec3 start;
	struct vec3 end;
	struct vec4 color;
	float width;
};

struct lines_context{
	DARRAY(struct line_info) lines;
	DARRAY(struct vec3) vertices;
	DARRAY(uint16_t) indexes;
	DARRAY(struct vec3) normals;
	DARRAY(struct vec3) tangents;
	DARRAY(uint32_t) colors;
};

static void line_v(struct lines_context *lctx, struct vec3 *p1, struct vec3 *p2,
		   struct vec4 *color, float width)
{
	struct line_info li;

	vec3_copy(&li.start, p1);
	vec3_copy(&li.end, p2);
	vec4_copy(&li.color, color);
	
	li.width = width;
	
	da_push_back(lctx->lines, &li);
}
static void line_f(struct lines_context *lctx, float x1, float y1, float x2,
		   float y2, struct vec4 *color, float width)
{
	struct vec3 p1, p2;

	vec3_set(&p1, x1, y1, 0.0f);
	vec3_set(&p2, x2, y2, 0.0f);
	
	line_v(lctx, &p1, &p2, color, width);
}

static void lines_context_init(struct lines_context *lctx)
{
	da_init(lctx->lines);
	da_init(lctx->vertices);
	da_init(lctx->indexes);
	da_init(lctx->normals);
	da_init(lctx->tangents);
	da_init(lctx->colors);
}

static void lines_context_reset(struct lines_context *lctx)
{
	da_free(lctx->lines);
	// da_free(lctx->indexes);
	// da_free(lctx->normals);
	// da_free(lctx->tangents);
	// da_free(lctx->colors);
	// da_free(lctx->vertices);
	lines_context_init(lctx);
}

static void line_normal(struct vec3 *dst, struct vec3 *v1, struct vec3 *v2)
{
	struct vec3 diff;
	vec3_sub(&diff, v2, v1);
	vec3_set(dst, -diff.y, diff.x, 0.0f);
	vec3_norm(dst, dst);
}

static void line_tangent(struct vec3 *dst, struct vec3 *v1, struct vec3 *v2)
{
	struct vec3 diff;
	vec3_sub(&diff, v2, v1);
	vec3_norm(dst, &diff);
}

static void lines_context_update(struct lines_context *lctx)
{
	//profile_start("LINE-AA");
	//profile_end("LINE-AA");
	uint16_t vertice_index = 0;
	struct line_info *line;
	struct vec3 normal, neg_normal;
	struct vec3 tangent, neg_tangent;
	
	//
	//        ↑  t
	//        |
	// n ←--1 O 2--→ -n
	//        |
	//        |
	//        |
	// n ←--3 O 4--→ -n
	//        |
	//        ↓ -t
	//

	line = lctx->lines.array;
	
	size_t num = lctx->lines.num;
	
	da_reserve(lctx->vertices, num * 4);
	da_reserve(lctx->normals, num * 4);
	da_reserve(lctx->tangents, num * 4);
	da_reserve(lctx->colors, num * 4);
	da_reserve(lctx->indexes, num * 6);

	for (uint32_t c = 0; c < num; c++) {
		struct vec3 start, end;
		
		vec3_copy(&start, &line[c].start);
		vec3_copy(&end, &line[c].end);

		for (uint32_t i = 0; i < 2; i++)
			da_push_back(lctx->vertices, &start);
		
		for (uint32_t i = 0; i < 2; i++)
			da_push_back(lctx->vertices, &end);
		
		line_normal(&normal, &end, &start);
		
		float width = line[c].width / 2.0f;

		if (width < 0.5f)
			width = 0.5f;
		
		vec3_mulf(&normal, &normal, width);
		vec3_neg(&neg_normal, &normal);

		for (uint32_t i = 0; i < 2; i++) {
			da_push_back(lctx->normals, &normal);
			da_push_back(lctx->normals, &neg_normal);
		}
		
		line_tangent(&tangent, &end, &start);
		
		float line_length = vec3_dist(&start, &end);

		vec3_mulf(&tangent, &tangent, line_length);
		
		vec3_neg(&neg_tangent, &tangent);

		for (uint32_t i = 0; i < 2; i++)
			da_push_back(lctx->tangents, &tangent);

		for (uint32_t i = 0; i < 2; i++)
			da_push_back(lctx->tangents, &neg_tangent);



		for (uint32_t i = 0; i < 3; i++) {
			uint16_t vi = c * 4 + i;
			da_push_back(lctx->indexes, &vi);
		}

		for (uint32_t i = 3; i > 0; i--) {
			uint16_t vi = c * 4 + i;
			da_push_back(lctx->indexes, &vi);
		}
		
		for (uint32_t i = 0; i < 4; i++) {
			uint32_t color = vec4_to_rgba(&line[c].color);
			da_push_back(lctx->colors, &color);
		}
	}
}


static void lines_start(struct lines_context *lctx)
{
	lines_context_reset(lctx);
}

static void lines_end(struct lines_context *lctx)
{
	lines_context_update(lctx);
}

static void lines_dump(struct lines_context *lctx)
{
	for (uint32_t l = 0; l < lctx->lines.num; l++) {
		blog(LOG_WARNING, "%f , %f , %f , %f",
		     lctx->lines.array[l].start.x, lctx->lines.array[l].start.y,
		     lctx->lines.array[l].end.x,
		     lctx->lines.array[l].end.y);
	}
}

struct source_context {
	struct vec4 color;
	gs_vertbuffer_t *vb;
	gs_indexbuffer_t *ib;
	gs_texrender_t *texrender;
	gs_eparam_t *ep_tex;
	
	uint32_t cx, cy;

	gs_effect_t *effect;
	gs_effect_t *tex_effect;

	gs_eparam_t *ep_w;
	gs_eparam_t *ep_r;
	gs_eparam_t *ep_sx;
	gs_eparam_t *ep_sy;

	struct lines_context *lines_ctx;
	float time;
};

static void setup_vertex_buffer(struct source_context *ctx)
{
	struct gs_vb_data *vbd;

	obs_enter_graphics();

	if (ctx->vb) {
		gs_vertbuffer_t *tmp = ctx->vb;
		ctx->vb = NULL;
		gs_vertexbuffer_destroy(tmp);
	}

	size_t num = ctx->lines_ctx->vertices.num;
	
	if (!num)
		return;
	
	vbd = gs_vbdata_create();
	
	vbd->num_tex = 0;
	vbd->tvarray = NULL;
	
	vbd->points = ctx->lines_ctx->vertices.array;
	vbd->normals = ctx->lines_ctx->normals.array;
	vbd->tangents = ctx->lines_ctx->tangents.array;
	vbd->colors = ctx->lines_ctx->colors.array;
	vbd->num = num;
	
	ctx->vb = gs_vertexbuffer_create(vbd, GS_DYNAMIC);
	
	obs_leave_graphics();

}

static void cleanup_vertex_buffer(struct source_context *ctx)
{
	obs_enter_graphics();

	gs_vertbuffer_t *tmp = ctx->vb;
	gs_vertexbuffer_destroy(tmp);
	ctx->vb = NULL;
	
	obs_leave_graphics();
}

static void setup_index_buffer(struct source_context *ctx)
{
	obs_enter_graphics();

	if (ctx->ib) {
		gs_indexbuffer_t *tmp = ctx->ib;
		gs_indexbuffer_destroy(tmp);
		ctx->ib = NULL;
	}
	
	size_t num  = ctx->lines_ctx->indexes.num;
	size_t size = sizeof(uint16_t) * num;
	
	if (!num)
		return;

	ctx->ib = gs_indexbuffer_create(
	    GS_UNSIGNED_SHORT, ctx->lines_ctx->indexes.array, num, GS_DYNAMIC);
	
	obs_leave_graphics();
}

static void cleanup_index_buffer(struct source_context *ctx)
{
	obs_enter_graphics();
	
	gs_indexbuffer_t *tmp = ctx->ib;
	gs_indexbuffer_destroy(tmp);
	ctx->ib = NULL;
	
	obs_leave_graphics();
}

static void draw_some_lines(struct source_context *ctx)
{
	struct vec4 color;

	lines_start(ctx->lines_ctx);
		
	vec4_from_rgba(&color, 0xFFF0377F);
	line_f(ctx->lines_ctx, 0.0f, 0.0f, 1920.0f, 1080.0f, &color, 100.0f);
	
	vec4_from_rgba(&color, 0xFF0000FF);
	line_f(ctx->lines_ctx, 50.0f, 720.0f, 1280.0f, 190.0f, &color, 1.0f);

	vec4_from_rgba(&color, 0xFF000000);
	line_f(ctx->lines_ctx, 0.0f, 360.0f, 1280.0f, 360.0f, &color, 5.0f);
	line_f(ctx->lines_ctx, 360.0f, 360.0f, 761.0f, 761.0f, &color, 50.0f);
	vec4_from_rgba(&color, 0xFF00FFFF);
	int step = 1;
	for (int i = 0; i < 1920; i += step + 3) {
		struct vec3 rand;
		vec3_rand(&rand, 1);
		vec4_from_vec3(&color, &rand);
		color.w = 1.0f;
		//line_f(ctx->lines_ctx, 10 + i + step, 1080, 10 + i + step + 1 , 720, &color, step );
		step++;
	}

	lines_end(ctx->lines_ctx);

	setup_index_buffer(ctx);
	setup_vertex_buffer(ctx);

}

static const char *lineaa_source_get_name(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return LINEAA_PLUGIN_NAME;
}

static void lineaa_source_update(void *data, obs_data_t *settings)
{
	struct source_context *ctx = data;

	//draw_some_lines(ctx);

	UNUSED_PARAMETER(settings);
}

static void *lineaa_source_create(obs_data_t *settings, obs_source_t *source)
{
	struct source_context *ctx;
	gs_effect_t *effect;

	char *file = obs_module_file("line_aa.effect");
	char *file_tex = obs_module_file("texture.effect");
	ctx = bzalloc(sizeof(struct source_context));
	ctx->lines_ctx = bzalloc(sizeof(struct lines_context));
	
	lines_context_init(ctx->lines_ctx);
	
	lineaa_source_update(ctx, settings);

	obs_enter_graphics();
	effect = gs_effect_create_from_file(file, NULL);
	ctx->tex_effect = gs_effect_create_from_file(file_tex, NULL);
	ctx->texrender = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	obs_leave_graphics();

	bfree(file);
	bfree(file_tex);
	ctx->effect = effect;

	ctx->ep_w = gs_effect_get_param_by_name(effect, "width");
	ctx->ep_r = gs_effect_get_param_by_name(effect, "feather");
	ctx->ep_sx = gs_effect_get_param_by_name(effect, "ep_sx");
	ctx->ep_sy = gs_effect_get_param_by_name(effect, "ep_sy");
	ctx->ep_tex = gs_effect_get_param_by_name(ctx->tex_effect, "image");

	ctx->time = 0.0f;
	audio_window_t aw;
	audio_window_init(&aw, 2, 1024);
	audio_window_free(&aw);
	return ctx;
}

static void lineaa_source_destroy(void *data)
{
	struct source_context *ctx = data;
	
	obs_enter_graphics();
	gs_texrender_destroy(ctx->texrender);
	obs_leave_graphics();

	cleanup_index_buffer(ctx);
	cleanup_vertex_buffer(ctx);
	
	lines_context_reset(ctx->lines_ctx);
	
	bfree(ctx->lines_ctx);
	bfree(ctx);
}

static uint32_t lineaa_source_getwidth(void *data)
{
	UNUSED_PARAMETER(data);
	return 1920;
}


static uint32_t lineaa_source_getheight(void *data)
{
	UNUSED_PARAMETER(data);
	return 1080;
}


static void lineaa_source_show(void *data)
{
	struct lineaa_source *context = data;
	if (!context) return;
}

static void lineaa_source_hide(void *data)
{
	struct lineaa_source *context = data;
	if (!context) return;
}

static void lineaa_source_activate(void *data)
{
	struct lineaa_source *context = data;
	if (!context) return;
}

static void lineaa_source_deactivate(void *data)
{
	struct source_context *ctx = data;
	if (!ctx) return;
}

static void lineaa_source_render(void *data, gs_effect_t *effect)
{
	struct source_context *ctx = data;
	
	gs_vertbuffer_t *vb = ctx->vb;
	gs_indexbuffer_t *ib = ctx->ib;

	
	if (!vb || !ib) return;



	gs_vertexbuffer_flush(vb);
	gs_load_vertexbuffer(vb);
	gs_indexbuffer_flush(ib);
	gs_load_indexbuffer(ib);

	gs_effect_set_float(ctx->ep_w, 100);
	gs_effect_set_float(ctx->ep_r, 20);
	gs_effect_set_float(ctx->ep_sx, 1920);
	gs_effect_set_float(ctx->ep_sy, 1080);
	

	gs_texrender_begin(ctx->texrender, 1920 / 2, 1080 / 2);
	struct vec4 blank;
	vec4_zero(&blank);
	gs_clear(GS_CLEAR_COLOR, &blank, 0.0f, 0);
	
	while (gs_effect_loop(ctx->effect, "Draw"))
			gs_draw(GS_TRIS, 0, 0);
	
	gs_texrender_end(ctx->texrender);

	gs_load_indexbuffer(NULL);
	gs_load_vertexbuffer(NULL);

	gs_effect_set_texture(ctx->ep_tex, gs_texrender_get_texture(ctx->texrender));
	while (gs_effect_loop(ctx->tex_effect, "Draw"))
		gs_draw_sprite(gs_texrender_get_texture(ctx->texrender), 0, 1920, 1080);
	
	gs_texrender_reset(ctx->texrender);

}

static void lineaa_source_tick(void *data, float seconds)
{
	struct source_context *ctx = data;
	float cx, cy, x1, x2, y1, y2, x ,y;
	 
	float theta = ctx->time;
	ctx->time += seconds;

	lines_start(ctx->lines_ctx);
	cx = 1920.0f / 2.0f;
	cy = 1080.0f / 2.0f;
	x = 500.0f * cosf(theta * 0.5f) * sinf(theta);
	y = 500.0f * sinf(theta * 0.1f);
	x1 = cx + x;
	y1 = cy + y;
	x2 = cx - x;
	y2 = cy - y;
	struct vec4 color;
	vec4_from_rgba(&color, 0xFF10505F);
	line_f(ctx->lines_ctx, cx + x, cy + y, cx - x, cy - y, &color, 100 + 30 * sinf(theta));
	vec4_from_rgba(&color, 0xC0333333);
	line_f(ctx->lines_ctx, cx + y, cy + x, cx - y, cy - x, &color, 30 + 30 * cosf(theta));
	lines_end(ctx->lines_ctx);
	setup_index_buffer(ctx);
	setup_vertex_buffer(ctx);

}

static obs_properties_t *lineaa_source_properties(void *data)
{
	return NULL;
}

static void lineaa_source_defaults(obs_data_t *settings)
{

}

static struct obs_source_info lineaa_source_info = {
	.id = "lineaa_source",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW,
	.get_name = lineaa_source_get_name,
	.create = lineaa_source_create,
	.destroy = lineaa_source_destroy,
	.update = lineaa_source_update,
	.get_defaults = lineaa_source_defaults,
	.show = lineaa_source_show,
	.hide = lineaa_source_hide,
	.activate = lineaa_source_activate,
	.deactivate = lineaa_source_deactivate,
	.get_width = lineaa_source_getwidth,
	.get_height = lineaa_source_getheight,
	.video_tick = lineaa_source_tick,
	.video_render = lineaa_source_render,
	.get_properties = lineaa_source_properties
};

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("lineaa-source", "en-US")

bool obs_module_load(void)
{
	obs_register_source(&lineaa_source_info);
	return true;
}
