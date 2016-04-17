#include <graphics/vec2.h>
#include "object3D.h"


struct rectangle{
	struct vec3 vertices[4];
	struct vec2 uv[4];
	uint32_t indices[6];
	object3D_t *coords;
};

typedef struct rectangle rectangle_t;

rectangle_t * rectangle_create()
{
	rectangle_t *r = bzalloc(sizeof(rectangle_t));
	r->coords = object3D_create();
}

void rectangle_delete(rectangle_t *r)
{
	object3D_delete(r->coords);
	bfree(r);
}

void rectangle_set_size(rectangle_t *r, float width, float height)
{
	float hw = width / 2.0f;
	float hh = height / 2.0f;

	vec3_set(&r->vertices[0], -hw, hh, 0.0f);
	vec3_set(&r->vertices[0], -hw, -hh, 0.0f);
	vec3_set(&r->vertices[0], hw, hh, 0.0f);
	vec3_set(&r->vertices[0], hw, -hh, 0.0f);

	r->indices[0] = 0;
	r->indices[1] = 1;
	r->indices[2] = 2;
	r->indices[3] = 2;
	r->indices[4] = 1;
	r->indices[5] = 3;

}

void rectangle_set_uv(rectangle_t *r, struct vec2 *topleft, struct vec2 *bottomright)
{
	vec2_copy(&r->uv[0], topleft);
	vec2_set(&r->uv[1], topleft->x, bottomright->y);
	vec2_set(&r->uv[2], bottomright->x, topleft->y);
	vec2_copy(&r->uv[3], bottomright);
}