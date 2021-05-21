#include "ScriptImporter.h"

#include "Application.h"
#include "Modules/ModuleResources.h"
#include "Resources/ResourceScript.h"

#include "Globals.h"
#include "Utils/Logging.h"
#include "Utils/Buffer.h"

#include "Modules/ModuleResources.h"
#include "Modules/ModuleFiles.h"
#include "Modules/ModuleTime.h"

#include "Utils/Leaks.h"

#define JSON_TAG_RESOURCES "Resources"
#define JSON_TAG_TYPE "Type"
#define JSON_TAG_ID "Id"

bool ScriptImporter::ImportScript(const char* filePath, JsonValue jMeta) {
	LOG("Importing Script from path: \"%s\".", filePath);

	// Timer to measure importing a material
	MSTimer timer;
	timer.Start();

	// Read from file
	Buffer<char> buffer = App->files->Load(filePath);
	if (buffer.Size() == 0) {
		LOG("Error loading script %s", filePath);
		return false;
	}

	// Script resource creation
	JsonValue jResources = jMeta[JSON_TAG_RESOURCES];
	JsonValue jResource = jResources[0];
	UID metaId = jResource[JSON_TAG_ID];
	UID id = metaId ? metaId : GenerateUID();
	App->resources->CreateResource<ResourceScript>(filePath, id);

	// Add resource to meta file
	jResource[JSON_TAG_TYPE] = GetResourceTypeName(ResourceScript::staticType);
	jResource[JSON_TAG_ID] = id;

	// Save to file
	const std::string& resourceFilePath = App->resources->GenerateResourcePath(id);
	bool saved = App->files->Save(resourceFilePath.c_str(), buffer);
	if (!saved) {
		LOG("Failed to save script resource.");
		return false;
	}

	unsigned timeMs = timer.Stop();
	LOG("Script imported in %ums", timeMs);
	return true;
}