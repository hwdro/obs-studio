#include <obs-module.h>
#include <graphics/matrix4.h>
#include <graphics/quat.h>

#define SOURCE_NAME "Test Tri3D"

struct source_context {
	gs_effect_t *effect;
	uint32_t cx;
	uint32_t cy;
	float time;

	gs_vertbuffer_t  *vb;
	gs_indexbuffer_t *ib;

	gs_eparam_t *ep_mvp;

	struct matrix4 projection_matrix;
	struct matrix4 mvp;
};

typedef struct source_context source_context_t;

static const char *source_get_name(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return SOURCE_NAME;
}

static void setup_inf_projection_matrix(struct matrix4 *mat, float fov,
	float aspect, float near, float epsilon)
{
	vec4_zero(&mat->x);
	vec4_zero(&mat->y);
	vec4_zero(&mat->z);
	vec4_zero(&mat->t);

	float tha = tanf(fov / 2.0f);
	float range = tha * near;
	float left = -range * aspect;
	float right = range * aspect;
	float bottom = -range;
	float top = range;

	mat->x.x = 2.0f * near / (right - left);
	mat->y.y = 2.0f * near / (top - bottom);
	mat->z.z = epsilon - 1.0f;
	mat->z.w = -1.0f;
	mat->t.z = (epsilon - 2.0f) * near;
}

static void setup_projection_matrix(struct matrix4 *mat, float fov,
	float aspect, float zNear, float zFar)
{
	vec4_zero(&mat->x);
	vec4_zero(&mat->y);
	vec4_zero(&mat->z);
	vec4_zero(&mat->t);

	float tha = tanf(fov / 2.0f);

	mat->x.x = 1.0f / (aspect * tha);
	mat->y.y = 1.0f / tha;
	mat->z.z = - (zFar + zNear) / (zFar - zNear);
	mat->z.w = -1.0f;
	mat->t.z = -2.0f * zFar * zNear / (zFar - zNear);
	mat->t.w = 0.0f;
}


static void setup_view_matrix(struct matrix4 *mat, struct vec3 *position, struct vec3 *target)
{
	struct vec3 f;
	struct vec3 up = { 0.0f, 1.0f, 0.0f };
	struct vec3 s;
	struct vec3 u;

	vec3_sub(&f, target, position);
	vec3_norm(&f, &f);

	vec3_cross(&s, &f, &up);
	vec3_norm(&s, &s);
	
	vec3_cross(&u, &s, &f);

	matrix4_identity(mat);
		
	mat->x.x = s.x;
	mat->y.x = s.y;
	mat->z.x = s.z;

	mat->x.y = u.x;
	mat->y.y = u.y;
	mat->z.y = u.z;

	mat->x.z = -f.x;
	mat->y.z = -f.y;
	mat->z.z = -f.z;

	mat->t.x = -vec3_dot(&s, position);
	mat->t.y = -vec3_dot(&u, position);
	mat->t.z = vec3_dot(&f, position);

}

static void setup_model_matrix(struct matrix4 *mat, struct vec3 *position,  float rx, float ry , float rz)
{
	matrix4_identity(mat);
	matrix4_rotate_aa4f(mat, mat, 1.0f, 0.0f, 0.0f, rx);
	matrix4_rotate_aa4f(mat, mat, 0.0f, 1.0f, 0.0f, ry);
	matrix4_rotate_aa4f(mat, mat, 0.0f, 0.0f, 1.0f, rz);
	matrix4_translate3v(mat, mat, position);
	
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

	struct vec3 *vertices = bzalloc(sizeof(struct vec3) * 4);
	uint32_t *indexes = bzalloc(sizeof(uint32_t) * 6);
	uint32_t *colors = bzalloc(sizeof(uint32_t) * 4);

	struct vec3 v;

	float ar = (float)ctx->cx / (float)ctx->cy;
	


	vec3_set(&v, -1.0f * ar, 1.0f, 0.0f); // ---> vertex 1
	memcpy(vertices, &v, sizeof(struct vec3));

	vec3_set(&v, -1.0f * ar, -1.0f, 0.0f); // ---> vertex 2
	memcpy(vertices + 1, &v, sizeof(struct vec3));

	vec3_set(&v, 1.0f * ar, 1.0f, 0.0f); // ---> vertex 3
	memcpy(vertices + 2, &v, sizeof(struct vec3));

	vec3_set(&v, 1.0f * ar, -1.0f, 0.0f); // ---> vertex 4
	memcpy(vertices + 3, &v, sizeof(struct vec3));

	indexes[0] = 0;
	indexes[1] = 1;
	indexes[2] = 2;
	indexes[3] = 1;
	indexes[4] = 2;
	indexes[5] = 3;

	colors[0] = 0xFF00FF00;
	colors[1] = 0xFF0000FF;
	colors[2] = 0xFFFF00FF;
	colors[3] = 0xFF000000;
	

	struct gs_vb_data *vbd = gs_vbdata_create();

	vbd->num_tex = 0;
	vbd->tvarray = NULL;
	vbd->num = 4;
	vbd->points = vertices;
	vbd->colors = colors;
	vbd->normals = NULL;
	vbd->tangents = NULL;

	obs_enter_graphics();
	
	ctx->vb = gs_vertexbuffer_create(vbd, GS_DYNAMIC);
	ctx->ib =
	    gs_indexbuffer_create(GS_UNSIGNED_LONG, indexes, 6, GS_DYNAMIC);

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


	//ctx->ep_projection_matrix =
	//	gs_effect_get_param_by_name(effect, "projection_matrix");
	ctx->ep_mvp =
		gs_effect_get_param_by_name(effect, "mvp");


	ctx->effect = effect;
	
	//setup_projection_matrix(&ctx->projection_matrix, RAD(45),
	//	(float)ctx->cx / (float)ctx->cy, 0.10f, 1000.0f);
	setup_inf_projection_matrix(&ctx->projection_matrix, RAD(45),
		(float)ctx->cx / (float)ctx->cy, 0.1f, TINY_EPSILON);

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
	source_context_t *ctx = (source_context_t *)data;
	ctx->time += seconds;

	struct vec3 cam = { 0, 0, 1};
	struct vec3 la = { 0, 0 , -1};
	struct vec3 obj_pos = { 0.0, 0, -1 };
	struct matrix4 obj;
	float rx = 0; fmodf(ctx->time, 2 * M_PI);
	float ry = fmodf(ctx->time / 2.0f + M_PI / 8.0f, 2 * M_PI);
	float rz = 0; fmodf(ctx->time + M_PI / 4.0f, 2 * M_PI);

	
	setup_model_matrix(&obj, &obj_pos, rx, -ry, rz);
	//matrix4_identity(&obj);
	
	setup_view_matrix(&ctx->mvp, &cam, &la);

	
	//matrix4_mul(&ctx->view_matrix, &obj, &ctx->view_matrix);
	//matrix4_mul(&ctx->view_matrix, &ctx->view_matrix, &ctx->projection_matrix);
	//matrix4_copy(&ctx->view_matrix, &ctx->projection_matrix);
	matrix4_mul(&ctx->mvp, &obj, &ctx->mvp);
	matrix4_mul(&ctx->mvp, &ctx->mvp, &ctx->projection_matrix);
	//matrix4_mul(&ctx->view_matrix, &obj, &ctx->projection_matrix);
	//matrix4_copy(&ctx->view_matrix, &obj);
	//matrix4_identity(&ctx->view_matrix);

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
	gs_enable_depth_test(true);

	//gs_frustum(-0.5f, 0.5, 0.5f, -0.5, 1.0f, 10.0f);
	//gs_effect_set_matrix4(ctx->ep_projection_matrix, &ctx->projection_matrix);
	gs_effect_set_matrix4(ctx->ep_mvp, &ctx->mvp);
	//gs_matrix_set(&ctx->view_matrix);
	while (gs_effect_loop(ctx->effect, "Draw"))
		gs_draw(GS_TRIS, 0, 0);

	gs_enable_depth_test(false);
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