#pragma once
#include <Math/Quat.h>
#include <list>

class ResourceClip;
class AnimationInterpolation;
class GameObject;
class State;
class AnimationController {
public:
	static bool GetTransform(const ResourceClip& clip, float& currentTime, const char* name, float3& pos, Quat& quat);
	static bool InterpolateTransitions(const std::list<AnimationInterpolation>::iterator& it, const std::list<AnimationInterpolation>& animationInterpolations, const GameObject& rootBone, const GameObject& gameObject, float3& pos, Quat& quat);
	static State* UpdateTransitions(std::list<AnimationInterpolation>& animationInterpolations, const float time);
	static Quat Interpolate(const Quat& first, const Quat& second, float lambda);
};