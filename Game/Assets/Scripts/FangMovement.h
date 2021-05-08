#pragma once

#include "Scripting/Script.h"

class GameObject;

class FangMovement : public Script
{
	GENERATE_BODY(FangMovement);

public:

	void Start() override;
	void Update() override;

public:
	UID cameraUID;
	UID fangUID;
	int speed = 10;
	float cameraXPosition = -163;

private:
	GameObject* camera = nullptr;
	GameObject* fang = nullptr;
};
