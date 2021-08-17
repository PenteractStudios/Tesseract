#pragma once

#include "Utils/JsonValue.h"

namespace SceneImporter {
	bool ImportScene(const char* filePath, JsonValue jMeta);

	void LoadScene(const char* filePath);
	bool SaveScene(const char* filePath);
} // namespace SceneImporter