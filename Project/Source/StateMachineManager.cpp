#include "StateMachineManager.h"
#include <Application.h>
#include <Modules/ModuleResources.h>
#include <Resources/ResourceStateMachine.h>
#include "Transition.h"
#include <Utils/UID.h>

void StateMachineManager::SendTrigger(const std::string& trigger,const UID stateMachineResourceUID, const State &currentState) {
	ResourceStateMachine* resourceStateMachine = App->resources->GetResource<ResourceStateMachine>(stateMachineResourceUID);

	Transition* transition = resourceStateMachine->FindTransitionGivenName(trigger);
	if (transition != nullptr) {
		if (transition->source.id == currentState.id) {
			if (animationInterpolations.size() == 0) {
				animationInterpolations.push_front(AnimationInterpolation(&transition->source, currentState.currentTime, 0, transition->interpolationDuration));
			}

			animationInterpolations.push_front(AnimationInterpolation(&transition->target, 0, 0, transition->interpolationDuration));
			currentState = &transition->target;
		} else {
			LOG("Warning: transition target from %s to %s, and current state is %s ", transition->source.name.c_str(), transition->target.name.c_str(), currentState.name.c_str());
		}
	}
}
