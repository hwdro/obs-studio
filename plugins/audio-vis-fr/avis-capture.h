#pragma once

#include <obs.h>
#include <util/dstr.h>

struct avis_capture {
	struct dstr source_name;
	obs_source_t *source;
};
typedef struct avis_capture avis_capture_t;

avis_capture_t * avis_capture_create(const char *source_name, uint32_t channels,
	uint32_t frame_size);
void avis_capture_destroy(avis_capture_t *ac);