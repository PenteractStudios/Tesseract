#pragma once

#include "FileSystem/JsonValue.h"

namespace AudioImporter {
	bool ImportAudio(const char* filePath, JsonValue jMeta);
};
