#include "MaskType.h"

#include "Utils/Logging.h"

#include <cassert>

const char* GetMaskTypeName(MaskType type) {
	
	switch (type) {
	case MaskType::NONE:
		return "NONE";
	case MaskType::ENEMY:
		return "ENEMY";
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
	} else {
		assert(false);
		return MaskType::NONE;
	}

}
