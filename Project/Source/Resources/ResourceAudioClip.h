#pragma once

#include "Resource.h"
#include "Components/ComponentAudioSource.h"

#include <vector>
#include <sndfile.h>

class ResourceAudioClip : public Resource {
public:
	REGISTER_RESOURCE(ResourceAudioClip, ResourceType::AUDIO);

	void Load() override;
	void FinishLoading() override;
	void Unload() override;

	void AddSource(ComponentAudioSource* component);
	void RemoveSource(ComponentAudioSource* component);

public:
	unsigned int alBuffer = 0;
	std::vector<ComponentAudioSource*> componentAudioSources;

private:
	bool validAudio = false;

	SF_INFO sfInfo;
	int format = 0;
	short* audioData = nullptr;
	unsigned size = 0;
};