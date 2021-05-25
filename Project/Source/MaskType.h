#pragma once

enum class MaskType {
	NONE = 0,
	ENEMY = 1 << 1,
	PLAYER = 1 << 2
};

struct Mask {
	int bitMask = static_cast<int>(MaskType::NONE);
	const char* maskNames[2] = {"Enemy", "Player"};
	bool maskValues[2] = {false, false};
};

const char* GetMaskTypeName(MaskType type);
MaskType GetMaskTypeFromName(const char* name);
