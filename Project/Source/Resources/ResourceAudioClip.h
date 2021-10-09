#pragma once

#include "Resource.h"

#include <vector>

enum class AudioFormat {
	WAV,
	OGG
};

class ResourceAudioClip : public Resource {
public:
	REGISTER_RESOURCE(ResourceAudioClip, ResourceType::AUDIO);

	void Load() override;
	void Unload() override;

	void LoadResourceMeta(JsonValue jResourceMeta) override;
	void SaveResourceMeta(JsonValue jResourceMeta) override;

public:
	unsigned int ALbuffer = 0;

	bool isMono = false;
	AudioFormat audioFormat = AudioFormat::WAV;
};