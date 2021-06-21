#include "ModulePrograms.h"

#include "Globals.h"
#include "Application.h"
#include "Modules/ModuleFiles.h"
#include "Modules/ModuleTime.h"
#include "Utils/Logging.h"
#include "Utils/FileDialog.h"
#include "Utils/StringBlocksParser.h"

#include "GL/glew.h"
#include <string>

#include "Utils/Leaks.h"

static unsigned CompileShader(unsigned type, const char* filePath, const char* snippets) {
	LOG("Creating shader from snippets: \"%s\"...", snippets);

	parsb_options options;
	options.line_directives = false;
	parsb_context* blocks = parsb_create_context(options);
	DEFER {
		parsb_destroy_context(blocks);
	};
	parsb_add_blocks_from_file(blocks, filePath);

	// Add version
	parsb_add_block(blocks, "prefix", GLSL_VERSION "\n");
	std::string s = snippets;
	std::string finalSnippet = "prefix " + s;

	// Concatenate
	const char* shaderData = parsb_get_blocks(blocks, finalSnippet.c_str());
	if (shaderData == nullptr) {
		LOG("Error reading blocks %s from %s", finalSnippet.c_str(), filePath);
		return 0;
	}

	unsigned shaderId = glCreateShader(type);
	if (shaderId == 0) return 0;

	glShaderSource(shaderId, 1, &shaderData, 0);
	glCompileShader(shaderId);

	int res = GL_FALSE;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &res);

	if (res == GL_FALSE) {
		int len = 0;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &len);

		if (len > 1) {
			int written = 0;
			Buffer<char> info = Buffer<char>(len);
			glGetShaderInfoLog(shaderId, len, &written, info.Data());
			LOG("Log Info: %s", info.Data());
		}
		return 0;
	}

	LOG("Shader created successfully.");
	return shaderId;
}

void ModulePrograms::LoadShaderBinFile() {
	// Clean file on Start
	Buffer<char> cleanBuffer;
	if (!App->files->IsDirectory(LIBRARY_PATH)) {
		App->files->CreateFolder(LIBRARY_PATH);
	}
	App->files->Save(filePath, cleanBuffer);

	// Save all .shader from Assets/Shaders/ into one bin file
	std::vector<std::string> files = App->files->GetFilesInFolder("Assets/Shaders/");
	for (std::string file : files) {
		if (FileDialog::GetFileExtension(file.c_str()) == ".shader") {
			Buffer<char> buffer = App->files->Load(("Assets/Shaders/" + file).c_str());
			App->files->Save(filePath, buffer, true);
		}
	}
}

void ModulePrograms::LoadShaders() {
	MSTimer timer;
	timer.Start();

#if !GAME
	LoadShaderBinFile();
#endif

	// SkyBox shaders
	hdrToCubemap = CreateProgram(filePath, "vertCube", "fragFunctionIBL fragHDRToCubemap");
	irradiance = CreateProgram(filePath, "vertCube", "fragFunctionIBL fragIrradianceMap");
	preFilteredMap = CreateProgram(filePath, "vertCube", "fragFunctionIBL fragPreFilteredMap");
	environmentBRDF = CreateProgram(filePath, "vertScreen", "fragFunctionIBL fragEnvironmentBRDF");
	skybox = CreateProgram(filePath, "vertCube", "fragSkybox");

	// General shaders
	phongNotNormal = CreateProgram(filePath, "vertVarCommon vertMainCommon", "fragVarStandard fragVarSpecular fragMainPhong");
	phongNormal = CreateProgram(filePath, "vertVarCommon vertMainNormal", "fragVarStandard fragVarSpecular fragMainPhong");
	standardNotNormal = CreateProgram(filePath, "vertVarCommon vertMainCommon", "fragVarStandard fragVarMetallic fragFunctionLight fragMainMetallic");
	standardNormal = CreateProgram(filePath, "vertVarCommon vertMainNormal", "fragVarStandard fragVarMetallic fragFunctionLight fragMainMetallic");
	specularNotNormal = CreateProgram(filePath, "vertVarCommon vertMainCommon", "fragVarStandard fragVarSpecular fragFunctionLight fragMainSpecular");
	specularNormal = CreateProgram(filePath, "vertVarCommon vertMainNormal", "fragVarStandard fragVarSpecular fragFunctionLight fragMainSpecular");

	// Depth Prepass Shaders
	depthPrepass = CreateProgram(filePath, "vertVarCommon vertMainCommon", "fragDepthPrepass");

	// SSAO Shaders
	ssao = CreateProgram(filePath, "vertScreen", "fragSSAO");
	ssaoBlur = CreateProgram(filePath, "vertScreen", "fragGaussianBlur");

	// Shadow Shaders
	shadowMap = CreateProgram(filePath, "vertDepthMap", "fragDepthMap");

	//UI shaders
	textUI = CreateProgram(filePath, "vertTextUI", "fragTextUI");
	imageUI = CreateProgram(filePath, "vertImageUI", "fragImageUI");

	// Engine Shaders
	drawSSAOTexture = CreateProgram(filePath, "vertScreen", "fragDrawSSAOTexture");
	drawDepthMapTexture = CreateProgram(filePath, "vertScreen", "fragDrawDepthMapTexture");

	// Particle Shaders
	billboard = CreateProgram(filePath, "billboardVertex", "billboardFragment");
	trail = CreateProgram(filePath, "trailVertex", "trailFragment");

	unsigned timeMs = timer.Stop();
	LOG("Shaders loaded in %ums", timeMs);
}

void ModulePrograms::UnloadShaders() {
	glDeleteProgram(hdrToCubemap);
	glDeleteProgram(irradiance);
	glDeleteProgram(preFilteredMap);
	glDeleteProgram(environmentBRDF);
	glDeleteProgram(skybox);

	glDeleteProgram(phongNormal);
	glDeleteProgram(phongNotNormal);
	glDeleteProgram(standardNormal);
	glDeleteProgram(standardNotNormal);
	glDeleteProgram(specularNormal);
	glDeleteProgram(specularNotNormal);

	glDeleteProgram(depthPrepass);

	glDeleteProgram(ssao);
	glDeleteProgram(ssaoBlur);

	glDeleteProgram(shadowMap);

	glDeleteProgram(textUI);
	glDeleteProgram(imageUI);

	glDeleteProgram(drawSSAOTexture);
	glDeleteProgram(drawDepthMapTexture);

	glDeleteProgram(billboard);
	glDeleteProgram(trail);
}

unsigned ModulePrograms::CreateProgram(const char* shaderFile, const char* vertexSnippets, const char* fragmentSnippets) {
	LOG("Creating program...");

	// Compile the shaders and delete them at the end
	LOG("Compiling shaders...");
	unsigned vertexShader = CompileShader(GL_VERTEX_SHADER, shaderFile, vertexSnippets);
	DEFER {
		glDeleteShader(vertexShader);
	};
	unsigned fragmentShader = CompileShader(GL_FRAGMENT_SHADER, shaderFile, fragmentSnippets);
	DEFER {
		glDeleteShader(fragmentShader);
	};

	// Link the program
	LOG("Linking program...");
	unsigned programId = glCreateProgram();
	glAttachShader(programId, vertexShader);
	glAttachShader(programId, fragmentShader);
	glLinkProgram(programId);
	int res = GL_FALSE;
	glGetProgramiv(programId, GL_LINK_STATUS, &res);
	if (res == GL_FALSE) {
		int len = 0;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &len);
		if (len > 0) {
			int written = 0;
			Buffer<char> info = Buffer<char>(len);
			glGetProgramInfoLog(programId, len, &written, info.Data());
			LOG("Program Log Info: %s", info.Data());
		}

		LOG("Error linking program.");
	} else {
		LOG("Program linked.");
	}

	return programId;
}

bool ModulePrograms::Start() {
	LoadShaders();
	return true;
}

void ModulePrograms::DeleteProgram(unsigned int IdProgram) {
	glDeleteProgram(IdProgram);
}

bool ModulePrograms::CleanUp() {
	UnloadShaders();
	return true;
}