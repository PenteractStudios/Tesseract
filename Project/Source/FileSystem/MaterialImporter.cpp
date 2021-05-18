#include "MaterialImporter.h"

#include "Utils/Logging.h"
#include "Utils/MSTimer.h"
#include "Utils/Buffer.h"

#include "Application.h"
#include "Modules/ModuleResources.h"
#include "Resources/ResourceMaterial.h"

#include "Modules/ModuleFiles.h"
#include "Modules/ModuleTime.h"

#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/document.h"

#include "Utils/Leaks.h"

#define JSON_TAG_RESOURCES "Resources"
#define JSON_TAG_TYPE "Type"
#define JSON_TAG_ID "Id"

#define JSON_TAG_SHADER "ShaderType"
#define JSON_TAG_HAS_DIFFUSE_MAP "HasDiffuseMap"
#define JSON_TAG_DIFFUSE_COLOR "DiffuseColor"
#define JSON_TAG_DIFFUSE_MAP "DiffuseMap"
#define JSON_TAG_HAS_SPECULAR_MAP "HasSpecularMap"
#define JSON_TAG_SPECULAR_COLOR "SpecularColor"
#define JSON_TAG_SPECULAR_MAP "SpecularMap"
#define JSON_TAG_METALLIC_MAP "MetallicMap"
#define JSON_TAG_METALLIC "Metalness"
#define JSON_TAG_NORMAL_MAP "NormalMap"
#define JSON_TAG_SMOOTHNESS "Smoothness"
#define JSON_TAG_HAS_SMOOTHNESS_IN_ALPHA_CHANNEL "HasSmoothnessInAlphaChannel"
#define JSON_TAG_TILING "Tiling"
#define JSON_TAG_OFFSET "Offset"

bool MaterialImporter::ImportMaterial(const char* filePath, JsonValue jMeta) {
	LOG("Importing material from path: \"%s\".", filePath);

	// Timer to measure importing a material
	MSTimer timer;
	timer.Start();

	// Read from file
	Buffer<char> buffer = App->files->Load(filePath);
	if (buffer.Size() == 0) {
		LOG("Error loading material %s", filePath);
		return false;
	}

	// Parse document from file
	rapidjson::Document document;
	document.ParseInsitu<rapidjson::kParseNanAndInfFlag>(buffer.Data());
	if (document.HasParseError()) {
		LOG("Error parsing JSON: %s (offset: %u)", rapidjson::GetParseError_En(document.GetParseError()), document.GetErrorOffset());
		return false;
	}

	// Material resource creation
	JsonValue jResources = jMeta[JSON_TAG_RESOURCES];
	JsonValue jResource = jResources[0];
	UID metaId = jResource[JSON_TAG_ID];
	UID id = metaId ? metaId : GenerateUID();
	App->resources->CreateResource<ResourceMaterial>(filePath, id);

	// Add resource to meta file
	jResource[JSON_TAG_TYPE] = GetResourceTypeName(ResourceMaterial::staticType);
	jResource[JSON_TAG_ID] = id;

	// Write document to buffer
	rapidjson::StringBuffer stringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>, rapidjson::UTF8<>, rapidjson::CrtAllocator, rapidjson::kWriteNanAndInfFlag> writer(stringBuffer);
	document.Accept(writer);

	// Save to file
	const std::string& resourceFilePath = App->resources->GenerateResourcePath(id);
	bool saved = App->files->Save(resourceFilePath.c_str(), stringBuffer.GetString(), stringBuffer.GetSize());
	if (!saved) {
		LOG("Failed to save material resource.");
		return false;
	}

	unsigned timeMs = timer.Stop();
	LOG("Material imported in %ums", timeMs);
	return true;
}

bool MaterialImporter::CreateAndSaveMaterial(const char* filePath) {
	LOG("Saving material to path: \"%s\".", filePath);

	MSTimer timer;
	timer.Start();

	// Create document
	rapidjson::Document document;
	JsonValue jMaterial(document, document);

	// Save JSON values
	jMaterial[JSON_TAG_SHADER] = (int) MaterialShader::STANDARD;

	jMaterial[JSON_TAG_HAS_DIFFUSE_MAP] = false;
	JsonValue jDiffuseColor = jMaterial[JSON_TAG_DIFFUSE_COLOR];
	jDiffuseColor[0] = 1.0f;
	jDiffuseColor[1] = 1.0f;
	jDiffuseColor[2] = 1.0f;
	jDiffuseColor[3] = 1.0f;
	jMaterial[JSON_TAG_DIFFUSE_MAP] = 0;

	jMaterial[JSON_TAG_HAS_SPECULAR_MAP] = false;
	JsonValue jSpecularColor = jMaterial[JSON_TAG_SPECULAR_COLOR];
	jSpecularColor[0] = 0.15f;
	jSpecularColor[1] = 0.15f;
	jSpecularColor[2] = 0.15f;
	jSpecularColor[3] = 1.f;
	jMaterial[JSON_TAG_SPECULAR_MAP] = 0;

	jMaterial[JSON_TAG_METALLIC] = 0.0f;
	jMaterial[JSON_TAG_METALLIC_MAP] = 0;
	jMaterial[JSON_TAG_NORMAL_MAP] = 0;

	jMaterial[JSON_TAG_SMOOTHNESS] = 1;
	jMaterial[JSON_TAG_HAS_SMOOTHNESS_IN_ALPHA_CHANNEL] = false;

	JsonValue jTiling = jMaterial[JSON_TAG_TILING];
	jTiling[0] = 1.0f;
	jTiling[1] = 1.0f;
	JsonValue jOffset = jMaterial[JSON_TAG_OFFSET];
	jOffset[0] = 0.0f;
	jOffset[1] = 0.0f;

	// Write document to buffer
	rapidjson::StringBuffer stringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>, rapidjson::UTF8<>, rapidjson::CrtAllocator, rapidjson::kWriteNanAndInfFlag> writer(stringBuffer);
	document.Accept(writer);

	// Save to file
	bool saved = App->files->Save(filePath, stringBuffer.GetString(), stringBuffer.GetSize());
	if (!saved) {
		LOG("Failed to save material asset.");
		return false;
	}

	unsigned timeMs = timer.Stop();
	LOG("Material saved in %ums", timeMs);
	return true;
}
