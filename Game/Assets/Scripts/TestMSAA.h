#pragma once

#include "Scripting/Script.h"
#include "GameObject.h"
#include "Utils/UID.h"

class TestMSAA : public Script
{
	GENERATE_BODY(TestMSAA);

public:

	void Start() override;
	void Update() override;

public:
	UID videoID = 0;
	GameObject* video = nullptr;
};

