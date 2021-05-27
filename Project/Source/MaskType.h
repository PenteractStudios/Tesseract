#pragma once

enum class MaskType {
	NONE = 0,
	ENEMY = 1 << 1,
	CAST_SHADOWS = 1 << 2,
	TRANSPARENT = 1 << 3
};

struct Mask {
	int bitMask = static_cast<int>(MaskType::NONE);

	// Only to Show in PanelInspector
	const char* maskNames[2] = {"Enemy", "Cast Shadows"};
	bool maskValues[2] = {false, false};
};

const char* GetMaskTypeName(MaskType type);
MaskType GetMaskTypeFromName(const char* name);
