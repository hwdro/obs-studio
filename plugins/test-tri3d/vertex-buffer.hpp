#include <obs-module.h>


class VertexBuffer {
private:
	gs_vertbuffer_t *vertex_buffer;
public:
	void Reset();
	void Load(vec3 *vertices, uint32_t *colors, vec2 *uvs, vec3 *normals, vec3 *tangents, size_t num_vertices);
	
	void Enable();
	void Disable();

	gs_vertbuffer_t * GetVertexBuffer();
	VertexBuffer();
	~VertexBuffer();
};
