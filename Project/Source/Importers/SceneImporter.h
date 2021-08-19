#pragma once

#include "Utils/JsonValue.h"

namespace SceneImporter {
	bool ImportScene(const char* filePath, JsonValue jMeta);
} // namespace SceneImporter