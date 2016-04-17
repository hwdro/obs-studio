#pragma once

#include <obs-module.h>


class IndexBuffer {
private:
	gs_indexbuffer_t *index_buffer;
public:
	void Reset();
	void Load(uint32_t *indexes, size_t num_indexes);
	void Enable();
	void Disable();

	gs_indexbuffer_t * GetIndexBuffer();
	
	IndexBuffer();
	~IndexBuffer();
};