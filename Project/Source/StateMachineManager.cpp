#include "StateMachineManager.h"
#include <Application.h>
#include <Modules/ModuleResources.h>
#include "Modules/ModuleTime.h"
#include <Resources/ResourceStateMachine.h>
#include "Resources/ResourceClip.h"
#include "Transition.h"
#include <Utils/UID.h>
#include <Utils/Logging.h>
#include "GameObject.h"

void StateMachineManager::SendTrigger(const std::string& trigger,  std::unordered_map<UID, float>& currentTimeStates, 
	std::list<AnimationInterpolation>& animationInterpolations, const UID& stateMachineResourceUID, State* currentState) {
	ResourceStateMachine* resourceStateMachine = App->resources->GetResource<ResourceStateMachine>(stateMachineResourceUID);

	Transition* transition = resourceStateMachine->FindTransitionGivenName(trigger);
	if (transition != nullptr) {
		if (transition->source.id == currentState->id) {
			if (animationInterpolations.size() == 0) {
				animationInterpolations.push_front(AnimationInterpolation(&transition->source, currentTimeStates[currentState->id], 0, transition->interpolationDuration));
			}

			animationInterpolations.push_front(AnimationInterpolation(&transition->target, 0, 0, transition->interpolationDuration));
			currentState = &transition->target;
		} else {
			LOG("Warning: transition target from %s to %s, and current state is %s ", transition->source.name.c_str(), transition->target.name.c_str(), currentState->name.c_str());
		}
	}
}

bool StateMachineManager::UpdateAnimations(GameObject* gameObject, const GameObject *owner, std::unordered_map<UID, float>& currentTimeStatesPrincipal, 
	std::list<AnimationInterpolation>& animationInterpolationsPrincipal, const  UID& stateMachineResourceUIDPrincipal, 
	State* currentStatePrincipal,  std::unordered_map<UID, float>& currentTimeStatesSecondary, 
	std::list<AnimationInterpolation>& animationInterpolationsSecondary,const  UID& stateMachineResourceUIDSecondary, 
	State* currentStateSecondary, float3& position, Quat& rotation) {

	bool result = true;
	StateMachineEnum stateMachineSelected = StateMachineEnum::PRINCIPAL;
	//The currentStateSecondary could be null
	if (currentStateSecondary != nullptr ) {
		ResourceStateMachine* resourceStateMachine = App->resources->GetResource<ResourceStateMachine>(stateMachineResourceUIDSecondary);
		
		if (resourceStateMachine && resourceStateMachine->bones.empty()) {
			auto nameBone = resourceStateMachine->bones.find(gameObject->name);
			if (nameBone != resourceStateMachine->bones.end()) {
				stateMachineSelected = StateMachineEnum::SECONDARY;
			}
		}

	}

	switch (stateMachineSelected) {
	case StateMachineEnum::PRINCIPAL:
		result = StateMachineManager::CalculateAnimation(gameObject, owner->GetRootBone(), currentTimeStatesPrincipal, animationInterpolationsPrincipal, stateMachineResourceUIDPrincipal, currentStatePrincipal, position, rotation, StateMachineEnum::PRINCIPAL);
		break;
	case StateMachineEnum::SECONDARY:
		result = StateMachineManager::CalculateAnimation(gameObject, owner->GetRootBone(), currentTimeStatesSecondary, animationInterpolationsSecondary, stateMachineResourceUIDSecondary, currentStateSecondary, position, rotation, StateMachineEnum::SECONDARY, currentStatePrincipal->id == currentStateSecondary->id);
		break;
	}


	return result;
}

bool StateMachineManager::CalculateAnimation(GameObject* gameObject, const GameObject* owner,  std::unordered_map<UID, float>& currentTimeStates, 
	std::list<AnimationInterpolation>& animationInterpolations, const UID& stateMachineResourceUID, State* currentState,
	float3& position, Quat& rotation, StateMachineEnum stateMachineEnum, bool principalEqualSecondary) {
	bool result = false;
	
	ResourceStateMachine* resourceStateMachine = App->resources->GetResource<ResourceStateMachine>(stateMachineResourceUID);
	if (!resourceStateMachine) {
		return result;
	}

	if (animationInterpolations.size() > 1) {
		result = AnimationController::InterpolateTransitions(animationInterpolations.begin(), animationInterpolations, *owner->GetRootBone(), *gameObject, position, rotation);

		//Updating times
		if (gameObject == owner->GetRootBone()) { // Only udate currentTime for the rootBone
			bool finishedTransition = AnimationController::UpdateTransitions(animationInterpolations, currentTimeStates, App->time->GetDeltaTime());
			//Compare the state principal with state secondary & set secondary to null
			if (finishedTransition && principalEqualSecondary) {
				currentState = nullptr;
			}
		}

	} else {
		if (currentState) {
			ResourceClip* clip = App->resources->GetResource<ResourceClip>(currentState->clipUid);
			result = AnimationController::GetTransform(*clip, currentTimeStates[currentState->id], gameObject->name.c_str(), position, rotation);

			if (gameObject == owner->GetRootBone()) {
				if (!clip->loop) {
					int currentSample = AnimationController::GetCurrentSample(*clip, currentTimeStates[currentState->id]);
					if (currentSample == clip->endIndex) {
						for (ComponentScript& script : owner->GetComponents<ComponentScript>()) {
							if (script.IsActive()) {
								Script* scriptInstance = script.GetScriptInstance();
								if (scriptInstance != nullptr) {
									scriptInstance->OnAnimationFinished();
									switch (stateMachineEnum) {
									case StateMachineEnum::PRINCIPAL:
										scriptInstance->OnAnimationFinished();
										break;
									case StateMachineEnum::SECONDARY:
										scriptInstance->OnAnimationSecondaryFinished();
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return result;
}
