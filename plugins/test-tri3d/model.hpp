#pragma once
#include <obs-module.h>

#include "vertex-buffer.hpp"
#include "index-buffer.hpp"
#include "effect.hpp"

class Model {
protected:
	VertexBuffer vertex_buffer;
	IndexBuffer index_buffer;
public:
	void EnableBuffers();
	void DisableBuffers();
	Model();
	~Model();
};

class Plane: public Model {
public:
	void Set(float width, float height, vec4 *color);
};

class Cube : public Model {
public:
	void Set();
};