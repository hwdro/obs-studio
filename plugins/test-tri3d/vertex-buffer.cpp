#include "vertex-buffer.hpp"


VertexBuffer::VertexBuffer()
	: vertex_buffer(nullptr)
{

}

VertexBuffer::~VertexBuffer()
{
	Reset();
}

void VertexBuffer::Reset()
{
	if (vertex_buffer) {
		obs_enter_graphics();
		gs_vertexbuffer_destroy(vertex_buffer);
		vertex_buffer = nullptr;
		obs_leave_graphics();
	}
}

void VertexBuffer::Load(vec3 *vertices, uint32_t *colors, vec2 *uvs,
	vec3 *normals, vec3 *tangents, size_t num_vertices)
{
	Reset();

	obs_enter_graphics();

	gs_vb_data *vbd = gs_vbdata_create();

	if (vertices) {
		vbd->points = static_cast<vec3 *>(bzalloc(sizeof(vec3) * num_vertices));
		memcpy(vbd->points, vertices, sizeof(vec3) * num_vertices);
		vbd->num = num_vertices;
	}

	if (colors) {
		vbd->colors = static_cast<uint32_t *>(bzalloc(sizeof(uint32_t) * num_vertices));
		memcpy(vbd->colors, colors, sizeof(uint32_t) * num_vertices);
	}

	if (uvs) {
		vbd->num_tex = 1;
		vbd->tvarray = static_cast<gs_tvertarray *>(bzalloc(sizeof(gs_tvertarray)));
		vbd->tvarray[0].width = 2;
		vbd->tvarray[0].array = static_cast<vec2 *>(bzalloc(sizeof(vec2) * num_vertices));
		memcpy(vbd->tvarray[0].array, uvs, sizeof(vec2) * num_vertices);
	}

	if (normals) {
		vbd->normals = static_cast<vec3 *> (bzalloc(sizeof(vec3) * num_vertices));
		memcpy(vbd->normals, normals, sizeof(vec3) * num_vertices);
	}

	if (tangents) {
		vbd->tangents = static_cast<vec3 *> (bzalloc(sizeof(vec3) * num_vertices));
		memcpy(vbd->tangents, tangents, sizeof(vec3) * num_vertices);
	}

	vertex_buffer = gs_vertexbuffer_create(vbd, GS_DYNAMIC);

	obs_leave_graphics();
}

void VertexBuffer::Enable()
{
	if (vertex_buffer) {
		gs_vertexbuffer_flush(vertex_buffer);
		gs_load_vertexbuffer(vertex_buffer);
	}
}

void VertexBuffer::Disable()
{
	gs_load_vertexbuffer(nullptr);
}

gs_vertbuffer_t * VertexBuffer::GetVertexBuffer()
{
	return vertex_buffer;
}
