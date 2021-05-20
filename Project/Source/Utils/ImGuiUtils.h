#pragma once

#include "Application.h"
#include "Modules/ModuleResources.h"
#include "Resources/ResourceTexture.h"

#include "imgui.h"
#include "GL/glew.h"
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

	ImGui::SameLine();
	std::string text = std::string("Id: ") + std::to_string(*target);
	ImGui::Text(text.c_str());
}

template<>
inline void ImGui::ResourceSlot<ResourceTexture>(const char* label, UID* target, std::function<void()> changeCallBack) {
	ImGui::Text(label);
	ImGui::BeginChildFrame(ImGui::GetID(target), ImVec2(200, 200));
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

	std::string text = std::string("Id: ") + std::to_string(*target);
	ImGui::Text(text.c_str());

	if (texture) {
		int width;
		int height;
		glGetTextureLevelParameteriv(texture->glTexture, 0, GL_TEXTURE_WIDTH, &width);
		glGetTextureLevelParameteriv(texture->glTexture, 0, GL_TEXTURE_HEIGHT, &height);
		ImGui::TextWrapped("Size: %d x %d", width, height);
	}
}