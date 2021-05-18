#pragma once

#include "Factory.h"

#include <string>

class Script;
class GameObject;

class TESSERACT_ENGINE_API Creator {
public:
	Creator(const std::string& className) {
		Factory::RegisterScript(className, this);
	}
	virtual ~Creator() {};

	virtual Script* Create(GameObject* owner) = 0;
};

template<class T>
class TESSERACT_ENGINE_API CreatorImplementation : public Creator {
public:
	CreatorImplementation<T>(const std::string& className) : Creator(className) {}

	virtual ~CreatorImplementation<T>() {}

	virtual Script* Create(GameObject* owner) {
		return new T(owner);
	}
};