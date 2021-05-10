#include "Mask.h"

#include "Utils/Logging.h"

#include <cassert>

const char*
	GetMaskTypeName(MaskType type) {
	switch (type) {
	case MaskType::NONE:
		return "NONE";
	case MaskType::ENEMY:
		return "ENEMY";
	case MaskType::CAST_SHADOW:
		return "Cast Shadow";
	default:
		LOG("The mask selected is not registered.");
		assert(false);
		return nullptr;
	}

	return nullptr;
}

MaskType GetMaskTypeFromName(const char* name) {
	if (strcmp(name, "NONE") == 0) {
		return MaskType::NONE;
	} else if (strcmp(name, "Enemy") == 0) {
		return MaskType::ENEMY;
	} else if (strcmp(name, "Cast Shadow") == 0) {
		return MaskType::CAST_SHADOW;
	} else {
		assert(false);
		return MaskType::NONE;
	}
}

bool Mask::AddMask(MaskType mask_) {
	switch (mask_) {
	case MaskType::ENEMY:
	case MaskType::CAST_SHADOW:
		bitMask |= static_cast<int>(mask_);
		return true;
	default:
		LOG("The requested mask doesn't exist.");
		return false;
	}
}

bool Mask::DeleteMask(MaskType mask_) {
	switch (mask_) {
	case MaskType::ENEMY:
	case MaskType::CAST_SHADOW:
		bitMask ^= static_cast<int>(mask_);
		return true;
	default:
		LOG("The requested mask doesn't exist.");
		return false;
	}
}