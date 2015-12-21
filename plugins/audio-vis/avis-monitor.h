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

#include <obs.h>
#include <util/dstr.h>
#include <util/threading.h>

#include "avis-buffer.h"

struct avis_monitor {
	obs_source_t    *source;
	avis_buffer_t    *data;
	struct dstr     name;
	pthread_mutex_t data_mutex;
	float		volume;
};
typedef struct avis_monitor avis_monitor_t;

avis_monitor_t * avis_monitor_create(const char *name,
	uint32_t sample_rate, uint32_t channels, size_t size);
void avis_monitor_destroy(avis_monitor_t *monitor);
void avis_monitor_acquire_obs_source(avis_monitor_t *monitor);
void avis_monitor_release_obs_source(avis_monitor_t *monitor);
