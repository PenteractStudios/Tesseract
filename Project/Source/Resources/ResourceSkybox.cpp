#include "ResourceSkybox.h"

#include "Globals.h"
#include "Application.h"
#include "Modules/ModuleFiles.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleCamera.h"
#include "Modules/ModulePrograms.h"
#include "Modules/ModuleRender.h"
#include "Utils/MSTimer.h"
#include "Utils/Logging.h"
#include "Utils/Buffer.h"
#include "Utils/Leaks.h"

#include "IL/il.h"
#include "IL/ilu.h"
#include "GL/glew.h"

#define CUBEMAP_RESOLUTION 512
#define IRRADIANCE_RESOLUTION 128

static void RenderToCubemap(unsigned cubemap, int resolution, unsigned program) {
	// Frustum setup
	const float3 front[6] = {float3::unitX, -float3::unitX, float3::unitY, -float3::unitY, float3::unitZ, -float3::unitZ};
	const float3 up[6] = {-float3::unitY, -float3::unitY, float3::unitZ, -float3::unitZ, -float3::unitY, -float3::unitY};
	Frustum frustum;
	frustum.SetKind(FrustumSpaceGL, FrustumRightHanded);

	// Render setup
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, resolution, resolution);
	glUseProgram(program);

	// Create framebuffer
	unsigned captureFBO = 0;
	glCreateFramebuffers(1, &captureFBO);
	DEFER {
		glDeleteFramebuffers(1, &captureFBO);
	};
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	// Set projection matrix
	frustum.SetPerspective(math::pi / 2.0f, math::pi / 2.0f);
	frustum.SetViewPlaneDistances(0.1f, 100.0f);
	float4x4 proj = frustum.ProjectionMatrix();
	glUniformMatrix4fv(glGetUniformLocation(program, "proj"), 1, GL_TRUE, proj.ptr());

	// Bind Unit Cube VAO
	glBindVertexArray(App->renderer->cubeVAO);
	DEFER {
		glBindVertexArray(0);
	};

	// Render cube 6 times (once looking at each face)
	for (unsigned i = 0; i < 6; ++i) {
		// Bind face
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap, 0);

		// Set view matrix
		frustum.SetFrame(float3::zero, front[i], up[i]);
		glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_TRUE, frustum.ViewMatrix().ptr());

		// Draw cube
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}

void ResourceSkybox::Load() {
	std::string filePath = GetResourceFilePath();
	LOG("Loading skybox from path: \"%s\".", filePath.c_str());

	// Timer to measure loading a skybox
	MSTimer timer;
	timer.Start();

	// Generate image handler
	unsigned image;
	ilGenImages(1, &image);
	DEFER {
		ilDeleteImages(1, &image);
	};

	// Load image
	ilBindImage(image);
	bool imageLoaded = ilLoad(IL_HDR, filePath.c_str());
	if (!imageLoaded) {
		LOG("Failed to load image.");
		return;
	}

	// Convert image
	bool imageConverted = ilConvertImage(IL_RGB, IL_FLOAT);
	if (!imageConverted) {
		LOG("Failed to convert image.");
		return;
	}

	// Flip image if neccessary
	ILinfo info;
	iluGetImageInfo(&info);
	if (info.Origin == IL_ORIGIN_UPPER_LEFT) {
		iluFlipImage();
	}

	// Create HDR texture
	unsigned hdrTexture = 0;
	glGenTextures(1, &hdrTexture);
	DEFER {
		glDeleteTextures(1, &hdrTexture);
	};

	// Load HDR texture from image
	glBindTexture(GL_TEXTURE_2D, hdrTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGB, GL_FLOAT, ilGetData());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create cubemap texture
	glGenTextures(1, &glCubeMap);

	// Set texture parameters
	glBindTexture(GL_TEXTURE_CUBE_MAP, glCubeMap);
	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, CUBEMAP_RESOLUTION, CUBEMAP_RESOLUTION, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// Render cubemap from HDR image
	unsigned program = App->programs->hdrToCubemap;
	glUseProgram(program);

	glUniform1i(glGetUniformLocation(program, "hdr"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	RenderToCubemap(glCubeMap, CUBEMAP_RESOLUTION, program);

	// Create irradiance texture
	glGenTextures(1, &glIrradianceMap);

	// Set texture parameters
	glBindTexture(GL_TEXTURE_CUBE_MAP, glIrradianceMap);
	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, IRRADIANCE_RESOLUTION, IRRADIANCE_RESOLUTION, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// Render Irradiance cubemap
	program = App->programs->irradiance;
	glUseProgram(program);

	glUniform1i(glGetUniformLocation(program, "environment"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, glCubeMap);

	RenderToCubemap(glIrradianceMap, IRRADIANCE_RESOLUTION, program);

	unsigned timeMs = timer.Stop();
	LOG("Skybox loaded in %ums.", timeMs);
}

void ResourceSkybox::Unload() {
	if (glCubeMap) glDeleteTextures(1, &glCubeMap);
	if (glIrradianceMap) glDeleteTextures(1, &glIrradianceMap);
}