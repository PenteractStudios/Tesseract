#pragma once
#include <AnimationInterpolation.h>
#include "Utils/UID.h"
#include "StateMachineEnum.h"

#include <Math/float3.h>
#include <Math/Quat.h>

#include <string>
#include <unordered_map>

class GameObject;
namespace StateMachineManager {

	void SendTrigger(const std::string& trigger, std::unordered_map<UID, float>& currentTimeStates, 
		std::list<AnimationInterpolation> &animationInterpolations, const UID &stateMachineResourceUID, 
		State& currentState);

	bool UpdateAnimations(GameObject* gameObject, const GameObject& owner, std::unordered_map<UID, float>& currentTimeStatesPrincipal, 
		std::list<AnimationInterpolation>& animationInterpolationsPrincipal, const UID& stateMachineResourceUIDPrincipal, 
		State* currentStatePrincipal,	std::unordered_map<UID, float>& currentTimeStatesSecondary, 
		std::list<AnimationInterpolation> &animationInterpolationsSecondary, const UID &stateMachineResourceUIDSecondary, 
		State* currentStateSecondary, float3 &position , Quat &rotation);

	bool CalculateAnimation(GameObject* gameObject, const GameObject& owner,  std::unordered_map<UID, float>& currentTimeStates, 
		std::list<AnimationInterpolation>& animationInterpolations, const UID& stateMachineResourceUID, State* currentState, 
		float3& position, Quat& rotation, StateMachineEnum stateMachineEnum, bool principalEqualSecondary = false);
};
