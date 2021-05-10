#include "ClipImporter.h"

#include "Utils/Logging.h"
#include "Utils/MSTimer.h"
#include "Utils/Buffer.h"

#include "Application.h"
#include "Modules/ModuleResources.h"
#include "Resources/Resourceclip.h"

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

bool ClipImporter::ImportClip(const char* filePath, JsonValue jMeta) {
	LOG("Importing clip from path: \"%s\".", filePath);

	// Timer to measure importing a Clip
	MSTimer timer;
	timer.Start();

	// Read from file
	Buffer<char> buffer = App->files->Load(filePath);
	if (buffer.Size() == 0) {
		LOG("Error loading clip %s", filePath);
		return false;
	}

	// Parse document from file
	rapidjson::Document document;
	document.ParseInsitu<rapidjson::kParseNanAndInfFlag>(buffer.Data());
	if (document.HasParseError()) {
		LOG("Error parsing JSON: %s (offset: %u)", rapidjson::GetParseError_En(document.GetParseError()), document.GetErrorOffset());
		return false;
	}

	// Clip resource creation
	JsonValue jResources = jMeta[JSON_TAG_RESOURCES];
	JsonValue jResource = jResources[0];
	UID metaId = jResource[JSON_TAG_ID];
	UID id = metaId ? metaId : GenerateUID();
	App->resources->CreateResource<ResourceClip>(filePath, id);

	// Add resource to meta file
	jResource[JSON_TAG_TYPE] = GetResourceTypeName(ResourceClip::staticType);
	jResource[JSON_TAG_ID] = id;

	// Write document to buffer
	rapidjson::StringBuffer stringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>, rapidjson::UTF8<>, rapidjson::CrtAllocator, rapidjson::kWriteNanAndInfFlag> writer(stringBuffer);
	document.Accept(writer);

	// Save to file
	const std::string& resourceFilePath = App->resources->GenerateResourcePath(id);
	bool saved = App->files->Save(resourceFilePath.c_str(), stringBuffer.GetString(), stringBuffer.GetSize());
	if (!saved) {
		LOG("Failed to save clip resource.");
		return false;
	}

	unsigned timeMs = timer.Stop();
	LOG("Clip imported in %ums", timeMs);
	return true;
}
