#pragma once
#include "Utils/UID.h"
#include <string>

class State {
public:
	State() {}
	State(const State& mState)
		: id(mState.id)
		, name(mState.name)
		, clipUid(mState.clipUid) {
	}

	State(const std::string& mName, UID mClipUid, UID mid = 0)
		: name(mName)
		, clipUid(mClipUid) {
		id = mid != 0 ? mid : GenerateUID();
	}

public:
	UID id = 0;
	UID clipUid = 0;
	std::string name = "";
};
