#include "ResourceSkybox.h"

#include "Globals.h"
#include "Application.h"
#include "Modules/ModuleFiles.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleCamera.h"
#include "Modules/ModulePrograms.h"
#include "Utils/MSTimer.h"
#include "Utils/Logging.h"
#include "Utils/Buffer.h"
#include "Utils/Leaks.h"

#include "IL/il.h"
#include "IL/ilu.h"
#include "GL/glew.h"

#define CUBEMAP_RESOLUTION 512

// clang-format off
static const float skyboxVertices[108] = {
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

	bool imageConverted = ilConvertImage(IL_RGB, IL_FLOAT);
	if (!imageConverted) {
		LOG("Failed to convert image.");
		return;
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

	// Create VAO
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// Load VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

	// Load vertex attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);

	glBindVertexArray(0);

	// Render cubemap from HDR image
	unsigned program = App->programs->hdrToCubemap;
	glUseProgram(program);

	unsigned captureFBO = 0;
	glCreateFramebuffers(1, &captureFBO);
	DEFER {
		glDeleteFramebuffers(1, &captureFBO);
	};

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glDisable(GL_DEPTH_TEST);

	glUniform1i(glGetUniformLocation(program, "hdri"), 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	glViewport(0, 0, CUBEMAP_RESOLUTION, CUBEMAP_RESOLUTION);
	for (unsigned i = 0; i < 6; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, glCubeMap, 0);

		glUniform1i(glGetUniformLocation(program, "face"), i);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	unsigned timeMs = timer.Stop();
	LOG("Skybox loaded in %ums.", timeMs);
}

void ResourceSkybox::Unload() {
	if (glCubeMap) glDeleteTextures(1, &glCubeMap);
	if (vao) glDeleteVertexArrays(1, &vao);
	if (vbo) glDeleteBuffers(1, &vbo);
}