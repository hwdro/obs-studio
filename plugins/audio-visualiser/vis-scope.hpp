#pragma once
#include <obs-module.h>
#include "audiodata.hpp"

class Visual{
protected:
	uint32_t cx;
	uint32_t cy;
	gs_texture *texture;
	uint8_t  *frameBuffer;
public:
	virtual uint32_t GetWidth()
	{
		return cx;
	}
	virtual uint32_t GetHeight()
	{
		return cy;
	}

	virtual void Update(AudioData *data) = 0;
	virtual void Tick(float seconds) = 0;
	virtual void Render(gs_effect_t *effect) = 0;

	Visual(uint32_t cx_, uint32_t cy_)
		: cx(cx_),
		  cy(cy_),
		  texture(nullptr),
		  frameBuffer(nullptr)
	{

	}
	virtual ~Visual() {};
};

class VisScope: public Visual{
	AudioData *audioData;
public:
	VisScope(uint32_t cx_, uint32_t cy_, uint32_t sampleRate_,
		uint32_t channels_, size_t size_);
	~VisScope();

	void Update(AudioData *data);
	void Tick(float seconds);
	void Render(gs_effect_t *effect);
};