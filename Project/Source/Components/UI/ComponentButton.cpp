#include "ComponentButton.h"

#include "Application.h"
#include "imgui.h"
#include "GameObject.h"
#include "Modules/ModuleInput.h"
#include "Modules/ModuleUserInterface.h"
#include "Components/UI/ComponentSelectable.h"
#include "Resources/ResourceScript.h"
#include "Utils/Logging.h"
#include "Utils/Leaks.h"

#define JSON_TAG_COLOR_HOVER "ColorHover"
#define JSON_TAG_COLOR_CLICK "ColorClick"
#define JSON_TAG_COLOR_DISABLE "ColorDisable"
#define JSON_TAG_CLICKED "Clicked"

void ComponentButton::Init() {
}

void ComponentButton::OnEditorUpdate() {
	ImGui::ColorEdit4("Click Color##", colorClicked.ptr());
}

void ComponentButton::Save(JsonValue jComponent) const {
	jComponent[JSON_TAG_CLICKED] = clicked;

	JsonValue jColorClick = jComponent[JSON_TAG_COLOR_CLICK];
	jColorClick[0] = colorClicked.x;
	jColorClick[1] = colorClicked.y;
	jColorClick[2] = colorClicked.z;
	jColorClick[3] = colorClicked.w;
}

void ComponentButton::Load(JsonValue jComponent) {
	clicked = jComponent[JSON_TAG_CLICKED];

	JsonValue jColorClick = jComponent[JSON_TAG_COLOR_CLICK];
	colorClicked.Set(jColorClick[0], jColorClick[1], jColorClick[2], jColorClick[3]);
}

void ComponentButton::OnClickedInternal() {
	clicked = true;
	App->userInterface->GetCurrentEventSystem()->SetSelected(GetOwner().GetComponent<ComponentSelectable>()->GetID());
}

void ComponentButton::OnClicked() {
	std::vector<ComponentScript*> scriptComponents = GetOwner().GetComponents<ComponentScript>();
	for (ComponentScript* scriptComponent : scriptComponents) {
		Resource* scriptResource = App->resources->GetResource(scriptComponent->GetScriptID());
		if (scriptResource != nullptr) {
			Script* script = ((ResourceScript*) scriptResource)->script;
			if (script != nullptr) {
				script->OnButtonClick();
			}
		}
	}
}

bool ComponentButton::IsClicked() const {
	return clicked;
}

void ComponentButton::SetClicked(bool clicked_) {
	clicked = clicked_;
}

const float4& ComponentButton::GetClickColor() const {
	return colorClicked;
}

const float4& ComponentButton::GetTintColor() const {
	if (!IsActive()) return float4::one;

	ComponentSelectable* sel = GetOwner().GetComponent<ComponentSelectable>();

	if (!sel) return float4::one;

	if (sel->GetTransitionType() == ComponentSelectable::TransitionType::COLOR_CHANGE) {
		if (!sel->IsInteractable()) {
			return sel->GetDisabledColor();
		} else if (IsClicked()) {
			return colorClicked;
		} else if (sel->IsSelected()) {
			return sel->GetSelectedColor();
		} else if (sel->IsHovered()) {
			return sel->GetHoverColor();
		}
	}

	return float4::one;
}

void ComponentButton::DuplicateComponent(GameObject& owner) {
	ComponentButton* component = owner.CreateComponent<ComponentButton>();
	component->colorClicked = colorClicked;
}

void ComponentButton::Update() {
	if (clicked) {
		if (!App->input->GetMouseButton(1)) {
			clicked = false;
		}
	}
}
