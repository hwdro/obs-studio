#include "avis-capture.h"


static void avis_capture_start(avis_capture_t *ac);
static void avis_capture_stop(avis_capture_t *ac);

avis_capture_t * avis_capture_create(const char *source_name, uint32_t channels,
	uint32_t frame_size)
{
	avis_capture_t *ac = bzalloc(sizeof(avis_capture_t));
	
	dstr_init_copy(&ac->source_name, source_name);
	
	avis_capture_start(ac);

	return ac;
}

void avis_capture_destroy(avis_capture_t *ac)
{
	if (!ac) return;
	avis_capture_stop(ac);
	dstr_free(&ac->source_name);
	bfree(ac);
}

static inline bool is_audio_source(obs_source_t *source)
{
	return !!(obs_source_get_output_flags(source) & OBS_SOURCE_AUDIO);
}

static void source_removed_cb(void *data, calldata_t *calldata)
{
	UNUSED_PARAMETER(calldata);
	
	avis_capture_t *ac = data;
	if (!ac) return;
	avis_capture_stop(ac);
}

static void avis_capture_start(avis_capture_t *ac)
{
	if (!ac) return;
	if (ac->source) return;

	const char *source_name = ac->source_name.array;
	obs_source_t *source = NULL;

	if (source_name)
		source = obs_get_source_by_name(source_name);
	if (!source)
		return;
	if (!is_audio_source(source))
		obs_source_release(source);

	ac->source = source;
	signal_handler_t *sh = obs_source_get_signal_handler(source);
	signal_handler_connect(sh, "remove", source_removed_cb, ac);
}

static void avis_capture_stop(avis_capture_t *ac)
{

	if (!ac || !ac->source) return;

	signal_handler_t *sh = obs_source_get_signal_handler(ac->source);

	signal_handler_disconnect(sh, "remove", source_removed_cb, ac);
	obs_source_release(ac->source);
	ac->source = NULL;
}
