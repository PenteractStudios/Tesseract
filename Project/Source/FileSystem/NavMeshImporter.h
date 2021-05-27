#pragma once

#include "FileSystem/JsonValue.h"

class NavMesh;

namespace NavMeshImporter {
	bool ImportNavMesh(const char* filePath, JsonValue jMeta);
	bool ExportNavMesh(NavMesh& navMesh, const char* filePath);
} // namespace MaterialImporter