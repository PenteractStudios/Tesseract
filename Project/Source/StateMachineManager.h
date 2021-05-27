#pragma once
#include <AnimationInterpolation.h>
#include "Utils/UID.h"

#include <string>
#include <unordered_map>

class GameObject;
namespace StateMachineManager {

	void SendTrigger(const std::string& trigger, const std::unordered_map<UID, float>& currentTimeStates, 
		std::list<AnimationInterpolation> &animationInterpolations, const UID &stateMachineResourceUID, 
		State* currentState);

	bool UpdateAnimations(GameObject* gameObject, const std::unordered_map<UID, float>& currentTimeStatesPrimary, 
		std::list<AnimationInterpolation>& animationInterpolationsPrimary, const UID& stateMachineResourceUIDPrimary, 
		State* currentStatePrimary,
		const std::unordered_map<UID, float>& currentTimeStatesSecondary, 
		std::list<AnimationInterpolation> &animationInterpolationsSecondary, const UID &stateMachineResourceUIDSecondary, 
		State* currentStateSecondary, float3 &position , Quat &rotation);

	bool CalculateAnimation(GameObject* gameObject, const std::unordered_map<UID, float>& currentTimeStates, std::list<AnimationInterpolation>& animationInterpolations, const UID& stateMachineResourceUID, State* currentState)
};
