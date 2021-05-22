#pragma once

#include "Application.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleEditor.h"
#include "Resources/ResourceTexture.h"

#include "imgui.h"
#include "GL/glew.h"
#include "IconsFontAwesome5.h"
#include <functional>

namespace ImGui {
	template<typename T> void ResourceSlot(
		const char* label,
		UID* target,
		std::function<void()> changeCallBack = []() {});
	void GameObjectSlot(const char* label, UID* target);
} // namespace ImGui

template<typename T>
inline void ImGui::ResourceSlot(const char* label, UID* target, std::function<void()> changeCallBack) {
	ImGui::Text(label);

	if (ImGui::BeginTable(label, 2)) {
		ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed, 32);
		ImGui::TableNextColumn();

		ImGui::BeginChildFrame(ImGui::GetID(target), ImVec2(32, 32));
		ImGui::EndChildFrame();

		if (ImGui::BeginDragDropTarget()) {
			std::string payloadType = std::string("_RESOURCE_") + GetResourceTypeName(T::staticType);
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadType.c_str())) {
				UID newUID = *(UID*) payload->Data;
				UID oldUID = *target;
				if (oldUID != newUID) {
					if (oldUID != 0) {
						changeCallBack();
						App->resources->DecreaseReferenceCount(oldUID);
					}
					*target = newUID;
					App->resources->IncreaseReferenceCount(newUID);
				}
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::TableNextColumn();

		std::string resourceName = "None";
		T* resource = App->resources->GetResource<T>(*target);
		if (resource != nullptr) {
			resourceName = resource->GetName();
		}

		std::string resourceNameLabel = std::string("Name: ") + resourceName;
		ImGui::Text(resourceNameLabel.c_str());

		ImGui::TextUnformatted("Id:");
		ImGui::SameLine();
		ImGui::TextColored(App->editor->textColor, "%llu", *target);

		ImGui::EndTable();
	}
}

template<>
inline void ImGui::ResourceSlot<ResourceTexture>(const char* label, UID* target, std::function<void()> changeCallBack) {
	ImGui::BeginChildFrame(ImGui::GetID(target), ImVec2(32, 32));
	ResourceTexture* texture = App->resources->GetResource<ResourceTexture>(*target);
	if (texture) {
		ImTextureID texid = (ImTextureID) texture->glTexture;
		ImGui::Image(texid, ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
	}
	ImGui::EndChildFrame();

	if (ImGui::BeginDragDropTarget()) {
		std::string payloadType = std::string("_RESOURCE_") + GetResourceTypeName(ResourceTexture::staticType);
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadType.c_str())) {
			if (*target != 0) {
				changeCallBack();
				App->resources->DecreaseReferenceCount(*target);
			}
			*target = *(UID*) payload->Data;
			App->resources->IncreaseReferenceCount(*target);
		}

		ImGui::EndDragDropTarget();
	}

	ImGui::SameLine();
	ImGui::Text(label);

	std::string removeButton = std::string(ICON_FA_TIMES "##") + label;
	if (ImGui::Button(removeButton.c_str())) {
		if (*target != 0) {
			App->resources->DecreaseReferenceCount(*target);
			*target = 0;
		}
	}
	ImGui::SameLine();
	std::string resourceIdLabel = std::string("Id: ") + std::to_string(*target);
	ImGui::Text(resourceIdLabel.c_str());
}