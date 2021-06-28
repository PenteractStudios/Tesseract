#pragma once

#include "Module.h"
#include "Rendering/Programs.h"

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
	ProgramHDRToCubemap* hdrToCubemap = nullptr;
	ProgramIrradiance* irradiance = nullptr;
	ProgramPreFilteredMap* preFilteredMap = nullptr;
	ProgramEnvironmentBRDF* environmentBRDF = nullptr;
	ProgramSkybox* skybox = nullptr;

	// Ilumination Shaders
	ProgramStandardPhong* phongNormal = nullptr;
	ProgramStandardPhong* phongNotNormal = nullptr;
	ProgramStandardMetallic* standardNormal = nullptr;
	ProgramStandardMetallic* standardNotNormal = nullptr;
	ProgramStandardSpecular* specularNormal = nullptr;
	ProgramStandardSpecular* specularNotNormal = nullptr;

	// Depth prepass Shaders
	ProgramDepthPrepass* depthPrepass = nullptr;

	// SSAO Shaders
	ProgramSSAO* ssao = nullptr;
	ProgramSSAOBlur* ssaoBlur = 0;

	// Shadow Shaders
	unsigned shadowMap = 0;

	// Engine Shaders
	ProgramDrawTexture* drawTexture = 0;

	// UI Shaders
	ProgramTextUI* textUI = 0;
	ProgramImageUI* imageUI = 0;

	// Particle Shaders
	ProgramBillboard* billboard = 0;
	unsigned trail = 0;
};
