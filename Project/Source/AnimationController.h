#pragma once
#include <Math/Quat.h>
#include <list>

class ResourceClip;
class AnimationInterpolation;
class GameObject;
class State;

namespace AnimationController {
	int GetCurrentSample(const ResourceClip& clip, float& currentTime);
	bool GetTransform(const ResourceClip& clip, float& currentTime, const char* name, float3& pos, Quat& quat);
	bool InterpolateTransitions(const std::list<AnimationInterpolation>::iterator& it, const std::list<AnimationInterpolation>& animationInterpolations, const GameObject& rootBone, const GameObject& gameObject, float3& pos, Quat& quat);
	State* UpdateTransitions(std::list<AnimationInterpolation>& animationInterpolations, const float time);
	Quat Interpolate(const Quat& first, const Quat& second, float lambda);
};