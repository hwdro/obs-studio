#pragma once

#include <assert.h>
#include <stdlib.h>
#include "util\bmem.h"

/**
 * Fixed size sliding buffer
 * Usage example:
 * Samples window for FFT where a power by 2 sized input buffer is
 * required while the audio frame size isn't a power by 2, e.g. 1024 samples
 * window while the audio frame has only 480 samples.
 */

struct slidingbuf {
	void *data;
	size_t size;
};
typedef struct slidingbuf slidingbuf_t;

/* Dynamic sliding buffer creation / destruction */
static inline struct slidingbuf *slidingbuf_create(size_t size)
{
	struct slidingbuf *sb;

	sb       = bzalloc(sizeof(struct slidingbuf));
	sb->data = bzalloc(size);
	sb->size = size;

	return sb;
}

static inline void slidingbuf_destroy(struct slidingbuf *sb)
{
	assert(sb != NULL);
	assert(sb->data != NULL);

	bfree(sb->data);
	bfree(sb);
}

/* Static sliding buffer init / free */
static inline void slidingbuf_init(struct slidingbuf *sb, size_t size)
{
	memset(sb, 0, sizeof(struct slidingbuf));
	sb->data = bzalloc(size);
	sb->size = size;
}

static inline void slidingbuf_free(struct slidingbuf *sb)
{
	bfree(sb->data);
	memset(sb, 0, sizeof(struct slidingbuf));
}

/* Input operations */
static inline void slidingbuf_push_back(struct slidingbuf *sb, void *data,
					size_t size)
{
	assert(size <= sb->size);

	size_t offset = sb->size - size;

	if (offset)
		memmove(sb->data, (uint8_t *)sb->data + size, offset);

	memcpy((uint8_t *)sb->data + offset, (uint8_t *)data, size);
}

static inline void slidingbuf_push_front(struct slidingbuf *sb, void *data,
					 size_t size)
{
	assert(size <= sb->size);

	size_t offset = sb->size - size;

	if (offset)
		memmove((uint8_t *)sb->data + size, sb->data, offset);

	memcpy(sb->data, data, size);
}
