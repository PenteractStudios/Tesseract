#pragma once
#include "Utils/UID.h"
#include <Math/Quat.h>
#include <list>
#include <unordered_map>

class ResourceClip;
class AnimationInterpolation;
class GameObject;
class State;

namespace AnimationController {
	int GetCurrentSample(const ResourceClip& clip, float& currentTime);
	bool GetTransform(ResourceClip& clip, float& currentTime, const char* name, float3& pos, Quat& quat, bool firstBone);
	bool InterpolateTransitions(const std::list<AnimationInterpolation>::iterator& it, const std::list<AnimationInterpolation>& animationInterpolations, const GameObject& rootBone, const GameObject& gameObject, float3& pos, Quat& quat);
	bool UpdateTransitions(std::list<AnimationInterpolation>& animationInterpolations, std::unordered_map<UID,float>& currentTimeStates, const float time);
	Quat Interpolate(const Quat& first, const Quat& second, float lambda);
};