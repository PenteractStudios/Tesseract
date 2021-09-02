#pragma once

#include "Globals.h"
#include "Application.h"
#include "Module.h"
#include "ModuleEvents.h"
#include "Utils/UID.h"
#include "Utils/AssetCache.h"
#include "Resources/Resource.h"
#include "FileSystem/JsonValue.h"
#include "FileSystem/ImportOptions.h"
#include "TesseractEvent.h"

#include <string>
#include <list>
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

	std::list<UID> ImportAssetResources(const char* filePath, bool force = false);

	template<typename T> T* GetImportOptions(const char* filePath, bool forceLoad = false);
	template<typename T> T* GetResource(UID id);
	AssetCache* GetAssetCache() const;

	void IncreaseReferenceCount(UID id);
	void DecreaseReferenceCount(UID id);
	unsigned GetReferenceCount(UID id) const;

	std::string GenerateResourcePath(UID id) const;

	template<typename T> std::unique_ptr<T> CreateResource(const char* resourceName, const char* assetFilePath, UID id);
	template<typename T> void SendCreateResourceEvent(std::unique_ptr<T>& resource);

private:
	void UpdateAsync();
	void ImportLibrary();

	void CheckForNewAssetsRecursive(const char* path, AssetCache& assetCache, AssetFolder& parentFolder);

	void SendCreateResourceEventByType(ResourceType type, const char* resourceName, const char* assetFilePath, UID id);
	Resource* CreateResourceByType(ResourceType type, const char* resourceName, const char* assetFilePath, UID id);
	void DestroyResource(UID id);

	void ValidateAssetResources(JsonValue jMeta, bool& validResourceFiles);
	void RecreateResources(JsonValue jMeta, const char* filePath);
	bool ImportAssetByExtension(JsonValue jMeta, const char* filePath);
	void ImportLibraryResource(const char* filePath);

	void LoadResource(Resource* resource);
	void LoadImportOptions(std::unique_ptr<ImportOptions>& importOptions, const char* filePath);

public:
	concurrency::concurrent_queue<std::string> assetsToReimport;

private:
	std::unordered_map<std::string, std::unique_ptr<ImportOptions>> assetImportOptions;
	std::unordered_map<UID, std::unique_ptr<Resource>> resources;
	std::unordered_map<UID, unsigned> referenceCounts;
	std::unique_ptr<AssetCache> assetCache;

	std::thread importThread;
	bool stopImportThread = false;
	std::unordered_map<UID, std::string> concurrentResourceUIDToAssetFilePath;
};

template<typename T>
inline T* ModuleResources::GetImportOptions(const char* filePath, bool forceLoad) {
	auto it = assetImportOptions.find(filePath);
	if (forceLoad || it == assetImportOptions.end()) {
		std::unique_ptr<ImportOptions>& importOptions = assetImportOptions[filePath];
		LoadImportOptions(importOptions, filePath);
		return reinterpret_cast<std::unique_ptr<T>&>(importOptions).get();
	} else {
		return reinterpret_cast<std::unique_ptr<T>&>(it->second).get();
	}
}

template<typename T>
inline T* ModuleResources::GetResource(UID id) {
	auto it = resources.find(id);
	T* resource = it != resources.end() ? static_cast<T*>(it->second.get()) : nullptr;
	return resource;
}

template<typename T>
inline std::unique_ptr<T> ModuleResources::CreateResource(const char* resourceName, const char* assetFilePath, UID id) {
	return std::unique_ptr<T>(static_cast<T*>(CreateResourceByType(T::staticType, resourceName, assetFilePath, id)));
}

template<typename T>
inline void ModuleResources::SendCreateResourceEvent(std::unique_ptr<T>& resource) {
	concurrentResourceUIDToAssetFilePath[resource->GetId()] = resource->GetAssetFilePath();

	TesseractEvent addResourceEvent(TesseractEventType::CREATE_RESOURCE);
	addResourceEvent.Set<CreateResourceStruct>(T::staticType, resource->GetId(), resource->GetName().c_str(), resource->GetAssetFilePath().c_str());
	App->events->AddEvent(addResourceEvent);
}
