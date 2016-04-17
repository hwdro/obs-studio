#pragma once
#include <obs-module.h>
#include <graphics/matrix4.h>


class Camera {
private:
	matrix4 projection_matrix;
	matrix4 view_matrix;
	vec3 position;
	vec3 direction;
	void UpdateViewMatrix();
public:
	Camera();
	void LookAt(vec3 *pos, vec3 *lookAt);
	void Perspective(float aspect, float fov, float zNear, float zFar);
	matrix4 * GetProjectionMatrix();
	matrix4 * GetViewMatrix();
};