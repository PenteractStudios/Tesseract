#include "VideoImporter.h"

#include "Application.h"
#include "Utils/Logging.h"
#include "Utils/Buffer.h"
#include "Utils/FileDialog.h"
#include "Resources/ResourceVideo.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleTime.h"
#include "Modules/ModuleTime.h"
#include "ImporterCommon.h"

bool VideoImporter::ImportVideo(const char* filePath, JsonValue jMeta) {
	// Timer to measure importing audio
	MSTimer timer;
	timer.Start();

	// Read from file
	Buffer<char> buffer = App->files->Load(filePath);
	if (buffer.Size() == 0) {
		LOG("Error loading audio %s", filePath);
		return false;
	}

	// Create video resource
	unsigned resourceIndex = 0;
	std::unique_ptr<ResourceVideo> audioClip = ImporterCommon::CreateResource<ResourceVideo>(FileDialog::GetFileName(filePath).c_str(), filePath, jMeta, resourceIndex);

	// Save resource meta file
	bool saved = ImporterCommon::SaveResourceMetaFile(audioClip.get());
	if (!saved) {
		LOG("Failed to save audio clip resource meta file.");
		return false;
	}

	// Save to file
	saved = App->files->Save(audioClip->GetResourceFilePath().c_str(), buffer);
	if (!saved) {
		LOG("Failed to save audio clip resource file.");
		return false;
	}

	// Save to file
	saved = App->files->Save(audioClip->GetResourceFilePath().c_str(), buffer);
	if (!saved) {
		LOG("Failed to save audio clip resource file.");
		return false;
	}

	// Send resource creation event
	App->resources->SendCreateResourceEvent(audioClip);

	unsigned timeMs = timer.Stop();
	LOG("Audio imported in %ums", timeMs);
	return true;
}
