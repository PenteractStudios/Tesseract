#pragma once

class State;

class AnimationInterpolation {
public:
	AnimationInterpolation(State* resourceState, float current, float fade, float transition)
		: state(resourceState)
		, currentTime(current)
		, fadeTime(fade)
		, transitionTime(transition) {}

public:
	State* state = nullptr;
	float currentTime = 0;
	float fadeTime = 0;
	float transitionTime = 0;
};
