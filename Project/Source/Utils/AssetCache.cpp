#include "AssetCache.h"

AssetFile::AssetFile(const char* path_)
	: path(path_) {}

AssetFolder::AssetFolder(const char* path_)
	: path(path_) {}

AssetCache::AssetCache(const char* rootPath)
	: root(rootPath) {
	foldersMap[rootPath] = &root;
}
