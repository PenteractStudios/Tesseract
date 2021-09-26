#pragma once

#include "Resource.h"

#include <vector>

class ResourceAudioClip : public Resource {
public:
	REGISTER_RESOURCE(ResourceAudioClip, ResourceType::AUDIO);

	void Load() override;
	void Unload() override;

public:
	unsigned int ALbuffer = 0;
};