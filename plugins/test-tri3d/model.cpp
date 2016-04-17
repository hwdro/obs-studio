#include "model.hpp"
//#include <util/bmem.h>


void Model::EnableBuffers()
{
	vertex_buffer.Enable();
	index_buffer.Enable();
}

void Model::DisableBuffers()
{
	vertex_buffer.Disable();
	index_buffer.Disable();
}

Model::Model()
{

}

Model::~Model()
{
}

void Rectangle::Set(float width, float height)
{
	float hw = width / 2;
	float hh = height / 2;
	vec3 verts[4];
	//vec2 uvs[4];
	uint32_t indexes[6];
	uint32_t colors[4];

	vec3_set(&verts[0], -hw, hh, 0.0f);
	vec3_set(&verts[1], -hw, -hh, 0.0f);
	vec3_set(&verts[2], hw, hh, 0.0f);
	vec3_set(&verts[3], hw, -hh, 0.0f);

	colors[0] = 0xFF0000FF;
	colors[1] = 0xFF00FF00;
	colors[2] = 0xFFFF0000;
	colors[3] = 0xFFFFFFFF;

	indexes[0] = 0;
	indexes[1] = 1;
	indexes[2] = 2;
	indexes[3] = 2;
	indexes[4] = 1;
	indexes[5] = 3;

	vertex_buffer.Load(verts, colors, nullptr, nullptr, nullptr, 4);
	index_buffer.Load(indexes, 6);
}

void Cube::Set()
{
	vec3 vertices[] = {
		{ -0.5f, 0.5f, -0.5f },
		{ -0.5f, -0.5f, -0.5f },
		{ 0.5f, -0.5f, -0.5f },
		{ 0.5f, 0.5f, -0.5f },

		{ -0.5f, 0.5f, 0.5f },
		{ -0.5f, -0.5f, 0.5f },
		{ 0.5f, -0.5f, 0.5f },
		{ 0.5f, 0.5f, 0.5f },

		{ 0.5f, 0.5f, -0.5f },
		{ 0.5f, -0.5f, -0.5f },
		{ 0.5f, -0.5f, 0.5f },
		{ 0.5f, 0.5f, 0.5f },

		{ -0.5f, 0.5f, -0.5f },
		{ -0.5f, -0.5f, -0.5f },
		{ -0.5f, -0.5f, 0.5f },
		{ -0.5f, 0.5f, 0.5f },

		{ -0.5f, 0.5f, 0.5f },
		{ -0.5f, 0.5f, -0.5f },
		{ 0.5f, 0.5f, -0.5f },
		{ 0.5f, 0.5f, 0.5f },

		{ -0.5f, -0.5f, 0.5f },
		{ -0.5f, -0.5f, -0.5f },
		{ 0.5f, -0.5f, -0.5f },
		{ 0.5f, -0.5f, 0.5f }

	};

	uint32_t indices [] = {
		0, 1, 3,
		3, 1, 2,
		4, 5, 7,
		7, 5, 6,

		8, 9, 11,
		11, 9, 10,
		12, 13, 15,
		15, 13, 14,

		16, 17, 19,
		19, 17, 18,
		20, 21, 23,
		23, 21, 22
	};

	uint32_t colors[24]; 

	for (int i = 0; i < 24; i += 4) {
		colors[i + 0] = 0xFF0000FF;
		colors[i + 1] = 0xFF00FF00;
		colors[i + 2] = 0xFFFF0000;
		colors[i + 3] = 0xFFFFFFFF;
	}
	vertex_buffer.Load(vertices, colors, nullptr, nullptr, nullptr, 24);
	index_buffer.Load(indices, 36);
}