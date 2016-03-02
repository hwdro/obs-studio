#include "audiocapture.h"

void audiocapture_start(audiocapture_t *ac);
void audiocapture_stop(audiocapture_t *ac);

void audio_source_removed(void *vptr, calldata_t *calldata);
void audio_source_data_callback(void *vptr, obs_source_t *source,
	const struct audio_data *data, bool muted);


audiocapture_t *audiocapture_create(const char *src_name, uint32_t channels,
			    size_t size)
{
	audiocapture_t *ac = bzalloc(sizeof(audiocapture_t));

	audiobuf_init(&ac->audio_buffers, channels, size);
	dstr_init_copy(&ac->audio_source_name, src_name);
	pthread_mutex_init(&ac->data_mutex, NULL);

	audiocapture_start(ac);

	return ac;
}

void audiocapture_destroy(audiocapture_t *ac)
{
	if (!ac) return;
	audiocapture_stop(ac);
	
	audiobuf_free(&ac->audio_buffers);
	dstr_free(&ac->audio_source_name);
	
	pthread_mutex_destroy(&ac->data_mutex);

	bfree(ac);
}

bool audiocapture_is_active(audiocapture_t *ac)
{
	if (!ac) return false;
	
	if(!ac->audio_source)
		audiocapture_start(ac);
	
	if (ac->audio_source) return true;
	
	return false;
}

void audiocapture_copy_buffers(audiocapture_t *ac, audiobuf_t *dst)
{
	if (!dst && !ac && !ac->audio_source) return;

	if (dst->frame_size != ac->audio_buffers.frame_size) return;
	if (dst->channels != ac->audio_buffers.channels) return;
	
	if (os_atomic_load_bool(&ac->data_consumed)) return;

	if (pthread_mutex_trylock(&ac->data_mutex)) return;
	
	for (size_t i = 0; i < dst->channels; i++)
		memcpy(dst->sbuffers[i]->data,
		       ac->audio_buffers.sbuffers[i]->data,
		       dst->frame_size * sizeof(float));

	os_atomic_set_bool(&ac->data_consumed, true);

	pthread_mutex_unlock(&ac->data_mutex);
}


static obs_source_t *find_audio_source_by_name(struct dstr *audio_source_name)
{
	obs_source_t *audio_source = NULL;

	for (uint32_t i = 1; i <= 10; i++) {
		obs_source_t *source = obs_get_output_source(i);

		if (!source)
			continue;

		if (!(obs_source_get_output_flags(source) & OBS_SOURCE_AUDIO))
			continue;

		const char *name = obs_source_get_name(source);

		if (audio_source_name->array)
			if (!dstr_cmp(audio_source_name, name)) {
				audio_source = source;
				break;
			}

		obs_source_release(source);
	}

	if (!audio_source && audio_source_name->array)
		audio_source = obs_get_source_by_name(audio_source_name->array);

	return audio_source;

}

static void audiocapture_start(audiocapture_t *ac)
{
	if (!ac) return;
	if (ac->audio_source) return;

	obs_source_t *audio_source;
	
	audio_source = find_audio_source_by_name(&ac->audio_source_name);

	if (!audio_source) return;

	ac->audio_source     = audio_source;
	signal_handler_t *sh = obs_source_get_signal_handler(audio_source);

	obs_source_add_audio_capture_callback(audio_source,
		audio_source_data_callback, ac);

	signal_handler_connect(sh, "remove", audio_source_removed, ac);
}

static void audiocapture_stop(audiocapture_t *ac)
{
	if (!ac) return;
	if (!ac->audio_source) return;

	signal_handler_t *sh = obs_source_get_signal_handler(ac->audio_source);

	pthread_mutex_lock(&ac->data_mutex);
	{
		obs_source_remove_audio_capture_callback(
			ac->audio_source, audio_source_data_callback, ac);
		signal_handler_disconnect(sh, "remove", audio_source_removed,
					  ac);
	}
	pthread_mutex_unlock(&ac->data_mutex);

	obs_source_release(ac->audio_source);

	ac->audio_source = NULL;
}

static void audio_source_data_callback(void *vptr, obs_source_t *source,
				       const struct audio_data *data,
				       bool muted)
{
	audiocapture_t *ac = (audiocapture_t *)vptr;
	audiobuf_t *ab     = &ac->audio_buffers;
	size_t channels    = ab->channels;

	if (ab->frame_size < data->frames) return;

	ab->volume = obs_source_get_volume(source);
	ab->muted  = muted;

	if (!pthread_mutex_trylock(&ac->data_mutex)) return;

	for (int i = 0; i < channels; i++)
		audiobuf_push_frame(ab, i, (float *)data->data[i],
				    data->frames);

	os_atomic_set_bool(&ac->data_consumed, false);

	pthread_mutex_unlock(&ac->data_mutex);
}

static void audio_source_removed(void *vptr, calldata_t *calldata)
{
	audiocapture_t *ac = (audiocapture_t *)vptr;

	if (!ac) return;

	audiocapture_stop(ac);

	UNUSED_PARAMETER(calldata);
}
