#pragma once

#include "Globals.h"
#include "Module.h"
#include "Utils/Buffer.h"

#include <string>
#include <vector>

constexpr char* extensions[] = {JPG_TEXTURE_EXTENSION, PNG_TEXTURE_EXTENSION, DDS_TEXTURE_EXTENSION, SCENE_EXTENSION, PREFAB_EXTENSION};

enum class AllowedExtensionsFlag {
	ALL = 0,
	JPG = 1,
	PNG = 1 << 1,
	DDS = 1 << 2,
	SCENE = 1 << 3,
	PREFAB = 1 << 4
};

inline AllowedExtensionsFlag operator|(AllowedExtensionsFlag l, AllowedExtensionsFlag r) {
	return static_cast<AllowedExtensionsFlag>(static_cast<int>(l) + static_cast<int>(r));
}

class ModuleFiles : public Module {
public:
	bool Init() override;
	bool CleanUp() override;

	Buffer<char> Load(const char* filePath) const;
	bool Save(const char* filePath, const Buffer<char>& buffer, bool append = false) const;
	bool Save(const char* filePath, const char* buffer, size_t size, bool append = false) const;
	void CreateFolder(const char* folderPath) const;
	void Erase(const char* path) const;
	bool AddSearchPath(const char* searchPath) const;

	bool Exists(const char* filePath) const;
	bool IsDirectory(const char* path) const;

	std::string GetFilePath(const char* file, bool absolute = false) const;
	std::vector<std::string> GetFilesInFolder(const char* folderPath) const;

	long long GetLocalFileModificationTime(const char* path) const;
};
