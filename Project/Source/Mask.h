#pragma once

enum class MaskType {
	NONE = 0,
	ENEMY = 1 << 1,
	CAST_SHADOW = 1 << 2
};

struct Mask {
	int bitMask = static_cast<int>(MaskType::NONE);
	const char* maskNames[2] = {"Enemy", "Cast Shadow"};
	bool maskValues[2] = {false, false};

	bool AddMask(MaskType mask_);
	bool DeleteMask(MaskType mask_);

};

const char* GetMaskTypeName(MaskType type);
MaskType GetMaskTypeFromName(const char* name);