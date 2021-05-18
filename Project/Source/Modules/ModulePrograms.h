#pragma once

#include "Module.h"

class ModulePrograms : public Module {
public:
	bool Start() override;
	bool CleanUp() override;

	void LoadShaderBinFile();
	unsigned CreateProgram(const char* shaderFile, const char* vertexSnippets = "vertex", const char* fragmentSnippets = "fragment");
	void DeleteProgram(unsigned int idProgram);

public:
	const char* filePath = "Library/shadersBin";

	// Skybox shader
	unsigned skybox = 0;

	// Ilumination Shaders
	unsigned phongNormal = 0;
	unsigned phongNotNormal = 0;
	unsigned standardNormal = 0;
	unsigned standardNotNormal = 0;
	unsigned specularNormal = 0;
	unsigned specularNotNormal = 0;

	//UI Shaders
	unsigned textUI = 0;
	unsigned imageUI = 0;
};
