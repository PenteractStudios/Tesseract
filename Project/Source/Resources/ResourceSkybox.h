#pragma once

#include "Resource.h"

#include "FileSystem/JsonValue.h"
class ResourceSkybox : public Resource {
public:
	REGISTER_RESOURCE(ResourceSkybox, ResourceType::SKYBOX);

	void Load() override;
	void Unload() override;

	unsigned GetVAO() {
		return vao;
	};

	unsigned GetGlCubeMap() {
		return glCubeMap;
	}

private:
	unsigned vao = 0;
	unsigned vbo = 0;
	unsigned glCubeMap = 0;
};
