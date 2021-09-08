#include "ResourceVideo.h"
#include "Utils/Logging.h"

void ResourceVideo::Load() {
	std::string filePath = GetResourceFilePath();
	LOG("Loading video from path: \"%s\".", filePath.c_str());
}

void ResourceVideo::Unload() {
}
