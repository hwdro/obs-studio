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
#include <string>
#include <util\threading.h>

#include "audiodata.hpp"

using namespace std;


class AudioSource {
	string          sourceName;
	obs_source_t    *audioSource;

	uint32_t        sampleRate;
	size_t          windowSize;

	AudioData       *audioData;
	pthread_mutex_t mutex;
public:
	int MutexTryLock();
	int MutexLock();
	int MutexUnlock();
	AudioSource(const string &sourceName_, uint32_t sampleRate_,
		size_t windowSize_);
	~AudioSource();
	void Update(const string &sourceName_, uint32_t sampleRate_,
		size_t windowSize_);
	void Tick(float seconds);
	obs_source_t * GetAudioSource();
	static void SourceDataRecievedSignal(void *obj, calldata_t *calldata);
	static void SourceRemovedSignal(void *obj, calldata_t *calldata);
	void AcquireAudioSource();
	void ReleaseAudioSource();
};