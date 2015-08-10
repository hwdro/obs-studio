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

audio_fft_t * audio_fft_create(uint32_t sample_rate, uint32_t channels,
	size_t size)
{
	audio_fft_t *context = bzalloc(sizeof(audio_fft_t));

	if (!context) return NULL;

	int nbits = (int)log2((double)size);

	context->rdft_context = av_rdft_init(nbits, DFT_R2C);
	context->window_func = bzalloc(size * sizeof(float));
	context->audio = hwa_buffer_create(sample_rate, channels, size);
	window_func_init(context->window_func, size, AUDIO_WINDOW_TYPE_HANNING);

	return context;
}

void audio_fft_process(audio_fft_t *context)
{
	if (!context) return;
	if (!context->audio) return;
	if (!context->rdft_context) return;

	hwa_buffer_t *audio = context->audio;
	uint32_t ch = audio->channels;
	size_t ws = audio->size;

	for (uint32_t i = 0; i < ch; i++) {
		if (audio->buffers[i])
			av_rdft_calc(context->rdft_context, audio->buffers[i]);
	}

	__m128 wsize = _mm_set_ps1((float)ws * 0.5f);

	for (uint32_t i = 0; i < ch; i++) {
		float *buffer = audio->buffers[i];
		if (!buffer) continue;

		for (uint32_t j = 0; j < ws; j += 4) {
			_mm_store_ps(buffer + j,
				_mm_div_ps(_mm_load_ps(buffer + j), wsize));
		}
	}
}

void audio_fft_audio_in(audio_fft_t *context, hwa_buffer_t *audio_in)
{
	if (!audio_in) return;
	if (!context) return;
	if (!context->audio) return;

	hwa_buffer_t *audio = context->audio;
	uint32_t ch = audio->channels;
	size_t ws = audio->size;

	__m128 vol = _mm_set1_ps(audio_in->volume);

	for (uint32_t i = 0; i < ch; i++) {
		float *dst = context->audio->buffers[i];
		float *src = audio_in->buffers[i];
		float *wf  = context->window_func;
		
		if (!src || !dst || !wf) continue;

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

void audio_fft_destroy(audio_fft_t *context)
{
	if (!context) return;

	hwa_buffer_destroy(context->audio);
	av_rdft_end(context->rdft_context);
	bfree(context->window_func);
	bfree(context);
}
