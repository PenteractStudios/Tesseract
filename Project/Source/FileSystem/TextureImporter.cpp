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
#include "ImporterCommon.h"

#include "IL/il.h"
#include "IL/ilu.h"
#include "GL/glew.h"
#include "rapidjson/prettywriter.h"

#include "Utils/Leaks.h"

bool TextureImporter::ImportTexture(const char* filePath, JsonValue jMeta) {
	LOG("Importing texture from path: \"%s\".", filePath);

	// Timer to measure importing a texture
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
	unsigned resourceIndex = 0;
	std::unique_ptr<ResourceTexture> texture = ImporterCommon::CreateResource<ResourceTexture>(FileDialog::GetFileName(filePath).c_str(), filePath, jMeta, resourceIndex);

	// Save resource meta file
	bool saved = ImporterCommon::SaveResourceMetaFile(texture.get());
	if (!saved) {
		LOG("Failed to save texture resource meta file.");
		return false;
	}

	// Save to file
	saved = App->files->Save(texture->GetResourceFilePath().c_str(), buffer);
	if (!saved) {
		LOG("Failed to save texture resource file.");
		return false;
	}

	unsigned timeMs = timer.Stop();
	LOG("Texture imported in %ums", timeMs);
	return true;
}