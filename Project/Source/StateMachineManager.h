#pragma once
#include <AnimationInterpolation.h>
#include "Utils/UID.h"
#include "StateMachineEnum.h"

#include <Math/float3.h>
#include <Math/Quat.h>

#include <string>
#include <unordered_map>

class GameObject;
//Class dedicated to manage the decision between the interpolation of the two state machines
namespace StateMachineManager {
	bool Contains(std::list<AnimationInterpolation>& animationInterpolations, const UID& id);

	void SendTrigger(const std::string& trigger, std::unordered_map<UID, float>& currentTimeStates, std::list<AnimationInterpolation>& animationInterpolations, const UID& stateMachineResourceUID, State& currentState, State& currentStateSecondary, std::unordered_map<UID, float>& currentTimeStatesPrincipal, StateMachineEnum stateMachineEnum, std::list<AnimationInterpolation>& animationInterpolationsSecondary);

	bool UpdateAnimations(GameObject* gameObject, const GameObject& owner, std::unordered_map<UID, float>& currentTimeStatesPrincipal, std::list<AnimationInterpolation>& animationInterpolationsPrincipal, const UID& stateMachineResourceUIDPrincipal, State* currentStatePrincipal, std::unordered_map<UID, float>& currentTimeStatesSecondary, std::list<AnimationInterpolation>& animationInterpolationsSecondary, const UID& stateMachineResourceUIDSecondary, State* currentStateSecondary, float3& position, Quat& rotation, bool& resetSecondaryStatemachine);

	bool SecondaryEqualsToAnyPrincipal(const State& currentStateSecondary, const std::unordered_map<UID, State>& states);

	bool CalculateAnimation(GameObject* gameObject, const GameObject& owner, std::unordered_map<UID, float>& currentTimeStates, std::list<AnimationInterpolation>& animationInterpolations, const UID& stateMachineResourceUID, State* currentState, float3& position, Quat& rotation, bool& resetSecondaryStatemachine,const StateMachineEnum stateMachineEnum, bool principalEqualSecondary = false);

}; // namespace StateMachineManager
