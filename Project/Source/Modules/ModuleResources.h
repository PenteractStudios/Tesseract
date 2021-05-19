#pragma once

#include "Application.h"
#include "Module.h"
#include "ModuleEvents.h"
#include "Utils/Pool.h"
#include "Utils/UID.h"
#include "Resources/Resource.h"
#include "Utils/AssetFile.h"
#include "FileSystem/JsonValue.h"
#include "TesseractEvent.h"

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

	template<typename T> T* GetResource(UID id);
	AssetFolder* GetRootFolder() const;

	void IncreaseReferenceCount(UID id);
	void DecreaseReferenceCount(UID id);
	unsigned GetReferenceCount(UID id) const;

	std::string GenerateResourcePath(UID id) const;

	template<typename T>
	std::unique_ptr<T> CreateResource(const char* resourceName, const char* assetFilePath, UID id); // Returns a copy of the resource for convenience. Any changes to the resource will be lost unless saved.

private:
	void UpdateAsync();

	void CheckForNewAssetsRecursive(const char* path, AssetFolder* folder);

	void CreateResourceByType(ResourceType type, const char* resourceName, const char* assetFilePath, UID id);
	Resource* DoCreateResourceByType(ResourceType type, const char* resourceName, const char* assetFilePath, UID id);
	void DestroyResource(UID id);

	void ValidateAssetResources(JsonValue jMeta, bool& validResourceFiles);
	void RecreateResources(JsonValue jMeta, const char* filePath);
	bool ImportAssetByExtension(JsonValue jMeta, const char* filePath);

public:
	std::unordered_set<std::string> assetsToNotUpdate;

private:
	std::unordered_map<UID, std::unique_ptr<Resource>> resources;
	std::unordered_map<UID, unsigned> referenceCounts;
	std::unique_ptr<AssetFolder> rootFolder;

	std::thread importThread;
	bool stopImportThread = false;
	std::unordered_map<UID, std::string> concurrentResourceUIDToAssetFilePath;
};

template<typename T>
inline T* ModuleResources::GetResource(UID id) {
	auto it = resources.find(id);
	T* resource = it != resources.end() ? static_cast<T*>(it->second.get()) : nullptr;
	return resource;
}

template<typename T>
inline std::unique_ptr<T> ModuleResources::CreateResource(const char* resourceName, const char* assetFilePath, UID id) {
	concurrentResourceUIDToAssetFilePath[id] = assetFilePath;

	TesseractEvent addResourceEvent(TesseractEventType::CREATE_RESOURCE);
	addResourceEvent.Set<CreateResourceStruct>(T::staticType, id, resourceName, assetFilePath);
	App->events->AddEvent(addResourceEvent);

	return std::unique_ptr<T>(static_cast<T*>(DoCreateResourceByType(T::staticType, resourceName, assetFilePath, id)));
}
