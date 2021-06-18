#pragma once

#include "Resource.h"

#include "FileSystem/JsonValue.h"
class ResourceSkybox : public Resource {
public:
	REGISTER_RESOURCE(ResourceSkybox, ResourceType::SKYBOX);

	void Load() override;
	void Unload() override;

	unsigned GetGlCubeMap() {
		return glCubeMap;
	}

	unsigned GetGlIrradianceMap() {
		return glIrradianceMap;
	}

private:
	unsigned glCubeMap = 0;
	unsigned glIrradianceMap = 0;
};
