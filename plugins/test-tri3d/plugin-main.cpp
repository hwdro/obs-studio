#include <obs-module.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("test-tri3d", "en-US")

void RegisterSource();

bool obs_module_load(void)
{
	RegisterSource();
	return true;
}