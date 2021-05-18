#pragma once

#include "FileSystem/JsonValue.h"

namespace FontImporter {
	bool ImportFont(const char* filePath, JsonValue jMeta);
}; //namespace FontImporter