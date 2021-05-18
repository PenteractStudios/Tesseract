#include "ImGuiUtils.h"

#include "GameObject.h"
#include "Application.h"
#include "Modules/ModuleScene.h"
#include "Scene.h"

void ImGui::GameObjectSlot(const char* label, UID* target) {
	ImGui::Text(label);
	ImGui::BeginChildFrame(ImGui::GetID(target), ImVec2(32, 32));
	ImGui::EndChildFrame();

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_HIERARCHY")) {
			*target = *(UID*) payload->Data;
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::SameLine();
	GameObject* targetGameObject = App->scene->scene->GetGameObject(*target);
	std::string idText = std::string("Id: ") + std::to_string(*target);
	ImGui::Text(idText.c_str());
	std::string nameText = std::string("GameObject: ") + (targetGameObject ? targetGameObject->name : "None");
	ImGui::Text(nameText.c_str());
}