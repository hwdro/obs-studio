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

#include "audio-spectrum.h"

static float a_weighting(float frequency)
{
	float ra;
	float f = frequency;
	float f2 = f * f;
	float n1 = 12200.0f * 12200.0f;
	float n2 = 20.6f * 20.6f;

	ra = (n1 * f2 * f2);
	ra /= (f2 + n2) * (f2 + n1) *
		sqrtf(f2 + 107.7f * 107.7f) * sqrtf(f2 + 737.9f * 737.9f);
	ra = 2.0f + 20.0f * log10f(ra);
	
	if (fabs(ra) < 0.002f)
		ra = 0.0f;
	
	return ra;
}

static float c_weighting(float frequency)
{
	float ra;
	float f = frequency;
	float f2 = f * f;
	float n1 = 12200.0f * 12200.0f;
	float n2 = 20.6f * 20.6f;

	ra = (n1 * f2);
	ra /= (f2 + n2) * (f2 + n1);
	ra = 0.06f + 20.0f * log10f(ra);

	if (fabs(ra) < 0.002f)
		ra = 0.0f;
	
	return ra;
}

static float get_bandwidth(uint32_t sampleRate, size_t timeSize)
{
	return (float)sampleRate / timeSize;
}

static uint32_t frequency_to_bin(float freq, uint32_t sample_rate, size_t size)
{
	return (uint32_t)(freq / get_bandwidth(sample_rate, size));
}

static int calc_octave_bins(uint32_t *bins, float *weights,
	uint32_t sample_rate, size_t size, int oct_den,
	enum AUDIO_WEIGHTING_TYPES weighting_type)
{

	float up_freq, low_freq;
	uint32_t bin_l, bin_u;

	float center = 31.62777f;
	int bins_nr = 0;
	float max_freq = sample_rate / 2.0f;

	if (max_freq > 16000.0f)
		max_freq = 16000.0f;

	while (center < max_freq)
	{
		up_freq = center * powf(10.0f,
			3.0f / (10.0f * 2.0f * (float)oct_den));
		low_freq = center / powf(10.0f,
			3.0f / (10.0f * 2.0f * (float)oct_den));

		bin_l = frequency_to_bin(low_freq, sample_rate, size);
		bin_u = frequency_to_bin(up_freq, sample_rate, size);

		if (bin_u > size / 2)
			bin_u = (uint32_t)size / 2 - 1;

		if (center < max_freq && bin_u != bin_l) {
			if (bins && weights) {
				bins[bins_nr] = bin_l;
				switch ((int)weighting_type) {
				case AUDIO_WEIGHTING_TYPE_Z:
					weights[bins_nr] = 0.0f;
					break;
				case AUDIO_WEIGHTING_TYPE_A:
					weights[bins_nr] = a_weighting(center);
					break;
				case AUDIO_WEIGHTING_TYPE_C:
					weights[bins_nr] = c_weighting(center);
				}
			}
			bins_nr++;
		}
		center = center * powf(10.0f, 3.0f / (10.0f * (float)oct_den));
	}

	if (bin_u < size / 2 - 1) {
		if (bins)
			bins[bins_nr] = bin_u;
		bins_nr++;
	}

	return bins_nr;
}



audio_spectrum_t * audio_spectrum_create(uint32_t sample_rate, size_t size,
	int oct_den, enum AUDIO_WEIGHTING_TYPES wg_type)
{
	audio_spectrum_t *context = bzalloc(sizeof(audio_spectrum_t));
	
	int bins = calc_octave_bins(NULL, NULL, sample_rate, size,
		oct_den, wg_type);

	context->bins = bins;
	context->bins_indexes = bzalloc((bins + 1) * sizeof(uint32_t));
	context->weights = bzalloc((bins)* sizeof(float));
	context->spectrum_data = bzalloc((bins)* sizeof(float));

	calc_octave_bins(context->bins_indexes, context->weights,
		sample_rate, size, oct_den, wg_type);
	
	return context;
}
void audio_spectrum_destroy(audio_spectrum_t *context)
{
	bfree(context->spectrum_data);
	bfree(context->weights);
	bfree(context->bins_indexes);
	bfree(context);
}

#define DB_MIN -72.0f

void audio_spectrum_process_fft_data(audio_spectrum_t *context,
	audio_fft_t *fft_context, float frame_time)
{
	if (!context) return;
	if (!fft_context->audio) return;

	avis_buffer_t *audio = fft_context->audio;
	uint32_t *bi = context->bins_indexes;
	uint32_t ch = audio->channels;
	uint32_t bins = context->bins;
	size_t ws = audio->size;

	for (uint32_t b = 0; b < bins; b++) {
		uint32_t start = bi[b] + 1;
		uint32_t stop = bi[b + 1] + 1;
		float mag = 0;
		for (uint32_t nch = 0; nch < ch; nch++) {
			float *buffer = audio->buffers[nch];
			float bmag = 0;

			if (!buffer) continue;

			for (uint32_t o = start; o < stop; o++) {
				float re = buffer[o * 2];
				float im = buffer[o * 2 + 1];
				bmag += re * re + im * im;
			}
			mag += bmag / (stop - start);
		}

		mag /= (float)ch;

		if (mag > 0.0f) {
			mag = 10.0f * log10f(mag);
			mag += context->weights[b];
			if (mag < DB_MIN)
				mag = DB_MIN;
		} else {
			mag = DB_MIN;
		}

		mag = (DB_MIN - mag) / DB_MIN;

		if (context->spectrum_data[b] > mag) {
			context->spectrum_data[b] -=
				(context->spectrum_data[b] - mag) *
				frame_time * 7.5f;
		}
		else if (context->spectrum_data[b] < mag) {
			context->spectrum_data[b] +=
				(mag - context->spectrum_data[b]) *
				frame_time * 7.5f;
		}
	}
}

uint32_t audio_spectrum_get_bins(audio_spectrum_t *context)
{
	return context->bins;
}

uint32_t audio_spectrum_get_bands(audio_spectrum_t *context)
{
	return context->bins - 1;
}

float * audio_spectrum_get_spectrum_data(audio_spectrum_t *context)
{
	return context->spectrum_data;
}
