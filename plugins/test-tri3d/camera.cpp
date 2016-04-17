#include "camera.hpp"

void Camera::Perspective(float aspect, float fov, float zNear, float zFar)
{
	vec4_zero(&projection_matrix.x);
	vec4_zero(&projection_matrix.y);
	vec4_zero(&projection_matrix.z);
	vec4_zero(&projection_matrix.t);

	float tha = tanf(fov / 2.0f);

	projection_matrix.x.x = 1.0f / (aspect * tha);
	projection_matrix.y.y = 1.0f / tha;
	projection_matrix.z.z = -(zFar + zNear) / (zFar - zNear);
	projection_matrix.z.w = -1.0f;
	projection_matrix.t.z = -2.0f * zFar * zNear / (zFar - zNear);
	projection_matrix.t.w = 0.0f;
}

void Camera::UpdateViewMatrix()
{
	struct vec3 f;
	struct vec3 up = { 0.0f, 1.0f, 0.0f };
	struct vec3 s;
	struct vec3 u;

	vec3_sub(&f, &direction, &position);
	vec3_norm(&f, &f);

	vec3_cross(&s, &f, &up);
	vec3_norm(&s, &s);

	vec3_cross(&u, &s, &f);

	matrix4_identity(&view_matrix);

	view_matrix.x.x = s.x;
	view_matrix.y.x = s.y;
	view_matrix.z.x = s.z;

	view_matrix.x.y = u.x;
	view_matrix.y.y = u.y;
	view_matrix.z.y = u.z;

	view_matrix.x.z = -f.x;
	view_matrix.y.z = -f.y;
	view_matrix.z.z = -f.z;

	view_matrix.t.x = -vec3_dot(&s, &position);
	view_matrix.t.y = -vec3_dot(&u, &position);
	view_matrix.t.z = vec3_dot(&f, &position);
}


Camera::Camera()
{
	vec3 pos = { 0.0f, 0.0f, 0.0f };
	vec3 dir = { 0.0f, 0.0f, -1.0f };
	LookAt(&pos, &dir);
	
}

void Camera::LookAt(vec3 *pos, vec3 *dir)
{
	vec3_copy(&position, pos);
	vec3_copy(&direction, dir);
	UpdateViewMatrix();
}

matrix4 * Camera::GetProjectionMatrix()
{
	return &projection_matrix;
}

matrix4 * Camera::GetViewMatrix()
{
	return &view_matrix;
}