#pragma once
#include "State.h"
#include "Utils/UID.h"
#include <string>

class Transition {
public:
	Transition(const State& mSource, const State& mTarget, float mInterpolation, UID mid = 0)
		: source(mSource)
		, target(mTarget)
		, interpolationDuration(mInterpolation) {
		id = mid != 0 ? mid : GenerateUID();
	}

public:
	UID id = 0;
	State source;
	State target;
	float interpolationDuration = 0;
};
