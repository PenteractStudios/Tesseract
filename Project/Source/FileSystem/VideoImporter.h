#pragma once

#include "FileSystem/JsonValue.h"

namespace VideoImporter {

	bool ImportVideo(const char* filePath, JsonValue jMeta);
};
