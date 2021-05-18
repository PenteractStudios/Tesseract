#pragma once

#include "Resource.h"

enum class TextureMinFilter {
	NEAREST,
	LINEAR,
	NEAREST_MIPMAP_NEAREST,
	LINEAR_MIPMAP_NEAREST,
	NEAREST_MIPMAP_LINEAR,
	LINEAR_MIPMAP_LINEAR
};

enum class TextureMagFilter {
	NEAREST,
	LINEAR
};

enum class TextureWrap {
	REPEAT,
	CLAMP_TO_EDGE,
	CLAMP_TO_BORDER,
	MIRROR_REPEAT,
	MIRROR_CLAMP_TO_EDGE
};

class ResourceTexture : public Resource {
public:
	REGISTER_RESOURCE(ResourceTexture, ResourceType::TEXTURE);

	void Load() override;
	void Unload() override;
	void OnEditorUpdate() override;

	void UpdateMinFilter(TextureMinFilter filter);
	void UpdateMagFilter(TextureMagFilter filter);
	void UpdateWrap(TextureWrap wrap);
	void Apply();

public:
	unsigned int glTexture = 0;

	bool hasChanged = false;

	TextureWrap wrap = TextureWrap::REPEAT;
	TextureMinFilter minFilter = TextureMinFilter::NEAREST_MIPMAP_LINEAR;
	TextureMagFilter magFilter = TextureMagFilter::LINEAR;
};