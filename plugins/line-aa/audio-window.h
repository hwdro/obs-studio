#pragma once

#include <obs.h>
#include "slidingbuf.h"

struct audio_window {
	slidingbuf_t *windows[MAX_AV_PLANES];
	size_t channels;
	size_t frame_size;
};
typedef struct audio_window audio_window_t;


/* We're working with samples as floats */

static inline audio_window_init(audio_window_t *aw, size_t channels,
				size_t size)
{
	assert(aw != NULL);
	assert(channels <= MAX_AV_PLANES);

	memset(aw, 0, sizeof(audio_window_t));
	
	for (uint32_t c = 0; c < channels; c++)
		aw->windows[c] = slidingbuf_create(size * sizeof(float));

	aw->channels   = channels;
	aw->frame_size = size;
}

static inline audio_window_free(audio_window_t *aw)
{
	assert(aw != NULL);

	for (uint32_t c = 0; c < aw->channels; c++)
		slidingbuf_destroy(aw->windows[c]);

	memset(aw, 0, sizeof(audio_window_t));
}

static inline float *audio_window_get_buffer(audio_window_t *aw, size_t channel)
{
	assert(aw != NULL);
	assert(channel < aw->channels);
	assert(aw->windows[channel] != NULL);

	return (float *)aw->windows[channel]->data;
}

static inline size_t audio_window_get_frame_size(audio_window_t *aw)
{
	assert(aw);

	return aw->frame_size;
}

static inline void audio_window_push_frame(audio_window_t *aw, size_t channel,
					   float *frame, size_t size)
{
	assert(aw != NULL);
	assert(frame != NULL);
	assert(channel < aw->channels);
	assert(size <= aw->frame_size);

	slidingbuf_push_back(aw->windows[channel], frame, size * sizeof(float));
}
