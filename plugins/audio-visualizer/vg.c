#include <obs.h>
#include "vg.h"

static void line_normal(struct vec3 *dst, struct vec3 *v1, struct vec3 *v2)
{
	struct vec3 diff;

	vec3_sub(&diff, v2, v1);
	vec3_set(dst, -diff.y, diff.x, 0.0f);
	vec3_norm(dst, dst);
}

void vg_draw_line_v(vg_context_t *ctx, struct vec3 *start, struct vec3 *end,
		    struct vec4 *color, float width)
{
	vg_shape_t shape;
	struct vec3 normal, neg_normal;

	shape.shape_type = VG_LINE;
	shape.color_mode = VG_COLOR_PER_SHAPE;

	line_normal(&normal, end, start);

	vec3_mulf(&normal, &normal, width);
	vec3_neg(&neg_normal, &normal);

	vec3_add(&shape.vertices[0], start, &normal);
	vec3_add(&shape.vertices[1], start, &neg_normal);
	vec3_add(&shape.vertices[2], end, &normal);
	vec3_add(&shape.vertices[3], end, &normal);

	for (uint32_t i = 0; i < 4; i++)
		vec4_copy(&shape.colors[i], color);

	da_push_back(ctx->shape_list, &shape);
}

void vg_draw_line_f(vg_context_t *ctx, float sx, float sy, float ex, float ey,
		    struct vec4 *color, float width)
{
	struct vec3 start, end;
	vec3_set(&start, sx, sy, 0.0f);
	vec3_set(&end, ex, ey, 0.0f);
	vg_draw_line_v(ctx, &start, &end, color, width);
}

void vg_draw_rectangle_v(vg_context_t *ctx, struct vec3 *top_left,
			 struct vec3 *bottom_right, struct vec4 *color)
{
	struct vec3 top_right, bottom_left;

	vg_shape_t shape;

	shape.shape_type = VG_RECTANGLE;
	shape.color_mode = VG_COLOR_PER_SHAPE;

	vec3_set(&top_right, bottom_right->x, top_left->y, 0.0f);
	vec3_set(&bottom_left, top_left->x, bottom_right->y, 0.0f);

	vec3_copy(&shape.vertices[0], top_left);
	vec3_copy(&shape.vertices[1], &top_right);
	vec3_copy(&shape.vertices[2], &bottom_left);
	vec3_copy(&shape.vertices[3], bottom_right);

	for (uint32_t i = 0; i < 4; i++)
		vec4_copy(&shape.colors[i], color);

	da_push_back(ctx->shape_list, &shape);
}

void vg_draw_rectangle_f(vg_context_t *ctx, float x, float y, float x2,
			 float y2, struct vec4 *color)
{
	struct vec3 top_left, bottom_right;

	vec3_set(&top_left, x, y, 0.0f);
	vec3_set(&bottom_right, x2, y2, 0.0f);

	vg_draw_rectangle_v(ctx, &top_left, &bottom_right, color);
}

void vg_draw_circle_v(vg_context_t *ctx, struct vec3 *center, float radius,
		      struct vec4 *color)
{
	vg_shape_t shape;

	shape.shape_type = VG_CIRCLE;
	shape.color_mode = VG_COLOR_PER_SHAPE;

	vec3_set(&shape.vertices[0], center->x - radius, center->y - radius,
		 0.0f);
	vec3_set(&shape.vertices[1], center->x + radius, center->y - radius,
		 0.0f);
	vec3_set(&shape.vertices[2], center->x - radius, center->y + radius,
		 0.0f);
	vec3_set(&shape.vertices[3], center->x + radius, center->y + radius,
		 0.0f);

	for (uint32_t i = 0; i < 4; i++)
		vec4_copy(&shape.colors[i], color);

	da_push_back(ctx->shape_list, &shape);
}

void vg_draw_circle_f(vg_context_t *ctx, float cx, float cy, float radius,
		      struct vec4 *color)
{
	struct vec3 center;

	vec3_set(&center, cx, cy, 0.0f);

	vg_draw_circle_v(ctx, &center, radius, color);
}


static void vg_cleanup_buffers(vg_context_t *ctx)
{
	obs_enter_graphics();

	if (ctx->vb) {
		gs_vertbuffer_t *tmp;
		tmp = ctx->vb;
		gs_vertexbuffer_destroy(tmp);
		ctx->vb = NULL;
	}

	if (ctx->ib) {
		gs_indexbuffer_t *tmp;
		tmp = ctx->ib;
		gs_indexbuffer_destroy(tmp);
		ctx->ib = NULL;
	}
	obs_leave_graphics();
}



static void vg_setup_gs_buffers(vg_context_t *ctx)
{
	size_t num_shapes = ctx->shape_list.num;

	struct vec3 *vertices = bzalloc(sizeof(struct vec3) * num_shapes * 4);
	uint32_t *indexes     = bzalloc(sizeof(uint32_t) * num_shapes * 6);
	uint32_t *colors      = bzalloc(sizeof(uint32_t) * num_shapes * 4);

	for (size_t i = 0; i < num_shapes; i++) {
		vg_shape_t *shape =
		    darray_item(sizeof(vg_shape_t),
				(const struct darray *)&ctx->shape_list, i);

		memcpy(vertices + i * 4, shape->vertices,
		       sizeof(struct vec3) * 4);

		for (uint32_t k = 0; k < 4; k++)
			colors[i * 4 + k] = vec4_to_rgba(&shape->colors[k]);

		for (uint32_t t = 0; t < 3; t++) {
			indexes[i * 6 + t] = (uint32_t)i * 4 + t;
			indexes[i * 6 + t + 3] = (uint32_t)i * 4 + 3 - t;
		}
	}


	vg_cleanup_buffers(ctx);

	obs_enter_graphics();
	
	struct gs_vb_data *vbd = gs_vbdata_create();

	vbd->num_tex  = 0;
	vbd->tvarray  = NULL;
	vbd->num      = num_shapes * 4;
	vbd->points   = vertices;
	vbd->colors   = colors;
	vbd->normals  = NULL;
	vbd->tangents = NULL;

	ctx->vb = gs_vertexbuffer_create(vbd, GS_DYNAMIC);
	ctx->ib = gs_indexbuffer_create(GS_UNSIGNED_LONG, indexes,
					num_shapes * 6, GS_DYNAMIC);
	obs_leave_graphics();
}

void vg_context_init(vg_context_t *ctx)
{
	da_init(ctx->shape_list);
	ctx->vb = NULL;
	ctx->ib = NULL;
}

void vg_context_free(vg_context_t *ctx)
{
	da_free(ctx->shape_list);
	vg_cleanup_buffers(ctx);
}

void vg_begin_paint(vg_context_t *ctx)
{
	da_free(ctx->shape_list);
}

void vg_end_paint(vg_context_t *ctx)
{
	vg_setup_gs_buffers(ctx);
}