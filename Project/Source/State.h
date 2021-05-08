#pragma once
#include "Utils/UID.h"
#include <string>

class State {
public:
	State() {}
	State(const State& mState)
		: id(mState.id)
		, name(mState.name)
		, clipUid(mState.clipUid)
		, currentTime(mState.currentTime) {
	}

	State(const std::string& mName, UID mClipUid, float mCurrentTime = 0, UID mid = 0)
		: name(mName)
		, clipUid(mClipUid)
		, currentTime(mCurrentTime) {
		id = mid != 0 ? mid : GenerateUID();
	}

public:
	UID id = 0;
	UID clipUid = 0;
	std::string name = "";
	float currentTime = 0;
};
