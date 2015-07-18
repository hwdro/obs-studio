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

#include "audiodata.hpp"

AudioData::AudioData(uint32_t sampleRate_, uint32_t channels_, size_t size_)
	: sampleRate(sampleRate_),
	  channels (channels_),
	  size     (size_)
{
	AllocBuffers();
}

AudioData::~AudioData()
{
	FreeBuffers();
}

void AudioData::AllocBuffers()
{
	buffers.clear();
	for (uint32_t i = 0; i < channels; i++)
		buffers.push_back(
		static_cast<float *>(bzalloc(size * sizeof(float)))
		);
}

void AudioData::FreeBuffers()
{
	for (auto b : buffers)
		bfree(b);

	buffers.clear();
}

size_t AudioData::GetSize()
{
	return size;
}

uint32_t AudioData::GetChannels()
{
	return channels;
}

const vector<float *>& AudioData::GetBuffers() const
{
	return buffers;
}

void AudioData::SetVolume(float volume_)
{
	volume = volume_;
}

float AudioData::GetVolume()
{
	return volume;
}

