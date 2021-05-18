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
#include <mutex>

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
	void CreateResource(const char* assetFilePath, UID id);
	void DestroyResource(UID id);

private:
	void UpdateAsync();

	void CheckForNewAssetsRecursive(const char* path, AssetFolder* folder);

	void CreateResourceByType(ResourceType type, const char* assetFilePath, UID id);
	Resource* DoCreateResourceByType(ResourceType type, const char* assetFilePath, UID id);

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
	std::mutex resourcesMutex;
	bool stopImportThread = false;
	std::vector<Resource> importedResources;
};

template<typename T>
inline T* ModuleResources::GetResource(UID id) {
	resourcesMutex.lock();
	auto it = resources.find(id);
	T* resource = it != resources.end() ? static_cast<T*>(it->second.get()) : nullptr;
	resourcesMutex.unlock();
	return resource;
}

template<typename T>
inline void ModuleResources::CreateResource(const char* assetFilePath, UID id) {
	TesseractEvent addResourceEvent(TesseractEventType::CREATE_RESOURCE);
	addResourceEvent.Set<CreateResourceStruct>(T::staticType, id, assetFilePath);
	App->events->AddEvent(addResourceEvent);
}
