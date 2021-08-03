#pragma once

#include "Resource.h"
#include "Components/ComponentAudioSource.h"
#include <vector>

class ResourceAudioClip : public Resource {
public:
	REGISTER_RESOURCE(ResourceAudioClip, ResourceType::AUDIO);

	void Load() override;
	void Unload() override;

	void AddSource(ComponentAudioSource* component);
	void RemoveSource(ComponentAudioSource* component);

public:
	unsigned int ALbuffer = 0;
	std::vector<ComponentAudioSource*> componentAudioSources;
};