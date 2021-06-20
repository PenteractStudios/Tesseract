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

	// Skybox shader
	unsigned skybox = 0;

	// Ilumination Shaders
	ProgramStandardPhong* phongNormal = nullptr;
	ProgramStandardPhong* phongNotNormal = nullptr;
	ProgramStandardMetallic* standardNormal = nullptr;
	ProgramStandardMetallic* standardNotNormal = nullptr;
	ProgramStandardSpecular* specularNormal = nullptr;
	ProgramStandardSpecular* specularNotNormal = nullptr;

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
