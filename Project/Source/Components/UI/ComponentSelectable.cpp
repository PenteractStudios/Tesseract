#include "ComponentSelectable.h"

#include "Application.h"
#include "GameObject.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleUserInterface.h"
#include "Modules/ModuleTime.h"
#include "Components/UI/ComponentTransform2D.h"
#include "Components/UI/ComponentEventSystem.h"
#include "Components/UI/ComponentButton.h"
#include "Components/UI/ComponentToggle.h"
#include "Components/UI/ComponentSlider.h"
#include "Utils/ImGuiUtils.h"

#include "Utils/Leaks.h"

#define JSON_TAG_COLOR_HOVERED "ColorHover"
#define JSON_TAG_COLOR_DISABLED "ColorDisable"
#define JSON_TAG_COLOR_SELECTED "ColorDisable"

#define JSON_TAG_INTERACTABLE "Interactable"
#define JSON_TAG_ON_AXIS_DOWN "OnAxisDown"
#define JSON_TAG_ON_AXIS_UP "OnAxisUp"
#define JSON_TAG_ON_AXIS_RIGHT "OnAxisRight"
#define JSON_TAG_ON_AXIS_LEFT "OnAxisLeft"
#define JSON_TAG_TRANSITION_TYPE "TransitionType"
#define JSON_TAG_NAVIGATION_TYPE "NavigationType"

#define JSON_TAG_SELECTABLE_TYPE "SelectableType"

ComponentSelectable::~ComponentSelectable() {
	ComponentEventSystem* eventSystem = App->userInterface->GetCurrentEventSystem();
	if (eventSystem) {
		if (eventSystem->GetCurrentSelected() == this) {
			eventSystem->SetSelected(nullptr);
		}
	}
}

bool ComponentSelectable::IsInteractable() const {
	return interactable;
}

void ComponentSelectable::SetInteractable(bool interactable_) {
	interactable = interactable_;
}

ComponentSelectable* ComponentSelectable::FindSelectableOnDir(float2 dir) {
	if (!App->userInterface->GetCurrentEventSystem()) return nullptr;

	switch (navigationType) {
	case NavigationType::AUTOMATIC: {
		ComponentSelectable* bestCandidate = nullptr;
		float minDistance = FLT_MAX;
		float3 thisPos = this->GetOwner().GetComponent<ComponentTransform2D>()->GetScreenPosition();

		// Get Gameobjects with the same parent
		for (ComponentSelectable* selectable : App->userInterface->GetCurrentEventSystem()->activeSelectableComponents) {
			if (!selectable) continue;
			
			GameObject selectableObject = selectable->GetOwner();
			if (!selectableObject.IsActive()) continue;
			// Get relative direction and distance to this Element
			float3 direction = selectableObject.GetComponent<ComponentTransform2D>()->GetScreenPosition() - thisPos;
			float distance = direction.LengthSq();

			// Compare best candidate
			if (distance < minDistance) {
				if (dir.x > 0.6f && direction.x > 0.6f) {
					bestCandidate = selectable;
					minDistance = distance;
				} else if (dir.x < -0.6f && direction.x < -0.6f) {
					bestCandidate = selectable;
					minDistance = distance;
				} else if (dir.y > 0.6f && direction.y > 0.6f) {
					bestCandidate = selectable;
					minDistance = distance;
				} else if (dir.y < -0.6f && direction.y < -0.6f) {
					bestCandidate = selectable;
					minDistance = distance;
				}
			}
		}
		return bestCandidate;
		break;
	}
	case NavigationType::MANUAL: {
		GameObject* onAxisObj = nullptr;

		if (dir.x > 0.6f) {
			onAxisObj = GetOwner().scene->GetGameObject(onAxisRight);
		} else if (dir.x < -0.6f) {
			onAxisObj = GetOwner().scene->GetGameObject(onAxisLeft);
		} else if (dir.y > 0.6f) {
			onAxisObj = GetOwner().scene->GetGameObject(onAxisUp);
		} else if (dir.y < -0.6f) {
			onAxisObj = GetOwner().scene->GetGameObject(onAxisDown);
		}

		return onAxisObj == nullptr ? nullptr : onAxisObj->GetComponent<ComponentSelectable>();
	}
	default:
		break;
	}

	return nullptr;
}

void ComponentSelectable::OnSelect() {
	selected = true;
}

void ComponentSelectable::OnDeselect() {
	selected = false;
}

void ComponentSelectable::Start() {
	if (App->userInterface->GetCurrentEventSystem()) {
		App->userInterface->GetCurrentEventSystem()->activeSelectableComponents.push_back(this);
	}
}

void ComponentSelectable::Update() {
}

void ComponentSelectable::OnEditorUpdate() {
	bool isInteractable = interactable;
	if (ImGui::Checkbox("Interactable", &isInteractable)) {
		SetInteractable(isInteractable);
	}

	// Navigation Type combo box
	const char* navigationTypeItems[] = {"None", "Automatic", "Manual"};
	const char* navigationCurrent = navigationTypeItems[(int) navigationType];
	if (ImGui::BeginCombo("Navigation Mode", navigationCurrent)) {
		for (int n = 0; n < IM_ARRAYSIZE(navigationTypeItems); ++n) {
			bool isSelected = (navigationCurrent == navigationTypeItems[n]);
			if (ImGui::Selectable(navigationTypeItems[n], isSelected)) {
				navigationType = NavigationType(n);
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if (navigationType == NavigationType::MANUAL) {
		ImGui::GameObjectSlot("Navigation Up", &onAxisUp);
		ImGui::GameObjectSlot("Navigation Down", &onAxisDown);
		ImGui::GameObjectSlot("Navigation Left", &onAxisLeft);
		ImGui::GameObjectSlot("Navigation Right", &onAxisRight);
	}

	//Transition Type combo box
	const char* transitionTypeItems[] = {"None", "Color Transition", "Animation"};
	const char* transitionCurrent = transitionTypeItems[(int) transitionType];
	if (ImGui::BeginCombo("Transition", transitionCurrent)) {
		for (int n = 0; n < IM_ARRAYSIZE(transitionTypeItems); ++n) {
			bool isSelected = (transitionCurrent == transitionTypeItems[n]);
			if (ImGui::Selectable(transitionTypeItems[n], isSelected)) {
				transitionType = TransitionType(n);
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::ColorEdit4("Disable Color##", colorDisabled.ptr());
	ImGui::ColorEdit4("Hovered Color##", colorHovered.ptr());
	ImGui::ColorEdit4("Selected Color##", colorSelected.ptr());

	//TODO Drag/Drop for manual navigation references (4 ComponentSelectable pointers)
}

void ComponentSelectable::OnEnable() {
	if (!App->time->IsGameRunning()) return; //This line to prevent OnEnable from loading scenes
	ComponentEventSystem* evSys = App->userInterface->GetCurrentEventSystem();
	if (evSys) evSys->activeSelectableComponents.push_back(this);
}

void ComponentSelectable::OnDisable() {
	ComponentEventSystem* evSys = App->userInterface->GetCurrentEventSystem();
	if (evSys != nullptr) {
		if (selected) {
			evSys->SetSelected(nullptr);
		}
		if (hovered) {
			hovered = false;
			evSys->ExitedPointerOnSelectable(this);
		}
		evSys->activeSelectableComponents.remove(this);
	}
}

void ComponentSelectable::OnPointerEnter() {
	ComponentEventSystem* evSys = App->userInterface->GetCurrentEventSystem();
	if (evSys != nullptr && GetOwner().IsActive()) {
		hovered = true;
		evSys->EnteredPointerOnSelectable(this);
	}
}

const float4 ComponentSelectable::GetDisabledColor() const {
	return colorDisabled;
}

void ComponentSelectable::OnPointerExit() {
	ComponentEventSystem* evSys = App->userInterface->GetCurrentEventSystem();
	if (evSys != nullptr && GetOwner().IsActive()) {
		hovered = false;
		evSys->ExitedPointerOnSelectable(this);
	}
}

bool ComponentSelectable::IsHovered() const {
	return hovered;
}

void ComponentSelectable::SetHovered(bool hovered_) {
	hovered = hovered_;
}

bool ComponentSelectable::IsSelected() const {
	return selected;
}

void ComponentSelectable::Save(JsonValue jsonVal) const {
	JsonValue jColorHover = jsonVal[JSON_TAG_COLOR_HOVERED];
	jColorHover[0] = colorHovered.x;
	jColorHover[1] = colorHovered.y;
	jColorHover[2] = colorHovered.z;
	jColorHover[3] = colorHovered.w;

	JsonValue jColorDisable = jsonVal[JSON_TAG_COLOR_DISABLED];
	jColorDisable[0] = colorDisabled.x;
	jColorDisable[1] = colorDisabled.y;
	jColorDisable[2] = colorDisabled.z;
	jColorDisable[3] = colorDisabled.w;

	JsonValue jColorSelected = jsonVal[JSON_TAG_COLOR_SELECTED];
	jColorSelected[0] = colorSelected.x;
	jColorSelected[1] = colorSelected.y;
	jColorSelected[2] = colorSelected.z;
	jColorSelected[3] = colorSelected.w;

	//TODO RESEARCH WHY THIS RETURNS ERROR
	JsonValue jInteractable = jsonVal[JSON_TAG_INTERACTABLE];
	jInteractable = interactable;

	JsonValue jNavigationType = jsonVal[JSON_TAG_NAVIGATION_TYPE];
	jNavigationType = (int) navigationType;

	JsonValue jTransitiontype = jsonVal[JSON_TAG_TRANSITION_TYPE];
	jTransitiontype = (int) transitionType;

	JsonValue jSelectableType = jsonVal[JSON_TAG_SELECTABLE_TYPE];
	jSelectableType = (int) selectableType;

	jsonVal[JSON_TAG_ON_AXIS_DOWN] = onAxisDown;

	jsonVal[JSON_TAG_ON_AXIS_UP] = onAxisUp;

	jsonVal[JSON_TAG_ON_AXIS_RIGHT] = onAxisRight;

	jsonVal[JSON_TAG_ON_AXIS_LEFT] = onAxisLeft;
}

void ComponentSelectable::Load(JsonValue jsonVal) {
	JsonValue jColorHover = jsonVal[JSON_TAG_COLOR_HOVERED];
	colorHovered.Set(jColorHover[0], jColorHover[1], jColorHover[2], jColorHover[3]);

	JsonValue jColorDisabled = jsonVal[JSON_TAG_COLOR_DISABLED];
	colorDisabled.Set(jColorDisabled[0], jColorDisabled[1], jColorDisabled[2], jColorDisabled[3]);

	JsonValue jColorSelected = jsonVal[JSON_TAG_COLOR_SELECTED];
	colorSelected.Set(jColorSelected[0], jColorSelected[1], jColorSelected[2], jColorSelected[3]);

	JsonValue jInteractable = jsonVal[JSON_TAG_INTERACTABLE];
	interactable = jInteractable;

	JsonValue jNavigationType = jsonVal[JSON_TAG_NAVIGATION_TYPE];
	navigationType = (NavigationType)((int) jsonVal[JSON_TAG_NAVIGATION_TYPE]);

	JsonValue jTransitionType = jsonVal[JSON_TAG_TRANSITION_TYPE];
	transitionType = (TransitionType)((int) jsonVal[JSON_TAG_TRANSITION_TYPE]);

	JsonValue jSelectableType = jsonVal[JSON_TAG_SELECTABLE_TYPE];
	selectableType = (ComponentType)((int) jsonVal[JSON_TAG_SELECTABLE_TYPE]);

	onAxisDown = jsonVal[JSON_TAG_ON_AXIS_DOWN];

	onAxisUp = jsonVal[JSON_TAG_ON_AXIS_UP];

	onAxisRight = jsonVal[JSON_TAG_ON_AXIS_RIGHT];

	onAxisLeft = jsonVal[JSON_TAG_ON_AXIS_LEFT];
}

const float4 ComponentSelectable::GetHoverColor() const {
	return colorHovered;
}

const float4 ComponentSelectable::GetSelectedColor() const {
	return colorSelected;
}

ComponentSelectable::TransitionType ComponentSelectable::GetTransitionType() const {
	return transitionType;
}

bool ComponentSelectable::IsClicked() const {
	UID toBeClicked = 0;
	ComponentType typeToPress = ComponentType::UNKNOWN;

	std::vector<Component*>::const_iterator it = GetOwner().components.begin();
	while (toBeClicked == 0 && it != GetOwner().components.end()) {
		if ((*it)->GetType() == ComponentType::BUTTON || (*it)->GetType() == ComponentType::TOGGLE || (*it)->GetType() == ComponentType::SLIDER) {
			toBeClicked = (*it)->GetID();
			typeToPress = (*it)->GetType();
		} else {
			++it;
		}
	}
	if (toBeClicked != 0) {
		Component* componentToPress = nullptr;
		switch (typeToPress) {
		case ComponentType::BUTTON:
			componentToPress = GetOwner().GetComponent<ComponentButton>();
			return static_cast<ComponentButton*>(componentToPress)->IsClicked();
		case ComponentType::TOGGLE:
			componentToPress = GetOwner().GetComponent<ComponentToggle>();
			return static_cast<ComponentToggle*>(componentToPress)->IsClicked();
		case ComponentType::SLIDER:
			componentToPress = GetOwner().GetComponent<ComponentSlider>();
			return static_cast<ComponentSlider*>(componentToPress)->IsClicked();
		default:
			return false;
		}
	}
	return false;
}

void ComponentSelectable::TryToClickOn(bool internalClick) const {
	if (!IsActive() || !IsInteractable()) return; //Cannot be clicked on if not active/not interactable
	UID toBeClicked = 0;
	ComponentType typeToPress = ComponentType::UNKNOWN;

	std::vector<Component*>::const_iterator it = GetOwner().components.begin();
	while (toBeClicked == 0 && it != GetOwner().components.end()) {
		if ((*it)->GetType() == ComponentType::BUTTON || (*it)->GetType() == ComponentType::TOGGLE || (*it)->GetType() == ComponentType::SLIDER) {
			toBeClicked = (*it)->GetID();
			typeToPress = (*it)->GetType();
		} else {
			++it;
		}
	}

	if (toBeClicked != 0) {
		Component* componentToPress = nullptr;
		switch (typeToPress) {
		case ComponentType::BUTTON:
			componentToPress = GetOwner().GetComponent<ComponentButton>();
			if (internalClick) {
				static_cast<ComponentButton*>(componentToPress)->OnClickedInternal();
			} else {
				static_cast<ComponentButton*>(componentToPress)->OnClicked();
			}
			break;
		case ComponentType::TOGGLE:
			componentToPress = GetOwner().GetComponent<ComponentToggle>();
			if (internalClick) {
				static_cast<ComponentToggle*>(componentToPress)->OnClickedInternal();
			} else {
				static_cast<ComponentToggle*>(componentToPress)->OnClicked();
			}
			break;
		case ComponentType::SLIDER:
			componentToPress = GetOwner().GetComponent<ComponentSlider>();
			if (internalClick) {
				static_cast<ComponentSlider*>(componentToPress)->OnClickedInternal();
			} else {
				static_cast<ComponentSlider*>(componentToPress)->OnClicked();
			}
			break;
		default:
			assert("This is not supposed to ever happen");
			break;
		}
	}
}

Component* ComponentSelectable::GetSelectableComponent() {
	switch (selectableType) {
	case ComponentType::BUTTON:
		return GetOwner().GetComponent<ComponentButton>();
	case ComponentType::TOGGLE:
		return GetOwner().GetComponent<ComponentToggle>();
	case ComponentType::SLIDER:
		return GetOwner().GetComponent<ComponentSlider>();
	default:
		return nullptr;
	}
}

void ComponentSelectable::SetSelectableType(ComponentType type_) {
	selectableType = type_;
}

bool ComponentSelectable::CanBeRemoved() const {
	return !(GetOwner().GetComponent<ComponentButton>() || GetOwner().GetComponent<ComponentToggle>() || GetOwner().GetComponent<ComponentSlider>());
}