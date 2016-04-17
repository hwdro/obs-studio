#include <obs.h>
#include "object3D.h"



object3D_t * object3D_create(void)
{
	return bzalloc(sizeof(object3D_t));
}

void object3D_delete(object3D_t *obj)
{
	bfree(obj);
}

void object3D_set_position(object3D_t *obj, float x, float y, float z)
{
	vec3_set(&obj->position, x, y, z);
}

void object3D_set_rotation(object3D_t *obj, float rx, float ry, float rz)
{
	quat_set(&obj->rotation, rx, ry, rz, 1);
}

void object3D_set_position_v(object3D_t *obj, struct vec3 *pos)
{
	vec3_copy(&obj->position, pos);
}

void object3D_set_rotation_q(object3D_t *obj, struct quat *rq)
{
	quat_copy(&obj->rotation, rq);
}

void object3D_move(object3D_t *obj, float dx, float dy, float dz)
{
	struct vec3 d = { dx, dy, dz };
	vec3_add(&obj->position, &obj->position, &d);
}

void object3D_rotate(object3D_t *obj, float drx, float dry, float drz)
{
	struct quat q = { drx, dry, drz, 1.0f };
	quat_add(&obj->rotation, &obj->rotation, &q);
}

