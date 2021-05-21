#include "ResourceShader.h"

#include "Application.h"
#include "Utils/Logging.h"
#include "FileSystem/JsonValue.h"
#include "Modules/ModulePrograms.h"
#include "Modules/ModuleFiles.h"
#include "Modules/ModuleTime.h"

#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/document.h"

#include "Utils/Leaks.h"

#define META_EXTENSION ".meta"
#define JSON_TAG_RESOURCES "Resources"
#define JSON_TAG_TYPE "Type"
#define JSON_TAG_ID "Id"
#define JSON_TAG_TIMESTAMP "Timestamp"

static bool ReadMetaFile(const char* filePath, rapidjson::Document& document) {
	// Read from file
	Buffer<char> buffer = App->files->Load(filePath);
	if (buffer.Size() == 0) {
		LOG("Error reading meta file %s", filePath);
		return false;
	}

	// Parse document from file
	document.Parse<rapidjson::kParseNanAndInfFlag>(buffer.Data());
	if (document.HasParseError()) {
		LOG("Error parsing JSON: %s (offset: %u)", rapidjson::GetParseError_En(document.GetParseError()), document.GetErrorOffset());
		return false;
	}

	return true;
}

static void SaveMetaFile(const char* filePath, rapidjson::Document& document) {
	// Write document to buffer
	rapidjson::StringBuffer stringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>, rapidjson::UTF8<>, rapidjson::CrtAllocator, rapidjson::kWriteNanAndInfFlag> writer(stringBuffer);
	document.Accept(writer);

	// Save to file
	App->files->Save(filePath, stringBuffer.GetString(), stringBuffer.GetSize());
}

void ResourceShader::Load() {
	// Timer to measure loading a shader
	MSTimer timer;
	timer.Start();
	std::string filePath = GetResourceFilePath();
	LOG("Loading shader from path: \"%s\".", filePath.c_str());

	shaderProgram = App->programs->CreateProgram(filePath.c_str());

	unsigned timeMs = timer.Stop();
	LOG("Shader loaded in %ums.", timeMs);
}

void ResourceShader::Unload() {
	App->programs->DeleteProgram(shaderProgram);
}

unsigned ResourceShader::GetShaderProgram() {
	return shaderProgram;
}
