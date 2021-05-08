#include "PanelProject.h"

#include "Globals.h"
#include "Application.h"
#include "Utils/Logging.h"
#include "Utils/FileDialog.h"
#include "GameObject.h"
#include "Components/ComponentTransform.h"
#include "Components/ComponentBoundingBox.h"
#include "Modules/ModuleInput.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleCamera.h"
#include "Modules/ModuleRender.h"
#include "Modules/ModuleTime.h"
#include "Modules/ModuleResources.h"
#include "Utils/AssetFile.h"
#include "Resources/Resource.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome5.h"
#include "Math/float3x3.h"
#include "Math/float2.h"
#include "Geometry/OBB.h"
#include "SDL_mouse.h"
#include "SDL_scancode.h"
#include <algorithm>

#include "Utils/Leaks.h"

PanelProject::PanelProject()
	: Panel("Project", true) {}

void PanelProject::Update() {
	ImGui::SetNextWindowDockID(App->editor->dockDownId, ImGuiCond_FirstUseEver);
	std::string windowName = std::string(ICON_FA_FOLDER " ") + name;
	if (ImGui::Begin(windowName.c_str(), &enabled)) {
		AssetFolder* rootFolder = App->resources->GetRootFolder();
		if (rootFolder != nullptr) {
			ImGui::Columns(3);
			UpdateFoldersRecursive(*rootFolder);
			ImGui::NextColumn();
			UpdateAssetsRecursive(*rootFolder);
			ImGui::NextColumn();
			UpdateResourcesRecursive(*rootFolder);
		}
	}
	ImGui::End();
}

void PanelProject::UpdateFoldersRecursive(const AssetFolder& folder) {
	std::string name = std::string(ICON_FA_FOLDER " ") + FileDialog::GetFileName(folder.path.c_str());

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
	if (folder.folders.empty()) flags |= ImGuiTreeNodeFlags_Leaf;
	bool isSelected = selectedFolder == folder.path;
	if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

	bool open = ImGui::TreeNodeEx(folder.path.c_str(), flags, name.c_str());

	if (ImGui::IsItemClicked()) {
		selectedFolder = folder.path;
	}

	if (open) {
		for (const AssetFolder& childFolder : folder.folders) {
			UpdateFoldersRecursive(childFolder);
		}
		ImGui::TreePop();
	}
}

void PanelProject::UpdateAssetsRecursive(const AssetFolder& folder) {
	if (folder.path == selectedFolder) {
		for (const AssetFile& assetFile : folder.files) {
			std::string name = std::string(ICON_FA_BOX " ") + FileDialog::GetFileNameAndExtension(assetFile.path.c_str());

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
			bool isSelected = selectedAsset == assetFile.path;
			if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

			bool open = ImGui::TreeNodeEx(assetFile.path.c_str(), flags, name.c_str());

			if (ImGui::IsItemClicked()) {
				selectedAsset = assetFile.path;
			}

			if (open) {
				ImGui::TreePop();
			}
		}
	} else {
		for (const AssetFolder& childFolder : folder.folders) {
			UpdateAssetsRecursive(childFolder);
		}
	}
}

void PanelProject::UpdateResourcesRecursive(const AssetFolder& folder) {
	if (folder.path == selectedFolder) {
		for (const AssetFile& assetFile : folder.files) {
			if (assetFile.path != selectedAsset) continue;

			for (UID resourceId : assetFile.resourceIds) {
				Resource* resource = App->resources->GetResource<Resource>(resourceId);
				if (resource == nullptr) continue;

				std::string resourceName = std::to_string(resourceId);
				std::string resourceTypeName = GetResourceTypeName(resource->GetType());
				std::string name = std::string(ICON_FA_FILE " ") + "[" + resourceTypeName + "] " + resourceName.c_str();

				ImGuiSelectableFlags flags = ImGuiSelectableFlags_None;
				ImGui::PushID(resourceName.c_str());
				if (ImGui::Selectable(name.c_str(), flags)) {
					App->editor->selectedResource = resource;
				}
				ImGui::PopID();

				if (ImGui::BeginDragDropSource()) {
					std::string payloadType = std::string("_RESOURCE_") + resourceTypeName;
					ImGui::SetDragDropPayload(payloadType.c_str(), &resourceId, sizeof(UID));
					ImGui::EndDragDropSource();
				}
			}
		}
	} else {
		for (const AssetFolder& childFolder : folder.folders) {
			UpdateResourcesRecursive(childFolder);
		}
	}
}
