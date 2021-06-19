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

	unsigned GetGlPreFilteredMap() {
		return glPreFilteredMap;
	}

	unsigned GetGlEnvironmentBRDF() {
		return glEnvironmentBRDF;
	}

	int GetPreFilteredMapNumLevels() {
		return preFilteredMapNumLevels;
	}

private:
	unsigned glCubeMap = 0;
	unsigned glIrradianceMap = 0;
	unsigned glPreFilteredMap = 0;
	unsigned glEnvironmentBRDF = 0;

	int preFilteredMapNumLevels = 0;
};
