#pragma once

#include "Application.h"
#include "Modules/ModuleResources.h"

#include "imgui.h"
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