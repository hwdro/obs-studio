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

#include <obs-module.h>
#include <util/dstr.h>
#include <util/darray.h>
#include <emmintrin.h>
#include "vis-spectrum.h"
#include "audio-fft.h"
#include "audio-spectrum.h"

#define SPECTRUM_WINDOW_SIZE "visual.spectrum.windowsize"
#define SPECTRUM_WINDOW_SIZE_DESC \
		obs_module_text("visual.spectrum.windowsize.desc")

#define SPECTRUM_OCTAVE_FRACT "visual.spectrum.octavefract"
#define SPECTRUM_OCTAVE_FRACT_DESC \
		obs_module_text("visual.spectrum.octavefract.desc")

#define SPECTRUM_WG_TYPE "visual.spectrum.wgtype"
#define SPECTRUM_WG_TYPE_DESC obs_module_text("visual.spectrum.wgtype.desc")

#define SPECTRUM_BG_COLOR "visual.spectrum.bgcolor"
#define SPECTRUM_FG_COLOR "visual.spectrum.fgcolor"

#define SPECTRUM_BG_COLOR_DESC obs_module_text("visual.spectrum.bgcolor.desc")
#define SPECTRUM_FG_COLOR_DESC obs_module_text("visual.spectrum.fgcolor.desc")

struct sp_bar
{
	int b_x;
	int b_y;
	int width;
};
typedef struct sp_bar sp_bar_t;

struct spectrum_visual
{
	uint32_t         cx;
	uint32_t         cy;

	gs_texture_t     *texture;
	uint8_t          *framebuffer;

	audio_fft_t      *fft;
	audio_spectrum_t *spectrum;
	
	float            frame_time;
	uint32_t         fg_color;
	uint32_t         bg_color;

	DARRAY(sp_bar_t) bars;
};
typedef struct spectrum_visual spectrum_visual_t;

static void spectrum_bars_init(spectrum_visual_t *context);
static void spectrum_bars_destroy(spectrum_visual_t *context);
static void spectrum_bars_draw(spectrum_visual_t *context);

static void * spectrum_create(obs_data_t *settings, uint32_t sample_rate,
	uint32_t channels)
{
	spectrum_visual_t *context = bzalloc(sizeof(spectrum_visual_t));
	
	uint32_t nbits = (uint32_t)obs_data_get_int(settings,
		SPECTRUM_WINDOW_SIZE);
	
	uint32_t oct_fract = (uint32_t)obs_data_get_int(settings,
		SPECTRUM_OCTAVE_FRACT);

	uint32_t wg_type = (uint32_t)obs_data_get_int(settings,
		SPECTRUM_WG_TYPE);
	
	uint32_t size = 2 << nbits;

	context->fg_color = (uint32_t)obs_data_get_int(settings,
		SPECTRUM_FG_COLOR);
	context->bg_color = (uint32_t)obs_data_get_int(settings,
		SPECTRUM_BG_COLOR);


	context->cx = 640;
	context->cy = 360;
	

	context->framebuffer = bzalloc(context->cx * context->cy * 4);
	
	obs_enter_graphics();
	context->texture = gs_texture_create(context->cx, context->cy,
		GS_RGBA, 1, NULL, GS_DYNAMIC);
	obs_leave_graphics();

	context->fft = audio_fft_create(sample_rate, channels, size);
	context->spectrum = audio_spectrum_create(sample_rate, size,
		oct_fract, wg_type);

	spectrum_bars_init(context);
	
	return context;
}

static void spectrum_destroy(void *data)
{
	spectrum_visual_t *context = (spectrum_visual_t *)data;
	
	if (!context)
		return;
	
	audio_fft_destroy(context->fft);
	audio_spectrum_destroy(context->spectrum);
	spectrum_bars_destroy(context);

	bfree(context->framebuffer);

	obs_enter_graphics();
	gs_texture_destroy(context->texture);
	obs_leave_graphics();

	bfree(context);
}

static void spectrum_render(void *data, gs_effect_t *effect)
{
	spectrum_visual_t *context = (spectrum_visual_t *)data;
	
	if (!context) return;

	gs_reset_blend_state();

	gs_effect_set_texture(gs_effect_get_param_by_name(effect, "image"),
		context->texture);

	gs_draw_sprite(context->texture, 0, context->cx, context->cy);
}

static void spectrum_tick(void *data, float seconds)
{
	spectrum_visual_t *context = (spectrum_visual_t *)data;
	
	context->frame_time = seconds;
	
	spectrum_bars_draw(context);
}

static void spectrum_process_audio(void *data, avis_buffer_t *audio)
{
	spectrum_visual_t *context = (spectrum_visual_t *)data;
	
	if (!context) return;
	if (!context->fft) return;

	audio_fft_audio_in(context->fft, audio);
}

#define MIN_FFT_BITS 8
#define MAX_FFT_BITS 11

void spectrum_get_defaults(obs_data_t *settings)
{
	obs_data_set_default_int(settings, SPECTRUM_WINDOW_SIZE, MAX_FFT_BITS);
	obs_data_set_default_int(settings, SPECTRUM_OCTAVE_FRACT, 12);
	obs_data_set_default_int(settings, SPECTRUM_FG_COLOR, 0xFF00FFFF);
	obs_data_set_default_int(settings, SPECTRUM_BG_COLOR, 0x00000000);
	obs_data_set_default_int(settings, SPECTRUM_WG_TYPE,
		AUDIO_WEIGHTING_TYPE_Z);
}

#define SP_ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

static void spectrum_get_properties(void *data, obs_properties_t *props)
{
	spectrum_visual_t     *context = (spectrum_visual_t *)data;
	obs_property_t   *prop;

	prop = obs_properties_add_list(
		props,
		SPECTRUM_WINDOW_SIZE,
		SPECTRUM_WINDOW_SIZE_DESC,
		OBS_COMBO_TYPE_LIST,
		OBS_COMBO_FORMAT_INT);

	struct dstr str = { 0 };

	for (int i = MIN_FFT_BITS; i < MAX_FFT_BITS + 1; i++) {
		dstr_printf(&str, "%d", 2 << i);
		obs_property_list_add_int(prop, str.array, i);
	}

	prop = obs_properties_add_list(
		props,
		SPECTRUM_OCTAVE_FRACT,
		SPECTRUM_OCTAVE_FRACT_DESC,
		OBS_COMBO_TYPE_LIST,
		OBS_COMBO_FORMAT_INT);

	int ofd[] = { 1, 3, 6, 12 };

	for (int i = 0; i < SP_ARRAY_LEN(ofd); i++) {
		dstr_printf(&str, "1 / %d", ofd[i]);
		obs_property_list_add_int(prop, str.array, ofd[i]);
	}

	dstr_free(&str);

	const char *wgtype[] = { "Z (Flat)", "A" };

	prop = obs_properties_add_list(
		props,
		SPECTRUM_WG_TYPE,
		SPECTRUM_WG_TYPE_DESC,
		OBS_COMBO_TYPE_LIST,
		OBS_COMBO_FORMAT_INT);

	for (int i = 0; i < 2; i++)
		obs_property_list_add_int(prop, wgtype[i], i);

	obs_properties_add_color(props, SPECTRUM_FG_COLOR,
		SPECTRUM_FG_COLOR_DESC);
	obs_properties_add_color(props, SPECTRUM_BG_COLOR,
		SPECTRUM_BG_COLOR_DESC);
}

static uint32_t spectrum_get_width(void *data)
{
	spectrum_visual_t *context = (spectrum_visual_t *)data;
	return context->cx;
}

static uint32_t spectrum_get_height(void *data)
{
	spectrum_visual_t *context = (spectrum_visual_t *)data;
	return context->cy;
}

static size_t spectrum_frame_size(void *data)
{
	spectrum_visual_t *context = (spectrum_visual_t *)data;
	return context->fft->audio->size;
}


typedef struct gs_rect sp_rect_t;

static inline void sp_memset32(uint32_t *pixels, uint32_t val, size_t size)
{
	uint32_t remaining = (uint32_t)size & 3;
	uint32_t aligned = (uint32_t)size - remaining;
	__m128i *start = (__m128i *)pixels;
	__m128i pval = _mm_set1_epi32(val);

	for (uint32_t i = 0; i < aligned / 4; i++)
		_mm_store_si128(start + i, pval);

	for (uint32_t i = aligned; i < size; i++)
		*(pixels + i) = val;
}

static inline void sp_rect_set(sp_rect_t *r, int x, int y, int cx, int cy)
{
	if (!r) return;

	r->x = x;
	r->y = y;
	r->cx = cx;
	r->cy = cy;
}

static inline bool sp_rect_intersection(sp_rect_t *a, sp_rect_t *b,
	sp_rect_t *result)
{
	int x = a->x > b->x ? a->x : b->x;
	int y = a->y > b->y ? a->y : b->y;
	int nx = (a->x + a->cx) < (b->x + b->cx) ?
		(a->x + a->cx) : (b->x + b->cx);
	int ny = (a->y + a->cy) < (b->y + b->cy) ?
		(a->y + a->cy) : (b->y + b->cy);
	if (nx >= x && ny >= y)
	{
		sp_rect_set(result, x, y, nx - x, ny - y);
		return true;
	} 

	return false;
};

static inline void sp_draw_bar(uint32_t *pixels, sp_rect_t *bar,
	sp_rect_t *area, uint32_t color)
{
	sp_rect_t res;
	int x, y, w, h;

	if (!sp_rect_intersection(bar, area, &res)) return;

	x = res.x;
	y = res.y;
	w = res.cx;
	h = res.cy;

	for (int i = y; i < y + h; i++)
		for (int j = x; j < x + w; j++)
			pixels[i*area->cx + j] = color;
}


static void spectrum_bars_draw(spectrum_visual_t *context)
{
	if (!context) return;
	if (!context->texture) return;

	uint32_t cx = context->cx;
	uint32_t cy = context->cy;
	
	uint32_t *framebuffer = (uint32_t *)context->framebuffer;
	
	audio_fft_process(context->fft);
	audio_spectrum_process_fft_data(context->spectrum, context->fft, 
		context->frame_time);

	sp_memset32(framebuffer, context->bg_color, cx * cy);
	
	uint32_t bands = audio_spectrum_get_bands(context->spectrum);
	float *sp_data = audio_spectrum_get_spectrum_data(context->spectrum);

	sp_rect_t view = { 0, 0, cx, cy }, bar;
	sp_bar_t *bars = context->bars.array;


	for (uint32_t b = 0; b < bands; b++) {
		int by , bcy;
		bcy = (int)(bars[b].b_y * sp_data[b]);
		by = bars[b].b_y - bcy;

		sp_rect_set(&bar, bars[b].b_x, by, bars[b].width, bcy);
		sp_draw_bar(framebuffer, &bar, &view, context->fg_color);
	}

	obs_enter_graphics();
	gs_texture_set_image(context->texture, context->framebuffer, cx * 4,
		false);
	obs_leave_graphics();
}

static void spectrum_bars_init(spectrum_visual_t *context)
{
	uint32_t bands = audio_spectrum_get_bands(context->spectrum);
	uint32_t cx = context->cx;
	uint32_t cy = context->cy;

	da_init(context->bars);

	int fbw = cx / bands;
	int pad = (cx - bands * fbw) / 2;
	int bpad = (int)(fbw * 0.2f);
	int bw = fbw - bpad;
	
	sp_bar_t bar;

	for (uint32_t b = 0; b < bands; b++) {
		bar.b_x = fbw * b;
		bar.b_y  = cy;
		bar.width = bw;
		da_push_back(context->bars, &bar);
	}
}

static void spectrum_bars_destroy(spectrum_visual_t *context)
{
	da_free(context->bars);
}

static visual_t spectrum_visual = {
	.create         = spectrum_create,
	.destroy        = spectrum_destroy,
	.render         = spectrum_render,
	.tick           = spectrum_tick,
	.process_audio  = spectrum_process_audio,
	.get_defaults   = spectrum_get_defaults,
	.get_properties = spectrum_get_properties,
	.get_width      = spectrum_get_width,
	.get_height     = spectrum_get_height,
	.frame_size     = spectrum_frame_size
};


visual_t * get_spectrum_visualisation(void)
{
	return &spectrum_visual;
}
