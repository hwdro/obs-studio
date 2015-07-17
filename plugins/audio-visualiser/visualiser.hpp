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

#pragma once

#include <obs-module.h>
#include "audiosource.hpp"
class AudioVisSource;

class AudioVisSource {
	obs_source_t *source;
	AudioSource  *audioSource;
	uint32_t     cx;
	uint32_t     cy;
public:
	AudioVisSource(obs_data_t *settings, obs_source_t *source_);
	inline ~AudioVisSource();
	uint32_t GetWidth();
	uint32_t GetHeight();
	static bool EnumAudioSourcesCallback(void *param,
		obs_source_t *source);
	void Update(obs_data_t *settings);
	obs_properties_t *Properties();
	void Tick(float seconds);
};
