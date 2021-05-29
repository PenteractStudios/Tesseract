#pragma once
#include "Utils/UID.h"

#include <unordered_map>

namespace StateMachinGenerator {
	bool GenerateStateMachine(const char* filePath);
	void SaveToFile(const char* filePath, std::unordered_map<UID, std::string>& listClips);
}; // namespace StateMachinGenerator
	