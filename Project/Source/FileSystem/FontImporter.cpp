#include "FontImporter.h"

#include "Application.h"
#include "Modules/ModuleFiles.h"
#include "Modules/ModuleResources.h"
#include "Resources/ResourceFont.h"
#include "Utils/Buffer.h"
#include "Utils/Logging.h"

#include "Utils/Leaks.h"

#define JSON_TAG_RESOURCES "Resources"
#define JSON_TAG_TYPE "Type"
#define JSON_TAG_ID "Id"

bool FontImporter::ImportFont(const char* filePath, JsonValue jMeta) {
	LOG("Importing font from path: \"%s\".", filePath);
	Buffer<char> buffer;
	buffer = App->files->Load(filePath);

	// Create font resource
	JsonValue jResources = jMeta[JSON_TAG_RESOURCES];
	JsonValue jResource = jResources[0];
	UID metaId = jResource[JSON_TAG_ID];
	UID id = metaId ? metaId : GenerateUID();
	App->resources->CreateResource<ResourceFont>(filePath, id);

	// Add resource to meta file
	jResource[JSON_TAG_TYPE] = GetResourceTypeName(ResourceFont::staticType);
	jResource[JSON_TAG_ID] = id;

	// Save to file
	const std::string& resourceFilePath = App->resources->GenerateResourcePath(id);
	bool saved = App->files->Save(resourceFilePath.c_str(), buffer);
	
	if (!saved) {
		LOG("Failed to save font resource.");
		return false;
	}

	return true;
}