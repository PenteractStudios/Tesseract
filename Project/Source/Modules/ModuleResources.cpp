#include "ModuleResources.h"

#include "Globals.h"
#include "Application.h"
#include "Utils/Logging.h"
#include "Utils/FileDialog.h"
#include "Resources/ResourcePrefab.h"
#include "Resources/ResourceMaterial.h"
#include "Resources/ResourceMesh.h"
#include "Resources/ResourceScene.h"
#include "Resources/ResourceShader.h"
#include "Resources/ResourceTexture.h"
#include "Resources/ResourceFont.h"
#include "Resources/ResourceSkybox.h"
#include "Resources/ResourceScript.h"
#include "Resources/ResourceAnimation.h"
#include "Resources/ResourceStateMachine.h"
#include "Resources/ResourceClip.h"
#include "Resources/ResourceAudioClip.h"
#include "FileSystem/SceneImporter.h"
#include "FileSystem/ModelImporter.h"
#include "FileSystem/PrefabImporter.h"
#include "FileSystem/TextureImporter.h"
#include "FileSystem/MaterialImporter.h"
#include "FileSystem/SkyboxImporter.h"
#include "FileSystem/ShaderImporter.h"
#include "FileSystem/AudioImporter.h"
#include "FileSystem/StateMachineImporter.h"
#include "FileSystem/ClipImporter.h"
#include "UI/FontImporter.h"
#include "FileSystem/ScriptImporter.h"
#include "Modules/ModuleTime.h"
#include "Modules/ModuleFiles.h"
#include "Modules/ModuleInput.h"
#include "Modules/ModuleEvents.h"
#include "TesseractEvent.h"
#include "Utils/AssetFile.h"

#include "IL/il.h"
#include "IL/ilu.h"
#include "GL/glew.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/error/en.h"
#include <string>
#include <future>
#include <chrono>
#include "Brofiler.h"

#include "Utils/Leaks.h"

#define JSON_TAG_RESOURCES "Resources"
#define JSON_TAG_TYPE "Type"
#define JSON_TAG_ID "Id"
#define JSON_TAG_TIMESTAMP "Timestamp"

static bool ReadMetaFile(const char* filePath, rapidjson::Document& document) {
	// Read from file
	Buffer<char> buffer = App->files->Load(filePath);
	if (buffer.Size() == 0) {
		LOG("Error reading meta file %s", filePath);
		return false;
	}

	// Parse document from file
	document.Parse<rapidjson::kParseNanAndInfFlag>(buffer.Data());
	if (document.HasParseError()) {
		LOG("Error parsing JSON: %s (offset: %u)", rapidjson::GetParseError_En(document.GetParseError()), document.GetErrorOffset());
		return false;
	}

	return true;
}

static void SaveMetaFile(const char* filePath, rapidjson::Document& document) {
	// Write document to buffer
	rapidjson::StringBuffer stringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>, rapidjson::UTF8<>, rapidjson::CrtAllocator, rapidjson::kWriteNanAndInfFlag> writer(stringBuffer);
	document.Accept(writer);

	// Save to file
	App->files->Save(filePath, stringBuffer.GetString(), stringBuffer.GetSize());
}

bool ModuleResources::Init() {
	ilInit();
	iluInit();
	App->events->AddObserverToEvent(TesseractEventType::ADD_RESOURCE, this);
	App->events->AddObserverToEvent(TesseractEventType::UPDATE_FOLDERS, this);
	return true;
}

bool ModuleResources::Start() {
	stopImportThread = false;

	importThread = std::thread(&ModuleResources::UpdateAsync, this);

	return true;
}

UpdateStatus ModuleResources::Update() {
	BROFILER_CATEGORY("ModuleResources - Update", Profiler::Color::Orange)
	// Copy dropped file to assets folder
	const char* droppedFilePath = App->input->GetDroppedFilePath();
	if (droppedFilePath != nullptr) {
		std::string newFilePath = std::string(ASSETS_PATH) + "/" + FileDialog::GetFileNameAndExtension(droppedFilePath);
		FileDialog::Copy(droppedFilePath, newFilePath.c_str());
		App->input->ReleaseDroppedFilePath();
	}
	return UpdateStatus::CONTINUE;
}

bool ModuleResources::CleanUp() {
	stopImportThread = true;
	importThread.join();
	return true;
}

void ModuleResources::ReceiveEvent(TesseractEvent& e) {
	if (e.type == TesseractEventType::ADD_RESOURCE) {
		Resource* resource = e.Get<AddResourceStruct>().resource;
		UID id = resource->GetId();
		if (GetReferenceCount(id) > 0) {
			resource->Load();
		}
		resources[id].reset(resource);
		e.Get<AddResourceStruct>().resource = nullptr;
	} else if (e.type == TesseractEventType::UPDATE_FOLDERS) {
		AssetFolder* folder = e.Get<UpdateFoldersStruct>().folder;
		rootFolder.reset(folder);

		e.Get<UpdateFoldersStruct>().folder = nullptr;
	}
}

void ModuleResources::ValidateAssetResources(JsonValue jMeta, bool& validResourceFiles) {
	JsonValue jResources = jMeta[JSON_TAG_RESOURCES];
	for (unsigned i = 0; i < jResources.Size(); ++i) {
		UID id = jResources[i][JSON_TAG_ID];
		std::string resourcePath = App->resources->GenerateResourcePath(id);
		if (!App->files->Exists(resourcePath.c_str())) {
			validResourceFiles = false;
			break;
		}
	}
}

void ModuleResources::ReimportResources(JsonValue jMeta, const char* filePath) {
	JsonValue jResources = jMeta[JSON_TAG_RESOURCES];
	for (unsigned i = 0; i < jResources.Size(); ++i) {
		JsonValue jResource = jResources[i];
		UID id = jResource[JSON_TAG_ID];
		if (GetResource<Resource>(id) == nullptr) {
			std::string typeName = jResource[JSON_TAG_TYPE];
			ResourceType type = GetResourceTypeFromName(typeName.c_str());
			CreateResourceByType(type, filePath, id);
		}
	}
}

bool ModuleResources::ImportAssetByExtension(JsonValue jMeta, const char* filePath) {
	bool validExtension = true;
	std::string extension = FileDialog::GetFileExtension(filePath);

	Resource* resource = nullptr;
	if (extension == SCENE_EXTENSION) {
		// Scene files
		SceneImporter::ImportScene(filePath, jMeta);
	} else if (extension == MATERIAL_EXTENSION) {
		// Material files
		MaterialImporter::ImportMaterial(filePath, jMeta);
	} else if (extension == FRAGMENT_SHADER_EXTENSION || extension == VERTEX_SHADER_EXTENSION || extension == DEFAULT_SHADER_EXTENSION) {
		// Shader files
		ShaderImporter::ImportShader(filePath, jMeta);
	} else if (extension == DEFAULT_MODEL_EXTENSION || extension == OBJ_MODEL_EXTENSION) {
		// Model files
		ModelImporter::ImportModel(filePath, jMeta);
	} else if (extension == SKYBOX_EXTENSION) {
		// Skybox files
		SkyboxImporter::ImportSkybox(filePath, jMeta);
	} else if (extension == JPG_TEXTURE_EXTENSION || extension == PNG_TEXTURE_EXTENSION || extension == TIF_TEXTURE_EXTENSION || extension == DDS_TEXTURE_EXTENSION || extension == TGA_TEXTURE_EXTENSION) {
		// Texture files
		TextureImporter::ImportTexture(filePath, jMeta);
	} else if (extension == FONT_EXTENSION) {
		// Font files
		FontImporter::ImportFont(filePath, jMeta);
	} else if (extension == SCRIPT_EXTENSION) {
		// Script files
		ScriptImporter::ImportScript(filePath, jMeta);
	} else if (extension == WAV_AUDIO_EXTENSION || extension == OGG_AUDIO_EXTENSION) {
		// Audio files
		AudioImporter::ImportAudio(filePath, jMeta);
	} else if (extension == PREFAB_EXTENSION) {
		// Prefab files
		PrefabImporter::ImportPrefab(filePath, jMeta);
	} else if (extension == STATE_MACHINE_EXTENSION) {
		// State Machine files
		StateMachineImporter::ImportStateMachine(filePath, jMeta);
	} else if (extension == ANIMATION_CLIP_EXTENSION) {
		// Clip files
		ClipImporter::ImportClip(filePath, jMeta);
	} else {
		validExtension = false;
	}
	return validExtension;
}

std::vector<UID> ModuleResources::ImportAssetResources(const char* filePath) {
	std::vector<UID> resources;

	// Return an empty list if the asset couldn't be found
	if (!App->files->Exists(filePath)) return resources;

	std::string metaFilePath = std::string(filePath) + META_EXTENSION;

	// Flag to keep track of the validity of the asset's meta file
	bool validMetaFile = App->files->Exists(metaFilePath.c_str());

	// Flag to check if the asset resources are created/imported successfully
	bool validResourceFiles = true;

	rapidjson::Document document;
	JsonValue jMeta(document, document);

	// Check meta file resources
	if (validMetaFile) {
		validMetaFile = ReadMetaFile(metaFilePath.c_str(), document);
		if (validMetaFile) {
			ValidateAssetResources(jMeta, validResourceFiles);
		}
	}

	// if resources are valid resources, reimport them or import them if needed
	if (validMetaFile && validResourceFiles) {
		ReimportResources(jMeta, filePath);
	} else {
		if (ImportAssetByExtension(jMeta, filePath)) {
			if (!validMetaFile) {
				jMeta[JSON_TAG_TIMESTAMP] = App->time->GetCurrentTimestamp();
				SaveMetaFile(metaFilePath.c_str(), document);
			}
		}
	}

	// Fill the temporal resource vector with the imported resources
	JsonValue jResources = jMeta[JSON_TAG_RESOURCES];
	for (unsigned i = 0; i < jResources.Size(); ++i) {
		resources.push_back(jResources[i][JSON_TAG_ID]);
	}

	return resources;
}

AssetFolder* ModuleResources::GetRootFolder() const {
	return rootFolder.get();
}

void ModuleResources::IncreaseReferenceCount(UID id) {
	if (id == 0) return;

	if (referenceCounts.find(id) != referenceCounts.end()) {
		referenceCounts[id] = referenceCounts[id] + 1;
	} else {
		Resource* resource = GetResource<Resource>(id);
		if (resource != nullptr) {
			resource->Load();
		}
		referenceCounts[id] = 1;
	}
}

void ModuleResources::DecreaseReferenceCount(UID id) {
	if (id == 0) return;

	if (referenceCounts.find(id) != referenceCounts.end()) {
		referenceCounts[id] = referenceCounts[id] - 1;
		if (referenceCounts[id] <= 0) {
			Resource* resource = GetResource<Resource>(id);
			if (resource != nullptr) {
				resource->Unload();
			}
			referenceCounts.erase(id);
		}
	}
}

unsigned ModuleResources::GetReferenceCount(UID id) const {
	auto it = referenceCounts.find(id);
	return it != referenceCounts.end() ? it->second : 0;
}

std::string ModuleResources::GenerateResourcePath(UID id) const {
	std::string strId = std::to_string(id);
	std::string metaFolder = std::string(LIBRARY_PATH "/") + strId.substr(0, 2);

	if (!App->files->Exists(metaFolder.c_str())) {
		App->files->CreateFolder(metaFolder.c_str());
	}

	return metaFolder + "/" + strId;
}

void ModuleResources::UpdateAsync() {
	while (!stopImportThread) {
		// Check if any asset file has been modified / deleted
		std::vector<UID> resourcesToRemove;
		std::vector<std::string> assetToReimport;

		for (std::pair<const UID, std::unique_ptr<Resource>>& entry : resources) {
			Resource* resource = entry.second.get();
			const std::string& resourceFilePath = resource->GetResourceFilePath();
			const std::string& assetFilePath = resource->GetAssetFilePath();
			std::string metaFilePath = assetFilePath + META_EXTENSION;

			// Check for deleted assets
			if (!App->files->Exists(assetFilePath.c_str())) {
				resourcesToRemove.push_back(entry.first);
				continue;
			}

			// Check for deleted, invalid or outdated meta files
			if (!App->files->Exists(metaFilePath.c_str())) {
				resourcesToRemove.push_back(entry.first);
				continue;
			} else {
				rapidjson::Document document;
				bool success = ReadMetaFile(metaFilePath.c_str(), document);
				JsonValue jMeta(document, document);

				if (success) {
#if !GAME
					long long timestamp = jMeta[JSON_TAG_TIMESTAMP];
					long long assetTimestamp = App->files->GetLocalFileModificationTime(assetFilePath.c_str());
					if (assetTimestamp > timestamp) {
						if (jMeta[JSON_TAG_RESOURCES].Size() > 1) {
							resourcesToRemove.push_back(entry.first);
						} else {
							resourcesToRemove.push_back(entry.first);
							if (std::find(assetToReimport.begin(), assetToReimport.end(), assetFilePath) == assetToReimport.end()) {
								assetToReimport.push_back(assetFilePath);
								jMeta[JSON_TAG_TIMESTAMP] = assetTimestamp;
								SaveMetaFile(metaFilePath.c_str(), document);
							}
						}
						continue;
					}
#endif
				} else {
					resourcesToRemove.push_back(entry.first);
					continue;
				}
			}
			// Check for deleted resources
			if (!App->files->Exists(resourceFilePath.c_str())) {
				if (std::find(assetToReimport.begin(), assetToReimport.end(), assetFilePath) == assetToReimport.end()) {
					assetToReimport.push_back(assetFilePath);
				}
			}
		}
		for (UID id : resourcesToRemove) {
			Resource* resource = resources[id].get();

			const std::string& assetFilePath = resource->GetAssetFilePath();
			const std::string& resourceFilePath = resource->GetResourceFilePath();
			std::string metaFilePath = assetFilePath + META_EXTENSION;
			if (App->files->Exists(metaFilePath.c_str()) && std::find(assetToReimport.begin(), assetToReimport.end(), assetFilePath) == assetToReimport.end()) {
				App->files->Erase(metaFilePath.c_str());
			}
			if (App->files->Exists(resourceFilePath.c_str())) {
				App->files->Erase(resourceFilePath.c_str());
			}

			resource->Unload();
			resources.erase(id);
		}
		for (std::string& assetFilePath : assetToReimport) {
			ImportAssetResources(assetFilePath.c_str());
		}

		// Check if there are any new assets and build cached folder structure
		AssetFolder* newFolder = new AssetFolder(ASSETS_PATH);
		CheckForNewAssetsRecursive(ASSETS_PATH, newFolder);

		TesseractEvent updateFoldersEv(TesseractEventType::UPDATE_FOLDERS);

		updateFoldersEv.Set<UpdateFoldersStruct>(newFolder);

		App->events->AddEvent(updateFoldersEv);

		App->events->AddEvent(TesseractEventType::RESOURCES_LOADED);

		std::this_thread::sleep_for(std::chrono::milliseconds(TIME_BETWEEN_RESOURCE_UPDATES_MS));
	}
}

void ModuleResources::CheckForNewAssetsRecursive(const char* path, AssetFolder* assetFolder) {
	for (std::string& file : App->files->GetFilesInFolder(path)) {
		std::string filePath = std::string(path) + "/" + file;
		std::string extension = FileDialog::GetFileExtension(file.c_str());
		if (App->files->IsDirectory(filePath.c_str())) {
			assetFolder->folders.push_back(AssetFolder(filePath.c_str()));
			CheckForNewAssetsRecursive(filePath.c_str(), &assetFolder->folders.back());
		} else if (extension != META_EXTENSION) {
			std::vector<UID>& resourceIds = ImportAssetResources(filePath.c_str());
			if (!resourceIds.empty()) {
				AssetFile assetFile(filePath.c_str());
				assetFile.resourceIds = std::move(resourceIds);
				assetFolder->files.push_back(std::move(assetFile));
			}
		}
	}
}

Resource* ModuleResources::CreateResourceByType(ResourceType type, const char* assetFilePath, UID id) {
	std::string resourceFilePath = GenerateResourcePath(id);
	Resource* resource = nullptr;
	switch (type) {
	case ResourceType::MATERIAL:
		resource = new ResourceMaterial(id, assetFilePath, resourceFilePath.c_str());
		break;
	case ResourceType::MESH:
		resource = new ResourceMesh(id, assetFilePath, resourceFilePath.c_str());
		break;
	case ResourceType::PREFAB:
		resource = new ResourcePrefab(id, assetFilePath, resourceFilePath.c_str());
		break;
	case ResourceType::SCENE:
		resource = new ResourceScene(id, assetFilePath, resourceFilePath.c_str());
		break;
	case ResourceType::SHADER:
		resource = new ResourceShader(id, assetFilePath, resourceFilePath.c_str());
		break;
	case ResourceType::TEXTURE:
		resource = new ResourceTexture(id, assetFilePath, resourceFilePath.c_str());
		break;
	case ResourceType::FONT:
		resource = new ResourceFont(id, assetFilePath, resourceFilePath.c_str());
		break;
	case ResourceType::SKYBOX:
		resource = new ResourceSkybox(id, assetFilePath, resourceFilePath.c_str());
		break;
	case ResourceType::SCRIPT:
		resource = new ResourceScript(id, assetFilePath, resourceFilePath.c_str());
		resource->Load();
		break;
	case ResourceType::ANIMATION:
		resource = new ResourceAnimation(id, assetFilePath, resourceFilePath.c_str());
		break;
	case ResourceType::STATE_MACHINE:
		resource = new ResourceStateMachine(id, assetFilePath, resourceFilePath.c_str());
		break;
	case ResourceType::CLIP:
		resource = new ResourceClip(id, assetFilePath, resourceFilePath.c_str());
		break;	
	case ResourceType::AUDIO:
		resource = new ResourceAudioClip(id, assetFilePath, resourceFilePath.c_str());
		break;
	default:
		LOG("Resource of type %i hasn't been registered in ModuleResources::CreateResourceByType.", (unsigned) type);
		assert(false); // ERROR: Resource type not registered
		return nullptr;
	}
	SendAddResourceEvent(resource);
	return resource;
}

void ModuleResources::SendAddResourceEvent(Resource* resource) {
	TesseractEvent addResourceEvent(TesseractEventType::ADD_RESOURCE);

	addResourceEvent.Set<AddResourceStruct>(resource);

	App->events->AddEvent(addResourceEvent);
}