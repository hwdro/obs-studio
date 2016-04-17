#include "index-buffer.hpp"


void IndexBuffer::Reset()
{
	if (index_buffer) {
		obs_enter_graphics();
		gs_indexbuffer_destroy(index_buffer);
		obs_leave_graphics();
		index_buffer = nullptr;
	}
}

void IndexBuffer::Load(uint32_t *indexes, size_t num_indexes)
{
	Reset();
	if (indexes) {
		uint32_t *idx = static_cast<uint32_t *>(bzalloc(sizeof(uint32_t) * num_indexes));
		memcpy(idx, indexes, sizeof(uint32_t) * num_indexes);

		obs_enter_graphics();
		index_buffer = gs_indexbuffer_create(GS_UNSIGNED_LONG, idx, num_indexes, GS_DYNAMIC);
		obs_leave_graphics();
	}
}

void IndexBuffer::Enable()
{
	if (index_buffer) {
		gs_indexbuffer_flush(index_buffer);
		gs_load_indexbuffer(index_buffer);
	}
}

void IndexBuffer::Disable()
{
	gs_load_indexbuffer(nullptr);
}

gs_indexbuffer_t * IndexBuffer::GetIndexBuffer()
{
	return index_buffer;
}

IndexBuffer::IndexBuffer()
	: index_buffer(nullptr)
{

}

IndexBuffer::~IndexBuffer()
{
	Reset();
}

