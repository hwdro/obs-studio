/******************************************************************************
Copyright (C) 2015 by HomeWorld <homeworld@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#pragma once

#include <obs-module.h>
#include <util/dstr.h>
#include <util/threading.h>

#include "audio-buffer.h"

struct audio_monitor {
	obs_source_t    *source;
	audio_buffer_t  *data;
	struct dstr     name;
	pthread_mutex_t data_mutex;
};
typedef struct audio_monitor audio_monitor_t;

audio_monitor_t * audio_monitor_init(const char *name,
	uint32_t sample_rate, uint32_t channels, size_t size);
void audio_monitor_destroy(audio_monitor_t *am);
void audio_monitor_acquire_obs_source(audio_monitor_t *am);
void audio_monitor_release_obs_source(audio_monitor_t *am);
