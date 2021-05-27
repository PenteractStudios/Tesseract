#pragma once

enum class MaskType {
	NONE = 0,
	ENEMY = 1 << 1,
	PLAYER = 1 << 2,
	CAST_SHADOWS = 1 << 3
};

struct Mask {
	int bitMask = static_cast<int>(MaskType::NONE);

	const char* maskNames[3] = {"ENEMY", "PLAYER", "CAST SHADOWS"};
	bool maskValues[3] = {false, false, false};
};

const char* GetMaskTypeName(MaskType type);
MaskType GetMaskTypeFromName(const char* name);
