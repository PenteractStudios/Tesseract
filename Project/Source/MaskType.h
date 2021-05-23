#pragma once

enum class MaskType {
	NONE = 0,
	ENEMY = 1 << 1
};

struct Mask {

	int bitMask = static_cast<int>(MaskType::NONE);
	const char * maskNames[1] = {"Enemy"};
	bool maskValues[1] = {false};

};

const char* GetMaskTypeName(MaskType type);
MaskType GetMaskTypeFromName(const char* name);
