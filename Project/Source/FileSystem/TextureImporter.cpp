#include "TextureImporter.h"

#include "Globals.h"
#include "Application.h"
#include "Utils/Logging.h"
#include "Utils/Buffer.h"
#include "Utils/FileDialog.h"
#include "Resources/ResourceTexture.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleFiles.h"
#include "Modules/ModuleTime.h"

#include "IL/il.h"
#include "IL/ilu.h"
#include "GL/glew.h"

#include "Utils/Leaks.h"

#define JSON_TAG_RESOURCES "Resources"
#define JSON_TAG_TYPE "Type"
#define JSON_TAG_ID "Id"

bool TextureImporter::ImportTexture(const char* filePath, JsonValue jMeta) {
	LOG("Importing texture from path: \"%s\".", filePath);

	// Generate image handler
	unsigned image;
	ilGenImages(1, &image);
	DEFER {
		ilDeleteImages(1, &image);
	};

	// Load image
	ilBindImage(image);
	bool imageLoaded = ilLoadImage(filePath);
	if (!imageLoaded) {
		LOG("Failed to load image.");
		return false;
	}
	bool imageConverted = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	if (!imageConverted) {
		LOG("Failed to convert image.");
		return false;
	}

	ilSetInteger(IL_DXTC_FORMAT, IL_DXT5);
	size_t size = ilSaveL(IL_DDS, nullptr, 0);
	if (size == 0) {
		LOG("Failed to save image.");
		return false;
	}
	Buffer<char> buffer = Buffer<char>(size);
	size = ilSaveL(IL_DDS, buffer.Data(), size);
	if (size == 0) {
		LOG("Failed to save image.");
		return false;
	}

	// Create texture resource
	JsonValue jResources = jMeta[JSON_TAG_RESOURCES];
	JsonValue jResource = jResources[0];
	UID metaId = jResource[JSON_TAG_ID];
	UID id = metaId ? metaId : GenerateUID();
	App->resources->CreateResource<ResourceTexture>(filePath, id);

	// Add resource to meta file
	jResource[JSON_TAG_TYPE] = GetResourceTypeName(ResourceTexture::staticType);
	jResource[JSON_TAG_ID] = id;

	// Save to file
	const std::string& resourceFilePath = App->resources->GenerateResourcePath(id);
	bool saved = App->files->Save(resourceFilePath.c_str(), buffer);
	if (!saved) {
		LOG("Failed to save texture resource.");
		return false;
	}

	return true;
}