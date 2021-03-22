#pragma once

#include "Resource.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "Resources/ResourceShader.h"
#include "FileSystem/JsonValue.h"

#include <vector>
#include <string>

class ResourceSkybox : public Resource {
public:
	ResourceSkybox(UID id, const char* assetFilePath, const char* resourceFilePath);

	void Load();
	void Unload() override;
	void Draw();
	void ReadPath();

	ResourceShader* GetShader() {
		return shader;
	};
	unsigned int GetVao() {
		return skyboxVAO;
	};
	unsigned int GetVbo() {
		return skyboxVBO;
	};
	unsigned int GetGlCubeMap() {
		return glCubeMap;
	}
	// clang-format off
	float skyboxVertices[108] = {
		// Front (x, y, z)
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		// Left (x, y, z)
		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		// Right (x, y, z)
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		// Back (x, y, z)
		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		// Top (x, y, z)
		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		// Bottom (x, y, z)
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	}; // clang-format on

private:
	ResourceShader* shader;
	unsigned int skyboxVAO = 0;
	unsigned int skyboxVBO = 0;
	unsigned int glCubeMap = 0;
};
