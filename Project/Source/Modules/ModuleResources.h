#pragma once

#include "Module.h"
#include "Utils/Pool.h"
#include "Utils/UID.h"
#include "Resources/Resource.h"
#include "Utils/AssetFile.h"
#include "FileSystem/JsonValue.h"

#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <thread>

class ModuleResources : public Module {
public:
	bool Init() override;
	bool Start() override;
	UpdateStatus Update() override;
	bool CleanUp() override;
	void ReceiveEvent(TesseractEvent& e) override;

	std::vector<UID> ImportAssetResources(const char* filePath);

	template<typename T> T* GetResource(UID id) const;
	AssetFolder* GetRootFolder() const;

	void IncreaseReferenceCount(UID id);
	void DecreaseReferenceCount(UID id);
	unsigned GetReferenceCount(UID id) const;

	std::string GenerateResourcePath(UID id) const;

	template<typename T>
	T* CreateResource(const char* assetFilePath, UID id);

private:
	void UpdateAsync();

	void CheckForNewAssetsRecursive(const char* path, AssetFolder* folder);

	Resource* CreateResourceByType(ResourceType type, const char* assetFilePath, UID id);
	void SendAddResourceEvent(Resource* resource);

	void ValidateAssetResources(JsonValue jMeta, bool& validResourceFiles);
	void ReimportResources(JsonValue jMeta, const char* filePath);
	bool ImportAssetByExtension(JsonValue jMeta, const char* filePath);

public:
	std::unordered_set<std::string> assetsToNotUpdate;

private:
	std::unordered_map<UID, std::unique_ptr<Resource>> resources;
	std::unordered_map<UID, unsigned> referenceCounts;
	std::unique_ptr<AssetFolder> rootFolder;
	std::thread importThread;
	bool stopImportThread = false;
};

template<typename T>
inline T* ModuleResources::GetResource(UID id) const {
	auto it = resources.find(id);
	return it != resources.end() ? static_cast<T*>(it->second.get()) : nullptr;
}

template<typename T>
inline T* ModuleResources::CreateResource(const char* assetFilePath, UID id) {
	std::string resourceFilePath = GenerateResourcePath(id);
	T* resource = new T(id, assetFilePath, resourceFilePath.c_str());
	SendAddResourceEvent(resource);
	return resource;
}
