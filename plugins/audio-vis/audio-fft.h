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
#include <libavcodec/avfft.h>
#include "avis-buffer.h"

enum AUDIO_WINDOW_TYPES {
	AUDIO_WINDOW_TYPE_RECTANGULAR,
	AUDIO_WINDOW_TYPE_HANNING,
	AUDIO_WINDOW_TYPE_HAMMING,
	AUDIO_WINDOW_TYPE_BLACKMAN,
	AUDIO_WINDOW_TYPE_NUTTALL,
	AUDIO_WINDOW_TYPE_BLACKMAN_NUTTALL,
	AUDIO_WINDOW_TYPE_BLACKMAN_HARRIS
};

struct audio_fft {
	avis_buffer_t *audio;
	RDFTContext  *rdft_context;
	float        *window_func;
};

typedef struct audio_fft audio_fft_t;

audio_fft_t * audio_fft_create(uint32_t sample_rate, uint32_t channels,
	size_t size);
void audio_fft_process(audio_fft_t *context);
void audio_fft_audio_in(audio_fft_t *context, avis_buffer_t *audio_in);
void audio_fft_destroy(audio_fft_t *context);
