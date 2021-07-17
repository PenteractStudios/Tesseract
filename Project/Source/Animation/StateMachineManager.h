#pragma once

#include "AnimationInterpolation.h"
#include "StateMachineEnum.h"
#include "Utils/UID.h"
#include "Resources/ResourceClip.h"

#include "Math/float3.h"
#include "Math/Quat.h"

#include <string>
#include <unordered_map>

class ComponentAnimation;
class GameObject;
//Class dedicated to manage the decision between the interpolation of the two state machines
namespace StateMachineManager {
	bool Contains(std::list<AnimationInterpolation>& animationInterpolations, const UID& id);

	void SendTrigger(const std::string& trigger, StateMachineEnum stateMachineSelected, ComponentAnimation& componentAnimation);

	bool UpdateAnimations(GameObject* gameObject, const GameObject& owner, ComponentAnimation& componentAnimation, float3& position, Quat& rotation, bool& resetSecondaryStatemachine);

	bool SecondaryEqualsToAnyPrincipal(const State& currentStateSecondary, const std::unordered_map<UID, State>& states);

	bool CalculateAnimation(GameObject* gameObject, const GameObject& owner, StateMachineEnum stateMachineSelected, ComponentAnimation& componentAnimation, float3& position, Quat& rotation, bool& resetSecondaryStatemachine, bool principalEqualSecondary = false);

}; // namespace StateMachineManager
