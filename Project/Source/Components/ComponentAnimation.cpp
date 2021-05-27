#include "ComponentAnimation.h"

#include "Application.h"
#include "Transition.h"
#include "GameObject.h"
#include "AnimationInterpolation.h"
#include "AnimationController.h"
#include "Resources/ResourceAnimation.h"
#include "Resources/ResourceClip.h"
#include "Components/ComponentType.h"
#include "Components/ComponentTransform.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleTime.h"
#include "Modules/ModuleInput.h"
#include "Modules/ModuleEvents.h"
#include "Utils/UID.h"
#include "Utils/Logging.h"
#include "Utils/ImGuiUtils.h"
#include "StateMachineEnum.h"

#include <algorithm> // std::find

#include "Utils/Leaks.h"

#define JSON_TAG_LOOP "Controller"
#define JSON_TAG_ANIMATION_ID "AnimationId"
#define JSON_TAG_STATE_MACHINE_PRINCIPAL_ID "StateMachinePrincipalId"
#define JSON_TAG_STATE_MACHINE_SECONDARY_ID "StateMachineSecondaryId"
#define JSON_TAG_CLIP "Clip"

void ComponentAnimation::Update() {
	if (!currentStatePrincipal) { //Checking if there is no state machine
		LoadResourceStateMachine(stateMachineResourceUIDPrincipal, StateMachineEnum::PRIMARY);
	}
	if (!currentStateSecondary) { //Checking if there is no state machine
		LoadResourceStateMachine(stateMachineResourceUIDSecondary, StateMachineEnum::SECONDARY);
	}

	OnUpdate();
}

void ComponentAnimation::OnEditorUpdate() {
	if (ImGui::Checkbox("Active", &active)) {
		if (GetOwner().IsActive()) {
			if (active) {
				Enable();
			} else {
				Disable();
			}
		}
	}
	ImGui::Separator();

	ImGui::TextColored(App->editor->titleColor, "Animation");

	// Principal
	UID oldStateMachineUID = stateMachineResourceUIDPrincipal;
	ImGui::ResourceSlot<ResourceStateMachine>("State Machine Principal", &stateMachineResourceUIDPrincipal);
	if (oldStateMachineUID != stateMachineResourceUIDPrincipal) {
		ResourceStateMachine* resourceStateMachine = App->resources->GetResource<ResourceStateMachine>(stateMachineResourceUIDPrincipal);
		if (resourceStateMachine) {
			currentStatePrincipal = &resourceStateMachine->initialState;
		} else {
			currentStatePrincipal = nullptr;
		}
	}

	// Secondary
	UID oldStateMachineUID = stateMachineResourceUIDSecondary;
	ImGui::ResourceSlot<ResourceStateMachine>("State Machine Secondary", &stateMachineResourceUIDSecondary);
	if (oldStateMachineUID != stateMachineResourceUIDSecondary) {
		ResourceStateMachine* resourceStateMachine = App->resources->GetResource<ResourceStateMachine>(stateMachineResourceUIDSecondary);
		if (resourceStateMachine) {
			currentStateSecondary = &resourceStateMachine->initialState;
		} else {
			currentStateSecondary = nullptr;
		}
	}
}

void ComponentAnimation::Save(JsonValue jComponent) const {
	jComponent[JSON_TAG_STATE_MACHINE_PRINCIPAL_ID] = stateMachineResourceUIDPrincipal;
	jComponent[JSON_TAG_STATE_MACHINE_SECONDARY_ID] = stateMachineResourceUIDSecondary;
}

void ComponentAnimation::Load(JsonValue jComponent) {
	stateMachineResourceUIDPrincipal = jComponent[JSON_TAG_STATE_MACHINE_PRINCIPAL_ID];
	stateMachineResourceUIDSecondary = jComponent[JSON_TAG_STATE_MACHINE_SECONDARY_ID];

	if (stateMachineResourceUIDPrincipal != 0) App->resources->IncreaseReferenceCount(stateMachineResourceUIDPrincipal);
	if (stateMachineResourceUIDSecondary != 0) App->resources->IncreaseReferenceCount(stateMachineResourceUIDSecondary);

	LoadResourceStateMachine(stateMachineResourceUIDPrincipal, StateMachineEnum::PRIMARY);
	LoadResourceStateMachine(stateMachineResourceUIDSecondary, StateMachineEnum::SECONDARY);

}

void ComponentAnimation::OnUpdate() {
	// Update gameobjects matrix
	GameObject* rootBone = GetOwner().GetRootBone();

	UpdateAnimations(rootBone);

	if (currentState) {
		ResourceClip* currentClip = App->resources->GetResource<ResourceClip>(currentState->clipUid);
		currentState->currentTime += App->time->GetDeltaTime() * currentClip->speed;
	}
}

void ComponentAnimation::SendTrigger(const std::string& trigger) {
	ResourceStateMachine* resourceStateMachine = App->resources->GetResource<ResourceStateMachine>(stateMachineResourceUID);

	Transition* transition = resourceStateMachine->FindTransitionGivenName(trigger);
	if (transition != nullptr) {
		if(transition->source.id == currentState->id){
			if (animationInterpolations.size() == 0) {
				animationInterpolations.push_front(AnimationInterpolation(&transition->source, currentState->currentTime, 0, transition->interpolationDuration));
			}

			animationInterpolations.push_front(AnimationInterpolation(&transition->target, 0, 0, transition->interpolationDuration));
			currentState = &transition->target;
		} 
		else {
			LOG("Warning: transition target from %s to %s, and current state is %s ", transition->source.name.c_str(), transition->target.name.c_str(), currentState->name.c_str());
		}
		
	}
}

void ComponentAnimation::UpdateAnimations(GameObject* gameObject) {
	if (gameObject == nullptr) {
		return;
	}

	//find gameobject in hash
	float3 position = float3::zero;
	Quat rotation = Quat::identity;

	bool result = true;
	ResourceStateMachine* resourceStateMachine = App->resources->GetResource<ResourceStateMachine>(stateMachineResourceUID);
	if (!resourceStateMachine) {
		return;
	}

	if (animationInterpolations.size() > 1) {
		result = AnimationController::InterpolateTransitions(animationInterpolations.begin(), animationInterpolations, *GetOwner().GetRootBone(), *gameObject, position, rotation);

		//Updating times
		if (gameObject == GetOwner().GetRootBone()) { // Only udate currentTime for the rootBone
			AnimationController::UpdateTransitions(animationInterpolations, App->time->GetDeltaTime());
		}

	} else {
		if (currentState) {
			ResourceClip* clip = App->resources->GetResource<ResourceClip>(currentState->clipUid);
			result = AnimationController::GetTransform(*clip, currentState->currentTime, gameObject->name.c_str(), position, rotation);
			
			if (gameObject == GetOwner().GetRootBone()) {
				if (!clip->loop) {
					int currentSample = AnimationController::GetCurrentSample(*clip, currentState->currentTime);
					if (currentSample == clip->endIndex) {
						TesseractEvent animationFinishedEvent = TesseractEvent(TesseractEventType::ANIMATION_FINISHED);
						App->events->AddEvent(animationFinishedEvent);
					}
				}
			}
		}
	}

	ComponentTransform* componentTransform = gameObject->GetComponent<ComponentTransform>();

	if (componentTransform && result) {
		componentTransform->SetPosition(position);
		componentTransform->SetRotation(rotation);
	}

	for (GameObject* child : gameObject->GetChildren()) {
		UpdateAnimations(child);
	}
}

void ComponentAnimation::LoadResourceStateMachine(UID stateMachineResourceUid, StateMachineEnum stateMachineEnum) {
	ResourceStateMachine* resourceStateMachine = App->resources->GetResource<ResourceStateMachine>(stateMachineResourceUid);

	if (resourceStateMachine) {
		switch (stateMachineEnum) {
		case StateMachineEnum::PRIMARY:
			currentStatePrincipal = &resourceStateMachine->initialState;
		break;
		case StateMachineEnum::SECONDARY:
			currentStateSecondary = &resourceStateMachine->initialState;
		break;
	}
}
