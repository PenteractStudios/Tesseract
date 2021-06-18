#pragma once

#include "Module.h"

class ModulePrograms : public Module {
public:
	bool Start() override;
	bool CleanUp() override;

	void LoadShaderBinFile();
	void LoadShaders();
	void UnloadShaders();
	unsigned CreateProgram(const char* shaderFile, const char* vertexSnippets = "vertex", const char* fragmentSnippets = "fragment");
	void DeleteProgram(unsigned int idProgram);

public:
	const char* filePath = "Library/shadersBin";

	// Skybox shaders
	unsigned hdrToCubemap = 0;
	unsigned irradiance = 0;
	unsigned skybox = 0;

	// Ilumination Shaders
	unsigned phongNormal = 0;
	unsigned phongNotNormal = 0;
	unsigned standardNormal = 0;
	unsigned standardNotNormal = 0;
	unsigned specularNormal = 0;
	unsigned specularNotNormal = 0;

	// Depth prepass Shaders
	unsigned depthPrepass = 0;

	// SSAO Shaders
	unsigned ssao = 0;
	unsigned ssaoBlur = 0;

	// Shadow Shaders
	unsigned shadowMap = 0;

	// Engine Shaders
	unsigned drawSSAOTexture = 0;
	unsigned drawDepthMapTexture = 0;

	// UI Shaders
	unsigned textUI = 0;
	unsigned imageUI = 0;

	// Particle Shaders
	unsigned billboard = 0;
	unsigned trail = 0;
};
