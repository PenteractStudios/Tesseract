#include "ComponentEventSystem.h"

#include "GameObject.h"
#include "Components/UI/ComponentSelectable.h"
#include "Application.h"
#include "Modules/ModuleUserInterface.h"
#include "Modules/ModuleInput.h"
#include "Modules/ModuleScene.h"
#include "Modules/ModuleTime.h"
#include "Scene.h"

#include "Utils/ImGuiUtils.h"
#include "Utils/Logging.h"

#include "Utils/Leaks.h"

#define JSON_TAG_FIRST_SELECTED_ID "FirstSelectedId"

ComponentEventSystem ::~ComponentEventSystem() {
	ComponentEventSystem* eventSystem = App->userInterface->GetCurrentEventSystem();
	if (eventSystem != nullptr) {
		if (eventSystem->GetID() == GetID()) {
			App->userInterface->SetCurrentEventSystem(0);
		}
	}
}

void ComponentEventSystem::Init() {
	App->userInterface->SetCurrentEventSystem(GetID());
	LOG("established %u as CurrentEventSystem", GetID());
	started = false;
}

void ComponentEventSystem::Update() {
	if (App->time->IsGameRunning()) {
		if (!started) {
			started = true;
			GameObject* objectToSelect = App->scene->scene->GetGameObject(firstSelectedId);
			if (objectToSelect) {
				ComponentSelectable* selectable = objectToSelect->GetComponent<ComponentSelectable>();

				SetSelected(selectable->GetID());
			}
		}
	}

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
			ComponentSelectable* currentSel = GetCurrentSelected();
			if (currentSel != nullptr) {
				ComponentSelectable* newSel = currentSel->FindSelectableOnDir(selectionDir);
				if (newSel != nullptr) {
					SetSelected(newSel->GetID());
				}
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

	ImGui::GameObjectSlot("First selected object", &firstSelectedId);
}

void ComponentEventSystem::Save(JsonValue jComponent) const {
	jComponent[JSON_TAG_FIRST_SELECTED_ID] = firstSelectedId;
}

void ComponentEventSystem::Load(JsonValue jComponent) {
	firstSelectedId = jComponent[JSON_TAG_FIRST_SELECTED_ID];
}

void ComponentEventSystem::OnEnable() {
	App->userInterface->SetCurrentEventSystem(GetID());
}

void ComponentEventSystem::OnDisable() {
	if (App->userInterface->GetCurrentEventSystem() == this) {
		App->userInterface->SetCurrentEventSystem(0);
	}
}

void ComponentEventSystem::SetSelected(ComponentSelectable* newSelectableComponent) {
	ComponentSelectable* currentSel = GetCurrentSelected();

	if (currentSel != nullptr) {
		currentSel->OnDeselect();
	}

	if (!newSelectableComponent) return;

	selectedId = newSelectableComponent->GetID();

	newSelectableComponent->OnSelect();
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

void ComponentEventSystem::EnteredPointerOnSelectable(ComponentSelectable* newHoveredComponent) {
	if (hoveredSelectableID != 0) {
		ComponentSelectable* selectableToUnHover = App->scene->scene->selectableComponents.Find(hoveredSelectableID);
		if (selectableToUnHover) selectableToUnHover->SetHovered(false);
	}

	hoveredSelectableID = newHoveredComponent->GetID();
}

void ComponentEventSystem::ExitedPointerOnSelectable(ComponentSelectable* newUnHoveredComponent) {
	if (hoveredSelectableID == newUnHoveredComponent->GetID()) {
		hoveredSelectableID = 0;
	}
}

ComponentSelectable* ComponentEventSystem::GetCurrentSelected() const {
	if (selectedId == 0) return nullptr;

	return GetOwner().scene->GetComponent<ComponentSelectable>(selectedId);
}

ComponentSelectable* ComponentEventSystem::GetCurrentlyHovered() const {
	if (hoveredSelectableID == 0) return nullptr;
	return GetOwner().scene->GetComponent<ComponentSelectable>(hoveredSelectableID);
}

void ComponentEventSystem::SetClickedGameObject(GameObject* clickedObj_) {
	clickedObj = clickedObj_;
}

GameObject* ComponentEventSystem::GetClickedGameObject() {
	return clickedObj;
}
