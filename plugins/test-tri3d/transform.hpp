#pragma once

#include "graphics/vec3.h"
#include "graphics/matrix4.h"
#include "graphics/quat.h"


class Transform {
private:
	matrix4 transform;
	vec3 origin;
	vec3 position;
	vec3 scale;
	vec3 rotation;
	bool changed;
	void UpdateTransformMatrix();
public:
	Transform();
	void SetOrigin(float x, float y, float z);
	void SetPosition(float x, float y, float z);
	void SetScale(float sx, float sy, float sz);
	void IncreasePosition(float dx, float dy, float dz);
	void IncreaseRotation(float drx, float dry, float drz);
	matrix4 * GetTransformMatrix();
};