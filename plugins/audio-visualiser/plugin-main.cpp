#include <obs-module.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("audio-visualiser", "en-US")

void RegisterAudioVisualiserSource();

bool obs_module_load(void)
{
	RegisterAudioVisualiserSource();
	return true;
}