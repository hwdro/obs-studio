#pragma once
#include <obs.h>

enum VG_PRIMITIVE_TYPE {
	VG_LINE,
	VG_RECTANGLE,
	VG_CIRCLE,
	VG_TRIANGLE
};

enum VG_COLOR_MODE {
	VG_COLOR_PER_SHAPE,
	VG_COLOR_PER_VERTEX
};

struct vg_shape {
	struct vec3 vertices[4];
	struct vec4 colors[4];
	enum VG_PRIMITIVE_TYPE shape_type;
	enum VG_COLOR_MODE color_mode;
};
typedef struct vg_shape vg_shape_t;

struct vg_context {
	DARRAY(vg_shape_t) shape_list;
	gs_vertbuffer_t *vb;
	gs_indexbuffer_t *ib;
};

typedef struct vg_context vg_context_t;


void vg_context_init(vg_context_t *ctx);
void vg_context_free(vg_context_t *ctx);
void vg_begin_paint(vg_context_t *ctx);
void vg_end_paint(vg_context_t *ctx);

void vg_draw_line_v(vg_context_t *ctx, struct vec3 *start, struct vec3 *end,
struct vec4 *color, float width);

void vg_draw_line_f(vg_context_t *ctx, float sx, float sy, float ex, float ey,
struct vec4 *color, float width);

void vg_draw_rectangle_v(vg_context_t *ctx, struct vec3 *top_left,
struct vec3 *bottom_right, struct vec4 *color);

void vg_draw_rectangle_f(vg_context_t *ctx, float x, float y, float x2,
	float y2, struct vec4 *color);

void vg_draw_circle_v(vg_context_t *ctx, struct vec3 *center, float radius,
struct vec4 *color);

void vg_draw_circle_f(vg_context_t *ctx, float cx, float cy, float radius,
struct vec4 *color);

