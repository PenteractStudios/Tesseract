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

#include <algorithm> // std::find

#include "Utils/Leaks.h"

#define JSON_TAG_LOOP "Controller"
#define JSON_TAG_ANIMATION_ID "AnimationId"
#define JSON_TAG_STATE_MACHINE_ID "StateMachineId"
#define JSON_TAG_CLIP "Clip"

void ComponentAnimation::Update() {
	if (!currentState) { //Checking if there is no state machine
		LoadResourceStateMachine();
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

	UID oldStateMachineUID = stateMachineResourceUID;
	ImGui::ResourceSlot<ResourceStateMachine>("State Machine", &stateMachineResourceUID);
	if (oldStateMachineUID != stateMachineResourceUID) {
		ResourceStateMachine* resourceStateMachine = App->resources->GetResource<ResourceStateMachine>(stateMachineResourceUID);
		if (resourceStateMachine) {
			currentState = &resourceStateMachine->initialState;
		} else {
			currentState = nullptr;
		}
	}
}

void ComponentAnimation::Save(JsonValue jComponent) const {
	jComponent[JSON_TAG_STATE_MACHINE_ID] = stateMachineResourceUID;
}

void ComponentAnimation::Load(JsonValue jComponent) {
	stateMachineResourceUID = jComponent[JSON_TAG_STATE_MACHINE_ID];
	if (stateMachineResourceUID != 0) App->resources->IncreaseReferenceCount(stateMachineResourceUID);

	LoadResourceStateMachine();
	InitCurrentTimeStates();
}

void ComponentAnimation::OnUpdate() {
	// Update gameobjects matrix
	GameObject* rootBone = GetOwner().GetRootBone();

	UpdateAnimations(rootBone);

	if (currentState) {
		ResourceClip* currentClip = App->resources->GetResource<ResourceClip>(currentState->clipUid);
		currentTimeStates[currentState->id] += App->time->GetDeltaTime() * currentClip->speed;
	}
}

void ComponentAnimation::SendTrigger(const std::string& trigger) {
	ResourceStateMachine* resourceStateMachine = App->resources->GetResource<ResourceStateMachine>(stateMachineResourceUID);

	Transition* transition = resourceStateMachine->FindTransitionGivenName(trigger);
	if (transition != nullptr) {
		if(transition->source.id == currentState->id){
			if (animationInterpolations.size() == 0) {
				animationInterpolations.push_front(AnimationInterpolation(&transition->source, currentTimeStates[currentState->id], 0, transition->interpolationDuration));
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
			AnimationController::UpdateTransitions(animationInterpolations, currentTimeStates, App->time->GetDeltaTime());
		}

	} else {
		if (currentState) {
			ResourceClip* clip = App->resources->GetResource<ResourceClip>(currentState->clipUid);
			result = AnimationController::GetTransform(*clip, currentTimeStates[currentState->id], gameObject->name.c_str(), position, rotation);
			
			if (gameObject == GetOwner().GetRootBone()) {
				if (!clip->loop) {
					int currentSample = AnimationController::GetCurrentSample(*clip, currentTimeStates[currentState->id]);
					if (currentSample == clip->endIndex) {
						for (ComponentScript& script : GetOwner().GetComponents<ComponentScript>()) {
							if (script.IsActive()) {
								Script* scriptInstance = script.GetScriptInstance();
								if (scriptInstance != nullptr) {
									scriptInstance->OnAnimationFinished();
								}
							}
						}
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

void ComponentAnimation::LoadResourceStateMachine() {
	ResourceStateMachine* resourceStateMachine = App->resources->GetResource<ResourceStateMachine>(stateMachineResourceUID);

	if (resourceStateMachine) {
		currentState = &resourceStateMachine->initialState;
	}
}

void ComponentAnimation::InitCurrentTimeStates() {
	ResourceStateMachine* resourceStateMachine = App->resources->GetResource<ResourceStateMachine>(stateMachineResourceUID);

	if (resourceStateMachine) {
		std::list<State>::iterator itState;
		for (itState = resourceStateMachine->states.begin(); itState != resourceStateMachine->states.end(); ++itState) {
			currentTimeStates.insert({itState->id, 0.0f});
		}
	}
}