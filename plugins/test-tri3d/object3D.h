#pragma once
#include <graphics/vec3.h>
#include <graphics/quat.h>
#include <util/darray.h>

struct object3D{
	struct vec3 position;
	struct quat rotation;
};

typedef struct object3D object3D_t;

object3D_t * object3D_create(void);
void object3D_delete(object3D_t *obj);

void object3D_set_position(object3D_t *obj, float x, float y, float z);
void object3D_set_rotation(object3D_t *obj, float rx, float ry, float rz);

void object3D_set_position_v(object3D_t *obj, struct vec3 *pos);
void object3D_set_rotation_q(object3D_t *obj, struct quat *rq);

void object3D_move(object3D_t *obj, float dx, float dy, float dz);
void object3D_rotate(object3D_t *obj, float drx, float dry, float drz);

