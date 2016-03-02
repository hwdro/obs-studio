#include <obs-module.h>

OBS_DECLARE_MODULE()

OBS_MODULE_USE_DEFAULT_LOCALE("audio-visualizer", "en-US")

extern struct obs_source_info spectrum_source;


bool obs_module_load(void)
{
	obs_register_source(&spectrum_source);
	//obs_register_source(&fade_transition);
	return true;
}
