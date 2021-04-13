#include "ComponentEventSystem.h"

#include "Event.h"
#include "GameObject.h"
#include "Components/UI/ComponentSelectable.h"
#include "Application.h"
#include "Modules/ModuleUserInterface.h"
#include "Modules/ModuleInput.h"

#include "imgui.h"
#include "Utils/Logging.h"

#include "Utils/Leaks.h"

#define JSON_TAG_FIRST_SELECTED_ID "FirstSelectedId"

ComponentEventSystem ::~ComponentEventSystem() {
}

void ComponentEventSystem::Init() {
	App->userInterface->SetCurrentEventSystem(this);
	SetSelected(firstSelectedId);
}

void ComponentEventSystem::Update() {
	float2 selectionDir = float2(0.f, 0.f);
	bool keyPressed = false;
	if (App->input->GetKey(SDL_SCANCODE_UP) == KS_DOWN) {
		selectionDir = float2(0.f, 1.f);
		keyPressed = true;
	}
	if (App->input->GetKey(SDL_SCANCODE_DOWN) == KS_DOWN) {
		selectionDir = float2(0.f, -1.f);
		keyPressed = true;
	}
	if (App->input->GetKey(SDL_SCANCODE_LEFT) == KS_DOWN) {
		selectionDir = float2(-1.f, 0.f);
		keyPressed = true;
	}
	if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KS_DOWN) {
		selectionDir = float2(1.f, 0.f);
		keyPressed = true;
	}

	if (keyPressed) {
		if (selectedId != 0) {
			ComponentSelectable* newSel = GetCurrentSelected()->FindSelectableOnDir(selectionDir);
			if (newSel) {
				SetSelected(newSel->GetID());
			}
		}
	}
}

void ComponentEventSystem::OnEditorUpdate() {
	ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Current Selected:");

	if (selectedId != 0) {
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), GetCurrentSelected()->GetOwner().name.c_str());
	}

	//TO DO FIRST SELECTED RESOURCE SLOT <GAMEOBJECT>
}

void ComponentEventSystem::Save(JsonValue jComponent) const {
	jComponent[JSON_TAG_FIRST_SELECTED_ID] = firstSelectedId;
}

void ComponentEventSystem::Load(JsonValue jComponent) {
	firstSelectedId = jComponent[JSON_TAG_FIRST_SELECTED_ID];
}

void ComponentEventSystem::OnEnable() {
	App->userInterface->SetCurrentEventSystem(this);
}

void ComponentEventSystem::OnDisable() {
	if (App->userInterface->GetCurrentEventSystem() == this) {
		App->userInterface->SetCurrentEventSystem(nullptr);
	}
}

void ComponentEventSystem::SetSelected(UID newSelectableComponentId) {
	ComponentSelectable* currentSel = GetCurrentSelected();
	if (currentSel != nullptr) {
		currentSel->OnDeselect();
	}
	selectedId = newSelectableComponentId;

	if (newSelectableComponentId != 0) {
		ComponentSelectable* newSelectableComponent = GetOwner().scene->GetComponent<ComponentSelectable>(newSelectableComponentId);

		if (newSelectableComponent != nullptr) {
			newSelectableComponent->OnSelect();
		}
	}
}

void ComponentEventSystem::DuplicateComponent(GameObject& owner) {
	ComponentEventSystem* component = owner.CreateComponent<ComponentEventSystem>();
	component->firstSelectedId = firstSelectedId;
}

void ComponentEventSystem::EnteredPointerOnSelectable(ComponentSelectable* newHoveredComponent) {
	for (std::vector<UID>::const_iterator it = hoveredSelectableIds.begin(); it != hoveredSelectableIds.end(); ++it) {
		if ((*it) == newHoveredComponent->GetID()) {
			return;
		}
	}
	hoveredSelectableIds.push_back(newHoveredComponent->GetID());
}

void ComponentEventSystem::ExitedPointerOnSelectable(ComponentSelectable* newUnHoveredComponent) {
	std::vector<UID>::iterator itToRemove;
	ComponentSelectable* selectableToRemove = nullptr;
	for (std::vector<UID>::iterator it = hoveredSelectableIds.begin(); it != hoveredSelectableIds.end() && selectableToRemove == nullptr; ++it) {
		if ((*it) == newUnHoveredComponent->GetID()) {
			itToRemove = it;
			selectableToRemove = GetOwner().scene->GetComponent<ComponentSelectable>(*it);
		}
	}

	if (selectableToRemove != nullptr) {
		hoveredSelectableIds.erase(itToRemove);
	}
}

ComponentSelectable* ComponentEventSystem::GetCurrentSelected() const {
	if (selectedId == 0) return nullptr;

	return GetOwner().scene->GetComponent<ComponentSelectable>(selectedId);
}

ComponentSelectable* ComponentEventSystem::GetCurrentlyHovered() const {
	if (hoveredSelectableIds.size() == 0) return nullptr;
	return GetOwner().scene->GetComponent<ComponentSelectable>(hoveredSelectableIds.front());
}

void ComponentEventSystem::SetClickedGameObject(GameObject* clickedObj_) {
	clickedObj = clickedObj_;
}

GameObject* ComponentEventSystem::GetClickedGameObject() {
	return clickedObj;
}
