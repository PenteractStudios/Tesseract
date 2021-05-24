#pragma once

#include "UID.h"

#include <string>
#include <list>
#include <unordered_map>

struct AssetFile {
	AssetFile(const char* path);

	std::string path;
	std::list<UID> resourceIds;
};

struct AssetFolder {
	AssetFolder(const char* path);

	std::string path;
	std::list<AssetFolder> folders;
	std::list<AssetFile> files;
};

struct AssetCache {
	AssetCache(const char* rootPath);

	AssetFolder root;
	std::unordered_map<std::string, AssetFolder*> foldersMap;
	std::unordered_map<std::string, AssetFile*> filesMap;
};