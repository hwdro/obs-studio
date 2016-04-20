#include "transform.hpp"

Transform::Transform()
{
	vec3_zero(&position);
	vec3_zero(&origin);
	vec3_zero(&rotation);
	vec3_set(&scale, 1.0f, 1.0f, 1.0f);
	changed = true;
}

void Transform::UpdateTransformMatrix()
{
	if (!changed)
		return;

	matrix4_identity(&transform);

	matrix4_translate3v(&transform, &transform, &origin);

	matrix4_rotate_aa4f(&transform, &transform, 1.0f, 0.0f, 0.0f, rotation.x);
	matrix4_rotate_aa4f(&transform, &transform, 0.0f, 1.0f, 0.0f, rotation.y);
	matrix4_rotate_aa4f(&transform, &transform, 0.0f, 0.0f, 1.0f, rotation.z);

	matrix4_scale(&transform, &transform, &scale);

	matrix4_translate3v(&transform, &transform, &position);

	changed = false;
}

void Transform::SetOrigin(float x, float y, float z)
{
	vec3_set(&origin, x, y, z);
	changed = true;
}

void Transform::SetPosition(float x, float y, float z)
{
	vec3_set(&position, x, y, z);
	changed = true;
}

void Transform::SetScale(float sx, float sy, float sz)
{
	vec3_set(&scale, sx, sy, sz);
	changed = true;
}

void Transform::IncreasePosition(float dx, float dy, float dz)
{
	vec3_set(&position, position.x + dx, position.y + dy, position.z + dz);
	changed = true;
}

void Transform::IncreaseRotation(float drx, float dry, float drz)
{
	vec3_set(&rotation, rotation.x + drx, rotation.y + dry, rotation.z + drz);
	changed = true;
}

matrix4 * Transform::GetTransformMatrix()
{
	UpdateTransformMatrix();
	return &transform;
}