#pragma once

#include "Scripting/Script.h"

class GameObject;

class EnableDisableTest_1 : public Script
{
	GENERATE_BODY(EnableDisableTest_1);

public:

	void Start() override;
	void Update() override;

	UID gameObjectUID;

	GameObject* gameObject = nullptr;

	float timer = 0.0f;
	int dir = 1;
	bool canDisable = false;
	bool canEnable = false;
};

