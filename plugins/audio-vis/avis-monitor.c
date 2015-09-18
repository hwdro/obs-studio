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

#include "avis-monitor.h"

avis_monitor_t * avis_monitor_create(const char *name, uint32_t sample_rate,
	uint32_t channels, size_t size)
{
	avis_monitor_t *monitor;
	
	monitor = bzalloc(sizeof(avis_monitor_t));
	
	monitor->data = avis_buffer_create(sample_rate, channels, size);
	pthread_mutex_init(&monitor->data_mutex, NULL);
	dstr_init_copy(&monitor->name, name);

	avis_monitor_acquire_obs_source(monitor);

	return monitor;
}

void avis_monitor_destroy(avis_monitor_t *monitor)
{
	if (!monitor) return;

	avis_monitor_release_obs_source(monitor);
	dstr_free(&monitor->name);
	avis_buffer_destroy(monitor->data);
	pthread_mutex_destroy(&monitor->data_mutex);
	
	bfree(monitor);
}


static void avis_monitor_removed_signal(void *vptr, calldata_t *calldata);
static void avis_monitor_data_received_signal(void *vptr,
	calldata_t *calldata);

void avis_monitor_acquire_obs_source(avis_monitor_t *monitor)
{
	if (!monitor) return;
	if (monitor->source) return;

	signal_handler_t *sh;
	obs_source_t *src = NULL;
	struct dstr *m_name = &monitor->name;

	for (uint32_t i = 1; i <= 10; i++) {
		obs_source_t *source = obs_get_output_source(i);
		if (!source) continue;

		bool is_audio_source = !!(obs_source_get_output_flags(source) & 
			OBS_SOURCE_AUDIO);
		if (!is_audio_source) continue;

		const char *name = obs_source_get_name(source);
		if (m_name->array) {
			if (!dstr_cmp(m_name, name)) {
				src = source;
				break;
			}
		}
		obs_source_release(source);
	}

	if (!src && m_name->array)
		src = obs_get_source_by_name(m_name->array);

	if (!src) return;

	monitor->source = src;

	sh = obs_source_get_signal_handler(src);
	signal_handler_connect(sh, "audio_data",
		avis_monitor_data_received_signal, monitor);
	signal_handler_connect(sh, "remove",
		avis_monitor_removed_signal, monitor);
}

void avis_monitor_release_obs_source(avis_monitor_t *monitor)
{
	if (!monitor) return;
	if (!monitor->source) return;

	signal_handler_t *sh;

	sh = obs_source_get_signal_handler(monitor->source);

	pthread_mutex_lock(&monitor->data_mutex);

	signal_handler_disconnect(sh, "audio_data",
		avis_monitor_data_received_signal, monitor);
	signal_handler_disconnect(sh, "remove",
		avis_monitor_removed_signal, monitor);

	pthread_mutex_unlock(&monitor->data_mutex);

	obs_source_release(monitor->source);
	monitor->source = NULL;
}


static void avis_monitor_data_received_signal(void *vptr,
	calldata_t *calldata)
{
	uint32_t channels;
	size_t frames, offset, window_size;
	avis_monitor_t *monitor = vptr;

	if (!monitor) return;

	struct audio_data *data = calldata_ptr(calldata, "data");

	if (!data) return;

	if (pthread_mutex_trylock(&monitor->data_mutex)) return;

	avis_buffer_t *m_data = monitor->data;
	window_size = m_data->size;
	channels = m_data->channels;

	frames = data->frames;

	if (frames > window_size)
		goto BAD_REALLY_BAD;

	offset = window_size - frames;
	m_data->volume = data->volume;

	for (uint32_t i = 0; i < channels; i++) {
		float *abuffer = m_data->buffers[i];
		float *adata = (float*)data->data[i];
		if (adata) {
			memmove(abuffer,
				abuffer + frames,
				offset * sizeof(float));

			memcpy(abuffer + offset,
				adata,
				frames * sizeof(float));
		}
	}

BAD_REALLY_BAD:
	pthread_mutex_unlock(&monitor->data_mutex);
}

static void avis_monitor_removed_signal(void *vptr, calldata_t *calldata)
{
	avis_monitor_t *monitor = vptr;
	
	if (!monitor) return;

	avis_monitor_release_obs_source(monitor);
	UNUSED_PARAMETER(calldata);
}
