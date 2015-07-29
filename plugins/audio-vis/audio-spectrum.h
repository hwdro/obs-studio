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

#include <obs.h>
#include "audio-fft.h"

enum AUDIO_WEIGHTING_TYPES {
	AUDIO_WEIGHTING_TYPE_Z,
	AUDIO_WEIGHTING_TYPE_A,
	AUDIO_WEIGHTING_TYPE_C
};


struct audio_spectrum {
	float          *spectrum_data;
	uint32_t       *bins_indexes;
	float          *weights;
	uint32_t       bins;
};

typedef struct audio_spectrum audio_spectrum_t;
audio_spectrum_t * audio_spectrum_create(uint32_t sample_rate, size_t size,
	int oct_den, enum AUDIO_WEIGHTING_TYPES wg_type);
void audio_spectrum_destroy(audio_spectrum_t *as);
void as_process_fft_data(audio_spectrum_t *context,
	audio_fft_t *fft_context, float frame_time);
uint32_t audio_spectrum_get_bins(audio_spectrum_t *context);
uint32_t audio_spectrum_get_bands(audio_spectrum_t *context);
float * audio_spectrum_get_spectrum_data(audio_spectrum_t *context);
