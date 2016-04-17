#pragma once

#include <obs-module.h>
#include <graphics/matrix4.h>

#include "model.hpp"
#include "transform.hpp"
#include "effect.hpp"
#include "camera.hpp"



class Object {
	Model *model;
	Camera *camera;
	Effect effect;
	EffectParam ep_transform;
	EffectParam ep_view;
	EffectParam ep_proj;

public:
	Transform transform;

	Object(vec3 &pos, vec3 &scale, float rx, float ry, float rz);
	Object();
	void AttachModel(Model *);
	void AttachCamera(Camera *);
	void LoadEffect(const char *file_name);
	void Tick(float seconds);
	void BeginRendering();
	void Render();
	void EndRendering();
};