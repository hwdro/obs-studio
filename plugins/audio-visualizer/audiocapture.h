#pragma once

#include <obs.h>
#include <util/dstr.h>
#include <util/threading.h>

#include "audiobuf.h"

struct audiocapture {
	audiobuf_t audio_buffers;
	pthread_mutex_t data_mutex;

	struct dstr audio_source_name;
	obs_source_t *audio_source;
	volatile bool data_consumed;
};
typedef struct audiocapture audiocapture_t;

audiocapture_t *audiocapture_create(const char *src_name, uint32_t channels,
			    size_t size);

void audiocapture_destroy(audiocapture_t *ac);

// If audio capture is already active, true is returned.
// If not, an attempt is made to start audio capturing and the result of that
// attempt is returned;

bool audiocapture_is_active(audiocapture_t *ac);

// Copy current audio buffers. It does nothing if audio capturing isn't started
// or if there is no new data.

bool audiocapture_copy_buffers(audiocapture_t *ac, audiobuf_t *dst);



