#pragma once

#include "Scripting/Script.h"
class GameObject;

class TrailTest : public Script
{
	GENERATE_BODY(TrailTest);

public:

	void Start() override;
	void Update() override;

public:
	UID prefabId = 0;
private:
	GameObject* gameObject = nullptr;
};
