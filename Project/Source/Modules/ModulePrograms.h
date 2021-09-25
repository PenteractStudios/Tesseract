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
	unsigned CreateComputeProgram(const char* shaderFile, const char* computeSnippets = "compute");
	void DeleteProgram(unsigned int idProgram);

public:
	const char* filePath = "Library/shadersBin";

	// Skybox shaders
	ProgramHDRToCubemap* hdrToCubemap = nullptr;
	ProgramIrradiance* irradiance = nullptr;
	ProgramPreFilteredMap* preFilteredMap = nullptr;
	ProgramEnvironmentBRDF* environmentBRDF = nullptr;
	ProgramSkybox* skybox = nullptr;

	// Light culling shaders
	ProgramGridFrustumsCompute* gridFrustumsCompute = nullptr;
	ProgramLightCullingCompute* lightCullingCompute = nullptr;

	// Unlit Shader
	ProgramUnlit* unlit = nullptr;

	// Volumetric light Shader
	ProgramVolumetricLight* volumetricLight = nullptr;

	// Ilumination Shaders
	ProgramStandardPhong* phongNormal = nullptr;
	ProgramStandardPhong* phongNotNormal = nullptr;
	ProgramStandardMetallic* standardNormal = nullptr;
	ProgramStandardMetallic* standardNotNormal = nullptr;
	ProgramStandardSpecular* specularNormal = nullptr;
	ProgramStandardSpecular* specularNotNormal = nullptr;

	// Dissolve Shaders
	ProgramStandardDissolve* dissolveStandard = nullptr;
	ProgramUnlitDissolve* dissolveUnlit = nullptr;

	// Depth prepass Shaders
	ProgramDepthPrepass* depthPrepass = nullptr;
	ProgramDepthPrepassConvertTextures* depthPrepassConvertTextures = nullptr;
	ProgramDepthPrepassDissolve* depthPrepassDissolve = nullptr;

	// SSAO Shaders
	ProgramSSAO* ssao = nullptr;
	ProgramBlur* blur = nullptr;

	// Bloom shaders
	ProgramBloomCombine* bloomCombine = nullptr;

	// Post-processing Shaders
	ProgramPostprocess* postprocess = nullptr;
	ProgramColorCorrection* colorCorrection = nullptr;

	// Fog Shaders
	ProgramHeightFog* heightFog = nullptr;

	// Shadow Shaders
	unsigned shadowMap = 0;

	// Engine Shaders
	ProgramDrawTexture* drawTexture = nullptr;
	ProgramDrawLightTiles* drawLightTiles = nullptr;

	// UI Shaders
	ProgramTextUI* textUI = nullptr;
	ProgramImageUI* imageUI = nullptr;

	// Particle Shaders
	ProgramBillboard* billboard = nullptr;
	ProgramTrail* trail = nullptr;
};
