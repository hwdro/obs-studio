#include <obs-module.h>

OBS_DECLARE_MODULE()

OBS_MODULE_USE_DEFAULT_LOCALE("audio-vis-fr", "en-US")

extern struct obs_source_info avis_source_info;


bool obs_module_load(void)
{
	obs_register_source(&avis_source_info);
	return true;
}