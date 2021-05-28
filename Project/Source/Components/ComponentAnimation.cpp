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
#include "StateMachineManager.h"

#include <algorithm> // std::find

#include "Utils/Leaks.h"

#define JSON_TAG_LOOP "Controller"
#define JSON_TAG_ANIMATION_ID "AnimationId"
#define JSON_TAG_STATE_MACHINE_PRINCIPAL_ID "StateMachinePrincipalId"
#define JSON_TAG_STATE_MACHINE_SECONDARY_ID "StateMachineSecondaryId"
#define JSON_TAG_CLIP "Clip"

void ComponentAnimation::Update() {
	if (!currentStatePrincipal) { //Checking if there is no state machine
		LoadResourceStateMachine(stateMachineResourceUIDPrincipal, StateMachineEnum::PRINCIPAL);
	}
	if (!currentStateSecondary) { //Checking if there is no state machine
		LoadResourceStateMachine(stateMachineResourceUIDSecondary, StateMachineEnum::SECONDARY);
	}

	if (currentTimeStatesPrincipal.empty()) { //Checking if there is no state machine
		InitCurrentTimeStates(stateMachineResourceUIDPrincipal, StateMachineEnum::PRINCIPAL);
	}

	if (currentTimeStatesSecondary.empty()) { //Checking if there is no state machine
		InitCurrentTimeStates(stateMachineResourceUIDSecondary, StateMachineEnum::SECONDARY);
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
	UID oldStateMachinePrincipalUID = stateMachineResourceUIDPrincipal;
	ImGui::ResourceSlot<ResourceStateMachine>("State Machine Principal", &stateMachineResourceUIDPrincipal);
	if (oldStateMachinePrincipalUID != stateMachineResourceUIDPrincipal) {
		ResourceStateMachine* resourceStateMachine = App->resources->GetResource<ResourceStateMachine>(stateMachineResourceUIDPrincipal);
		if (resourceStateMachine) {
			currentStatePrincipal = &resourceStateMachine->initialState;
		} else {
			currentStatePrincipal = nullptr;
		}
	}

	// Secondary
	UID oldStateMachineSecondaryUID = stateMachineResourceUIDSecondary;
	ImGui::ResourceSlot<ResourceStateMachine>("State Machine Secondary", &stateMachineResourceUIDSecondary);
	if (oldStateMachineSecondaryUID != stateMachineResourceUIDSecondary) {
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

	LoadResourceStateMachine(stateMachineResourceUIDPrincipal, StateMachineEnum::PRINCIPAL);
	LoadResourceStateMachine(stateMachineResourceUIDSecondary, StateMachineEnum::SECONDARY);

	InitCurrentTimeStates(stateMachineResourceUIDPrincipal, StateMachineEnum::PRINCIPAL);
	InitCurrentTimeStates(stateMachineResourceUIDSecondary, StateMachineEnum::SECONDARY);
}

void ComponentAnimation::OnUpdate() {
	// Update gameobjects matrix
	GameObject* rootBone = GetOwner().GetRootBone();

	UpdateAnimations(rootBone);

	if (currentStatePrincipal) {
		ResourceClip* currentClip = App->resources->GetResource<ResourceClip>(currentStatePrincipal->clipUid);
		currentTimeStatesPrincipal[currentStatePrincipal->id] += App->time->GetDeltaTime() * currentClip->speed;
	}
	if(currentStateSecondary){ // TODO:: set currentStateSecondary to null when its not used
		ResourceClip* currentClip = App->resources->GetResource<ResourceClip>(currentStateSecondary->clipUid);
		currentTimeStatesSecondary[currentStateSecondary->id] += App->time->GetDeltaTime() * currentClip->speed;
	}
}


void ComponentAnimation::SendTriggerPrincipal(const std::string& trigger) {
	StateMachineManager::SendTrigger(trigger, currentTimeStatesPrincipal, animationInterpolationsPrincipal, stateMachineResourceUIDPrincipal, *currentStatePrincipal);
}
void ComponentAnimation::SendTriggerSecondary(const std::string& trigger) {
	if (currentStateSecondary == nullptr) {
		ResourceStateMachine* resourceStateMachine = App->resources->GetResource<ResourceStateMachine>(stateMachineResourceUIDSecondary);
		std::unordered_map<UID, State>::iterator it = resourceStateMachine->states.find(currentStatePrincipal->id);
		assert(it != resourceStateMachine->states.end());
		currentStateSecondary = &(*it).second;
	}
	StateMachineManager::SendTrigger(trigger, currentTimeStatesSecondary, animationInterpolationsSecondary, stateMachineResourceUIDSecondary,*currentStateSecondary);
}

void ComponentAnimation::UpdateAnimations(GameObject* gameObject) {
	if (gameObject == nullptr) {
		return;
	}

	//find gameobject in hash
	float3 position = float3::zero;
	Quat rotation = Quat::identity;

	bool result = StateMachineManager::UpdateAnimations(
		gameObject,
		GetOwner(),
		currentTimeStatesPrincipal,
		animationInterpolationsPrincipal,
		stateMachineResourceUIDPrincipal,
		currentStatePrincipal,
		currentTimeStatesSecondary,
		animationInterpolationsSecondary,
		stateMachineResourceUIDSecondary,
		currentStateSecondary,
		position,
		rotation);
	/*ResourceStateMachine* resourceStateMachine = App->resources->GetResource<ResourceStateMachine>(stateMachineResourceUID);
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
	*/
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
		case StateMachineEnum::PRINCIPAL:
			currentStatePrincipal = &resourceStateMachine->initialState;
			break;
		case StateMachineEnum::SECONDARY:
			currentStateSecondary = &resourceStateMachine->initialState;
			break;
		}
	}
}


void ComponentAnimation::InitCurrentTimeStates(UID stateMachineResourceUid, StateMachineEnum stateMachineEnum) {
	ResourceStateMachine* resourceStateMachine = App->resources->GetResource<ResourceStateMachine>(stateMachineResourceUid);

	if (resourceStateMachine) {
		switch (stateMachineEnum) {
			case StateMachineEnum::PRINCIPAL:
				for (const auto& element : resourceStateMachine->states) {
					currentTimeStatesPrincipal.insert({element.first, 0.0f});
				}
			break;
			case StateMachineEnum::SECONDARY:
				for (const auto& element : resourceStateMachine->states) {
					currentTimeStatesSecondary.insert({element.first, 0.0f});
				}
				break;
		}
		
	}
}