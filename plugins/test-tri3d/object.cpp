#include "object.hpp"


Object::Object(vec3 &pos, vec3 &scale, float rx, float ry, float rz)
{
	transform.SetPosition(pos.x, pos.y, pos.z);
	transform.SetScale(scale.x, scale.y, scale.z);
}

Object::Object()
{
}

void Object::AttachModel(Model *mdl)
{
	model = mdl;
}

void Object::AttachCamera(Camera *cam)
{
	camera = cam;
}

void Object::LoadEffect(const char *file_name)
{
	effect.Load(file_name);
	ep_transform.Load(effect, "transform_matrix");
	ep_view.Load(effect, "view_matrix");
	ep_proj.Load(effect, "proj_matrix");
}

void Object::Tick(float seconds)
{
}

void Object::BeginRendering()
{
	model->EnableBuffers();
	ep_transform.Set(transform.GetTransformMatrix());
	ep_view.Set(camera->GetViewMatrix());
	ep_proj.Set(camera->GetProjectionMatrix());
}

void Object::Render()
{
	while (gs_effect_loop(effect.GetEffect(), "Draw"))
		gs_draw(GS_TRIS, 0, 0);
}

void Object::EndRendering()
{
	model->DisableBuffers();
}
