#include "MaskType.h"

#include "Utils/Logging.h"

#include <cassert>

const char* GetMaskTypeName(MaskType type) {
	
	switch (type) {
	case MaskType::NONE:
		return "NONE";
	case MaskType::ENEMY:
		return "ENEMY";
	case MaskType::CAST_SHADOWS:
		return "Cast Shadows";
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
	}  else if (strcmp(name, "Cast Shadows") == 0) {
		return MaskType::CAST_SHADOWS;
	} else {
		assert(false);
		return MaskType::NONE;
	}

}
