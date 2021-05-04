#pragma once

#include "Globals.h"

#include <string>

class Script;
class Creator;
class GameObject;

namespace Factory {
	void CreateContext();
	void DestroyContext();

	TESSERACT_ENGINE_API Script* Create(const std::string& className, GameObject* owner);
	TESSERACT_ENGINE_API void RegisterScript(const std::string& classname, Creator* creator);
};
