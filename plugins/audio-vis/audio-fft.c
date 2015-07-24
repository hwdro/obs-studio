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


#include "audio-fft.h"

static void window_func_init(float *buffer, size_t size,
	enum AUDIO_WINDOW_TYPES window_type)
{
	float a0, a1, a2, a3, a4;

	a0 = a1 = a2 = a3 = a4 = 0.0f;

	if (window_type == AUDIO_WINDOW_TYPE_RECTANGULAR) {
		for (int i = 0; i < size; i++)
			buffer[i] = 1.0f;
		return;
	}

	switch ((int)window_type) {
	case AUDIO_WINDOW_TYPE_HAMMING:
		a0 = 0.53836f;
		a1 = 0.46164f;
		break;
	case AUDIO_WINDOW_TYPE_BLACKMAN:
		a0 = 0.42659f;
		a1 = 0.49656f;
		a2 = 0.076849f;
		break;
	case AUDIO_WINDOW_TYPE_NUTTALL:
		a0 = 0.355768f;
		a1 = 0.487396f;
		a2 = 0.144232f;
		a3 = 0.012604f;
		break;
	case AUDIO_WINDOW_TYPE_BLACKMAN_NUTTALL:
		a0 = 0.3635819f;
		a1 = 0.4891775f;
		a2 = 0.1365995f;
		a3 = 0.0106411f;
		break;
	case AUDIO_WINDOW_TYPE_BLACKMAN_HARRIS:
		a0 = 0.35875f;
		a1 = 0.48829f;
		a2 = 0.14128f;
		a3 = 0.01168f;
		break;
	case AUDIO_WINDOW_TYPE_HANNING:
	default:
		a0 = a1 = 0.5f;
	}

	for (size_t i = 0; i < size; i++) {
		buffer[i] =
			(
			a0
			-
			a1 * cosf((2.0f * M_PI * i) / (size - 1))
			+
			a2 * cosf((4.0f * M_PI * i) / (size - 1))
			-
			a3 * cosf((6.0f * M_PI * i) / (size - 1))
			+
			a4 * cosf((8.0f * M_PI * i) / (size - 1))
			);
	}
}

static float a_weighting(float frequency)
{
	float ra;
	float  f = frequency;
	float f2 = f * f;
	float n1 = 12200.0f * 12200.0f;
	float n2 = 20.6f * 20.6f;

	ra = (n1 * f2 * f2);
	ra /= (f2 + n2) * (f2 + n1) *
		sqrtf(f2 + 107.7f * 107.7f)
		*
		sqrtf(f2 + 737.9f * 737.9f);
	ra = 2.0f + 20.0f * log10f(ra);
	if (fabs(ra) < 0.002f)
		ra = 0.0f;
	return ra;
}

static float c_weighting(float frequency)
{
	float ra;
	float  f = frequency;
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

static int calc_octave_bins(uint32_t *bins, float *weights, uint32_t sample_rate,
	size_t size, int oct_den, enum AUDIO_WEIGHTING_TYPES weighting_type)
{

	float up_freq, low_freq;
	uint32_t bin_l, bin_u;

	float   center = 31.62777f;
	int      bands = 0;
	float max_freq = sample_rate / 2.0f;

	if (max_freq > 16000.0f)
		max_freq = 16000.0f;

	while (center < max_freq)
	{
		up_freq = center * powf(10,
			3.0f / (10.0f * 2 * (float)oct_den));
		low_freq = center / powf(10.0f,
			3.0f / (10 * 2 * (float)oct_den));

		bin_l = frequency_to_bin(low_freq, sample_rate, size);
		bin_u = frequency_to_bin(up_freq, sample_rate, size);

		if (bin_u > size / 2)
			bin_u = (uint32_t)size / 2 - 1;

		if (center < max_freq && bin_u != bin_l) {
			if (bins) {
				bins[bands] = bin_l;
				switch ((int)weighting_type) {
				case AUDIO_WEIGHTING_TYPE_Z:
					weights[bands] = 0.0f;
					break;
				case AUDIO_WEIGHTING_TYPE_A:
					weights[bands] = a_weighting(center);
					break;
				case AUDIO_WEIGHTING_TYPE_C:
					weights[bands] = c_weighting(center);
				}
			}
			bands++;
		}
		center = center * powf(10, 3 / (10 * (float)oct_den));
	}

	if (bin_u < size / 2 - 1) {
		if (bins)
			bins[bands] = bin_u;
		bands++;
	}

	return bands;
}



audio_fft_t * audio_fft_create(uint32_t sample_rate, uint32_t channels,
	size_t size, int oct_den, enum AUDIO_WEIGHTING_TYPES wg_type)
{
	audio_fft_t *context = bzalloc(sizeof(audio_fft_t));
	
	int nbits = (int)log2((double)size);
	context->rdft_context = av_rdft_init(nbits, DFT_R2C);
	
	context->audio = audio_buffer_create(sample_rate, channels, size);
	
	context->window_func = bzalloc(size * sizeof(float));
	window_func_init(context->window_func, size, AUDIO_WINDOW_TYPE_HANNING);

	int bins = calc_octave_bins(NULL, NULL, sample_rate, size,
		oct_den, wg_type);
	
	context->bins         = bins;
	context->bins_indexes = bzalloc((bins + 1) * sizeof(uint32_t));
	context->weights      = bzalloc((bins) * sizeof(float));
	context->spectrum     = bzalloc((bins) * sizeof(float));
	calc_octave_bins(context->bins_indexes, context->weights,
		sample_rate, size, oct_den, wg_type);
	return context;
}

void audio_fft_process(audio_fft_t *context)
{
	if (!context) return;
	if (!context->audio) return;
	if (!context->rdft_context) return;

	audio_buffer_t *audio = context->audio;
	uint32_t           ch = audio->channels;
	size_t             ws = audio->size;

	for (uint32_t i = 0; i < ch; i++) {
		if (audio->buffer[i]) {
			av_rdft_calc(context->rdft_context,
				audio->buffer[i]);
		}
	}

	__m128 wsize = _mm_set_ps1((float)ws * 0.5f);

	for (uint32_t i = 0; i < ch; i++) {
		float *buffer = audio->buffer[i];
		for (uint32_t j = 0; j < ws; j += 4) {
			_mm_store_ps(buffer + j,
				_mm_div_ps(_mm_load_ps(buffer + j), wsize));
		}
	}

}

void audio_fft_new_data(audio_fft_t *context, audio_buffer_t *audio_in)
{
	
	if (!context) return;
	if (!context->audio) return;

	audio_buffer_t *audio = context->audio;
	uint32_t           ch = audio->channels;
	size_t             ws = audio->size;

	__m128 vol = _mm_set1_ps(audio_in->volume);

	for (uint32_t i = 0; i < ch; i++) {
		float *dst = context->audio->buffer[i];
		float *src = audio_in->buffer[i];
		float *wf  = context->window_func;
		for (float *d = dst, *s = src;
			d < dst + ws;
			d += 4, s += 4, wf += 4) {
			__m128 wc = _mm_load_ps(wf);
			_mm_store_ps(
				d,
				_mm_mul_ps(
				_mm_load_ps(s), _mm_mul_ps(wc, vol))
				);
		}
	}
}

#define DB_MIN -70.0f
void audio_fft_build_spectrum(audio_fft_t *context, float frame_time)
{
	if (!context) return;
	if (!context->audio) return;

	audio_buffer_t *audio = context->audio;
	uint32_t           ch = audio->channels;
	size_t             ws = audio->size;
	uint32_t bins         = context->bins;
	uint32_t  *bi         = context->bins_indexes;

	for (uint32_t b = 0; b < bins; b++) {
		uint32_t start = bi[b] + 1;
		uint32_t  stop = bi[b + 1] + 1;
		float mag = 0;
		for (uint32_t nch = 0; nch < ch; nch++) {
			float *buffer = context->audio->buffer[nch];
			float bmag = 0;

			if (!buffer)
				break;

			for (uint32_t o = start; o < stop; o++) {
				float re, im;
				re = buffer[o * 2];
				im = buffer[o * 2 + 1];
				bmag += re * re + im * im;
			}
			mag += bmag / (stop - start);
		}

		mag /= (float)ch;
		mag = 10.0f * log10f(mag);

		mag += context->weights[b];

		if (mag < DB_MIN)
			mag = DB_MIN;

		mag = (DB_MIN - mag) / DB_MIN;

		if (context->spectrum[b] > mag) {
			context->spectrum[b] -=
				(context->spectrum[b] - mag) *
				frame_time * 7.5f;
		}
		else {
			context->spectrum[b] +=
				(mag - context->spectrum[b]) *
				frame_time * 15.0f;
		}
	}

}

void audio_fft_destroy(audio_fft_t *context)
{
	audio_buffer_destroy(context->audio);
	av_rdft_end(context->rdft_context);
	bfree(context->window_func);
	bfree(context->bins_indexes);
	bfree(context->weights);
	bfree(context->spectrum);
	bfree(context);
}
