#include "Programs.h"

#include "GL/glew.h"
#include <string>

PointLightUniforms::PointLightUniforms() {}

PointLightUniforms::PointLightUniforms(unsigned program, unsigned number) {
	posLocation = glGetUniformLocation(program, (std::string("light.points[") + std::to_string(number) + "].pos").c_str());
	colorLocation = glGetUniformLocation(program, (std::string("light.points[") + std::to_string(number) + "].color").c_str());
	intensityLocation = glGetUniformLocation(program, (std::string("light.points[") + std::to_string(number) + "].intensity").c_str());
	kcLocation = glGetUniformLocation(program, (std::string("light.points[") + std::to_string(number) + "].kc").c_str());
	klLocation = glGetUniformLocation(program, (std::string("light.points[") + std::to_string(number) + "].kl").c_str());
	kqLocation = glGetUniformLocation(program, (std::string("light.points[") + std::to_string(number) + "].kq").c_str());
}

SpotLightUniforms::SpotLightUniforms() {}

SpotLightUniforms::SpotLightUniforms(unsigned program, unsigned number) {
	posLocation = glGetUniformLocation(program, (std::string("light.spots[") + std::to_string(number) + "].pos").c_str());
	directionLocation = glGetUniformLocation(program, (std::string("light.spots[") + std::to_string(number) + "].direction").c_str());
	colorLocation = glGetUniformLocation(program, (std::string("light.spots[") + std::to_string(number) + "].color").c_str());
	intensityLocation = glGetUniformLocation(program, (std::string("light.spots[") + std::to_string(number) + "].intensity").c_str());
	kcLocation = glGetUniformLocation(program, (std::string("light.spots[") + std::to_string(number) + "].kc").c_str());
	klLocation = glGetUniformLocation(program, (std::string("light.spots[") + std::to_string(number) + "].kl").c_str());
	kqLocation = glGetUniformLocation(program, (std::string("light.spots[") + std::to_string(number) + "].kq").c_str());
	innerAngleLocation = glGetUniformLocation(program, (std::string("light.spots[") + std::to_string(number) + "].innerAngle").c_str());
	outerAngleLocation = glGetUniformLocation(program, (std::string("light.spots[") + std::to_string(number) + "].outerAngle").c_str());
}

Program::Program(unsigned program_)
	: program(program_) {}

Program::~Program() {
	glDeleteProgram(program);
}

ProgramStandard::ProgramStandard(unsigned program_)
	: Program(program_) {
	modelLocation = glGetUniformLocation(program, "model");
	viewLocation = glGetUniformLocation(program, "view");
	projLocation = glGetUniformLocation(program, "proj");

	viewLightLocation = glGetUniformLocation(program, "viewLight");
	projLightLocation = glGetUniformLocation(program, "projLight");

	paletteLocation = glGetUniformLocation(program, "palette");
	hasBonesLocation = glGetUniformLocation(program, "hasBones");

	viewPosLocation = glGetUniformLocation(program, "viewPos");

	diffuseMapLocation = glGetUniformLocation(program, "diffuseMap");
	diffuseColorLocation = glGetUniformLocation(program, "diffuseColor");
	hasDiffuseMapLocation = glGetUniformLocation(program, "hasDiffuseMap");
	smoothnessLocation = glGetUniformLocation(program, "smoothness");
	hasSmoothnessAlphaLocation = glGetUniformLocation(program, "hasSmoothnessAlpha");

	normalMapLocation = glGetUniformLocation(program, "normalMap");
	hasNormalMapLocation = glGetUniformLocation(program, "hasNormalMap");
	normalStrengthLocation = glGetUniformLocation(program, "normalStrength");

	emissiveMapLocation = glGetUniformLocation(program, "emissiveMap");
	hasEmissiveMapLocation = glGetUniformLocation(program, "hasEmissiveMap");

	ambientOcclusionMapLocation = glGetUniformLocation(program, "ambientOcclusion");
	hasAmbientOcclusionMapLocation = glGetUniformLocation(program, "hasAmbientOcclusion");

	depthMapTextureLocation = glGetUniformLocation(program, "depthMapTexture");

	ssaoTextureLocation = glGetUniformLocation(program, "ssaoTexture");

	tilingLocation = glGetUniformLocation(program, "tiling");
	offsetLocation = glGetUniformLocation(program, "offset");

	lightAmbientColorLocation = glGetUniformLocation(program, "light.ambient.color");

	lightDirectionalDirectionLocation = glGetUniformLocation(program, "light.directional.direction");
	lightDirectionalColorLocation = glGetUniformLocation(program, "light.directional.color");
	lightDirectionalIntensityLocation = glGetUniformLocation(program, "light.directional.intensity");
	lightDirectionalIsActiveLocation = glGetUniformLocation(program, "light.directional.isActive");

	lightNumPointsLocation = glGetUniformLocation(program, "light.numPoints");
	for (unsigned i = 0; i < POINT_LIGHTS; ++i) {
		lightPoints[i] = PointLightUniforms(program, i);
	}

	lightNumSpotsLocation = glGetUniformLocation(program, "light.numSpots");
	for (unsigned i = 0; i < SPOT_LIGHTS; ++i) {
		lightSpots[i] = SpotLightUniforms(program, i);
	}
}

ProgramStandardPhong::ProgramStandardPhong(unsigned program_)
	: ProgramStandard(program_) {
	specularColorLocation = glGetUniformLocation(program, "specularColor");
	hasSpecularMapLocation = glGetUniformLocation(program, "hasSpecularMap");
	specularMapLocation = glGetUniformLocation(program, "specularMap");
}

ProgramStandardSpecular::ProgramStandardSpecular(unsigned program_)
	: ProgramStandard(program_) {
	specularColorLocation = glGetUniformLocation(program, "specularColor");
	hasSpecularMapLocation = glGetUniformLocation(program, "hasSpecularMap");
	specularMapLocation = glGetUniformLocation(program, "specularMap");
}

ProgramStandardMetallic::ProgramStandardMetallic(unsigned program_)
	: ProgramStandard(program_) {
	metalnessLocation = glGetUniformLocation(program, "metalness");
	hasMetallicMapLocation = glGetUniformLocation(program, "hasMetallicMap");
	metallicMapLocation = glGetUniformLocation(program, "metallicMap");
}

ProgramDepthPrepass::ProgramDepthPrepass(unsigned program_)
	: Program(program_) {
	modelLocation = glGetUniformLocation(program, "model");
	viewLocation = glGetUniformLocation(program, "view");
	projLocation = glGetUniformLocation(program, "proj");

	paletteLocation = glGetUniformLocation(program, "palette");
	hasBonesLocation = glGetUniformLocation(program, "hasBones");
}

ProgramSSAO::ProgramSSAO(unsigned program_)
	: Program(program_) {
	projLocation = glGetUniformLocation(program, "proj");

	positionsLocation = glGetUniformLocation(program, "positions");
	normalsLocation = glGetUniformLocation(program, "normals");

	kernelSamplesLocation = glGetUniformLocation(program, "kernelSamples");
	randomTangentsLocation = glGetUniformLocation(program, "randomTangents");
	screenSizeLocation = glGetUniformLocation(program, "screenSize");
	biasLocation = glGetUniformLocation(program, "bias");
	rangeLocation = glGetUniformLocation(program, "range");
	powerLocation = glGetUniformLocation(program, "power");
}

ProgramSSAOBlur::ProgramSSAOBlur(unsigned program_)
	: Program(program_) {
	inputTextureLocation = glGetUniformLocation(program, "inputTexture");

	kernelLocation = glGetUniformLocation(program, "kernel");
	horizontalLocation = glGetUniformLocation(program, "horizontal");
}

ProgramDrawTexture::ProgramDrawTexture(unsigned program_)
	: Program(program_) {
	textureToDrawLocation = glGetUniformLocation(program, "textureToDraw");
}

ProgramImageUI::ProgramImageUI(unsigned program_)
	: Program(program_) {
	modelLocation = glGetUniformLocation(program, "model");
	viewLocation = glGetUniformLocation(program, "view");
	projLocation = glGetUniformLocation(program, "proj");

	inputColorLocation = glGetUniformLocation(program, "inputColor");
	hasDiffuseLocation = glGetUniformLocation(program, "hasDiffuse");
	diffuseLocation = glGetUniformLocation(program, "diffuse");
}

ProgramTextUI::ProgramTextUI(unsigned program_)
	: Program(program_) {
	modelLocation = glGetUniformLocation(program, "model");
	viewLocation = glGetUniformLocation(program, "view");
	projLocation = glGetUniformLocation(program, "proj");

	textColorLocation = glGetUniformLocation(program, "textColor");
}
