#pragma once

#include "Globals.h"

struct PointLightUniforms {
	PointLightUniforms();
	PointLightUniforms(unsigned program, unsigned number);

	int posLocation = -1;
	int colorLocation = -1;
	int intensityLocation = -1;
	int kcLocation = -1;
	int klLocation = -1;
	int kqLocation = -1;
};

struct SpotLightUniforms {
	SpotLightUniforms();
	SpotLightUniforms(unsigned program, unsigned number);

	int posLocation = -1;
	int directionLocation = -1;
	int colorLocation = -1;
	int intensityLocation = -1;
	int kcLocation = -1;
	int klLocation = -1;
	int kqLocation = -1;
	int innerAngleLocation = -1;
	int outerAngleLocation = -1;
};

struct Program {
	Program(unsigned program);
	virtual ~Program();

	unsigned program = 0;
};

struct ProgramCubemapRender : Program {
	ProgramCubemapRender(unsigned program);

	int viewLocation = -1;
	int projLocation = -1;
};

struct ProgramHDRToCubemap : ProgramCubemapRender {
	ProgramHDRToCubemap(unsigned program);

	int hdrLocation = -1;
};

struct ProgramIrradiance : ProgramCubemapRender {
	ProgramIrradiance(unsigned program);

	int environmentLocation = -1;
};

struct ProgramPreFilteredMap : ProgramCubemapRender {
	ProgramPreFilteredMap(unsigned program);

	int environmentLocation = -1;
	int environmentResolutionLocation = -1;
	int roughnessLocation = -1;
};

struct ProgramEnvironmentBRDF : Program {
	ProgramEnvironmentBRDF(unsigned program);
};

struct ProgramSkybox : Program {
	ProgramSkybox(unsigned program);

	int viewLocation = -1;
	int projLocation = -1;

	int cubemapLocation = -1;
};

struct ProgramUnlit : public Program {
	ProgramUnlit(unsigned program);

	int modelLocation = -1;
	int viewLocation = -1;
	int projLocation = -1;

	int paletteLocation = -1;
	int hasBonesLocation = -1;

	int diffuseMapLocation = -1;
	int diffuseColorLocation = -1;
	int hasDiffuseMapLocation = -1;

	int emissiveMapLocation = -1;
	int hasEmissiveMapLocation = -1;

	int tilingLocation = -1;
	int offsetLocation = -1;
};

struct ProgramStandard : public Program {
	ProgramStandard(unsigned program);

	int modelLocation = -1;
	int viewLocation = -1;
	int projLocation = -1;

	int viewLightLocation = -1;
	int projLightLocation = -1;

	int paletteLocation = -1;
	int hasBonesLocation = -1;

	int viewPosLocation = -1;

	int diffuseMapLocation = -1;
	int diffuseColorLocation = -1;
	int hasDiffuseMapLocation = -1;
	int smoothnessLocation = -1;
	int hasSmoothnessAlphaLocation = -1;

	int normalMapLocation = -1;
	int hasNormalMapLocation = -1;
	int normalStrengthLocation = -1;

	int emissiveMapLocation = -1;
	int hasEmissiveMapLocation = -1;

	int ambientOcclusionMapLocation = -1;
	int hasAmbientOcclusionMapLocation = -1;

	int depthMapTextureLocation = -1;

	int ssaoTextureLocation = -1;
	int ssaoDirectLightingStrengthLocation = -1;

	int tilingLocation = -1;
	int offsetLocation = -1;

	int diffuseIBLLocation = -1;
	int prefilteredIBLLocation = -1;
	int environmentBRDFLocation = -1;
	int prefilteredIBLNumLevelsLocation = -1;

	int lightAmbientColorLocation = -1;

	int lightDirectionalDirectionLocation = -1;
	int lightDirectionalColorLocation = -1;
	int lightDirectionalIntensityLocation = -1;
	int lightDirectionalIsActiveLocation = -1;

	PointLightUniforms lightPoints[POINT_LIGHTS];
	int lightNumPointsLocation = -1;

	SpotLightUniforms lightSpots[POINT_LIGHTS];
	int lightNumSpotsLocation = -1;
};

struct ProgramStandardPhong : ProgramStandard {
	ProgramStandardPhong(unsigned program);

	int specularColorLocation = -1;
	int hasSpecularMapLocation = -1;
	int specularMapLocation = -1;
};

struct ProgramStandardSpecular : ProgramStandard {
	ProgramStandardSpecular(unsigned program);

	int specularColorLocation = -1;
	int hasSpecularMapLocation = -1;
	int specularMapLocation = -1;
};

struct ProgramStandardMetallic : ProgramStandard {
	ProgramStandardMetallic(unsigned program);

	int metalnessLocation = -1;
	int hasMetallicMapLocation = -1;
	int metallicMapLocation = -1;
};

struct ProgramDepthPrepass : Program {
	ProgramDepthPrepass(unsigned program);

	int modelLocation = -1;
	int viewLocation = -1;
	int projLocation = -1;

	int diffuseMapLocation = -1;
	int diffuseColorLocation = -1;
	int hasDiffuseMapLocation = -1;

	int paletteLocation = -1;
	int hasBonesLocation = -1;

	int tilingLocation = -1;
	int offsetLocation = -1;
};

struct ProgramDepthPrepassConvertTextures : Program {
	ProgramDepthPrepassConvertTextures(unsigned program);

	int samplesNumberLocation = -1;

	int positionsLocation = -1;
	int normalsLocation = -1;
};

struct ProgramSSAO : Program {
	ProgramSSAO(unsigned program);

	int projLocation = -1;

	int positionsLocation = -1;
	int normalsLocation = -1;

	int kernelSamplesLocation = -1;
	int randomTangentsLocation = -1;
	int screenSizeLocation = -1;
	int biasLocation = -1;
	int rangeLocation = -1;
	int powerLocation = -1;
};

struct ProgramSSAOBlur : Program {
	ProgramSSAOBlur(unsigned program);

	int inputTextureLocation = -1;

	int kernelLocation = -1;
	int horizontalLocation = -1;
};

struct ProgramColorCorrection : Program {
	ProgramColorCorrection(unsigned program);

	int textureSceneLocation = -1;
	int textureBloomBlurLocation = -1;
};

struct ProgramPostprocess : Program {
	ProgramPostprocess(unsigned program);

	int textureSceneLocation = -1;
	int bloomThresholdLocation = -1;
	int samplesNumberLocation = -1;
};

struct ProgramDrawTexture : Program {
	ProgramDrawTexture(unsigned program);

	int textureToDrawLocation = -1;
};

struct ProgramImageUI : Program {
	ProgramImageUI(unsigned program);

	int modelLocation = -1;
	int viewLocation = -1;
	int projLocation = -1;

	int inputColorLocation = -1;
	int hasDiffuseLocation = -1;
	int diffuseLocation = -1;
};

struct ProgramTextUI : Program {
	ProgramTextUI(unsigned program);

	int modelLocation = -1;
	int viewLocation = -1;
	int projLocation = -1;

	int textColorLocation = -1;
};

struct ProgramBillboard : Program {
	ProgramBillboard(unsigned program);

	int modelLocation = -1;
	int viewLocation = -1;
	int projLocation = -1;

	int inputColorLocation = -1;
	int hasDiffuseLocation = -1;
	int diffuseMapLocation = -1;

	int currentFrameLocation = -1;
	int xTilesLocation = -1;
	int yTilesLocation = -1;
	int xFlipLocation = -1;
	int yFlipLocation = -1;
};
struct ProgramTrail : Program {
	ProgramTrail(unsigned program);

	int viewLocation = -1;
	int projLocation = -1;

	int inputColorLocation = -1;
	int hasDiffuseLocation = -1;
	int diffuseMap = -1;
};