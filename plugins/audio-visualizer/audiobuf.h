#pragma once

#include <obs.h>
#include "slidingbuf.h"

struct audiobuf {
	slidingbuf_t *sbuffers[MAX_AV_PLANES];
	size_t channels;
	size_t frame_size;
	
	float volume;
	bool muted;
};
typedef struct audiobuf audiobuf_t;


/* We're working with samples as floats */

static inline audiobuf_init(audiobuf_t *ab, size_t channels,
				size_t size)
{
	assert(ab != NULL);
	assert(channels <= MAX_AV_PLANES);

	memset(ab, 0, sizeof(audiobuf_t));
	
	for (uint32_t c = 0; c < channels; c++)
		ab->sbuffers[c] = slidingbuf_create(size * sizeof(float));

	ab->channels   = channels;
	ab->frame_size = size;
}

static inline audiobuf_free(audiobuf_t *ab)
{
	assert(ab != NULL);

	for (uint32_t c = 0; c < ab->channels; c++)
		slidingbuf_destroy(ab->sbuffers[c]);

	memset(ab, 0, sizeof(audiobuf_t));
}

static inline float *audiobuf_get_buffer(audiobuf_t *ab, size_t channel)
{
	assert(ab != NULL);
	assert(channel < ab->channels);
	assert(ab->sbuffers[channel] != NULL);

	return (float *)ab->sbuffers[channel]->data;
}

static inline size_t audiobuf_get_frame_size(audiobuf_t *ab)
{
	assert(ab);

	return ab->frame_size;
}

static inline void audiobuf_push_frame(audiobuf_t *ab, size_t channel,
					   float *frame, size_t size)
{
	assert(ab != NULL);
	assert(frame != NULL);
	assert(channel < ab->channels);
	assert(size <= ab->frame_size);

	slidingbuf_push_back(ab->sbuffers[channel], frame, size * sizeof(float));
}
