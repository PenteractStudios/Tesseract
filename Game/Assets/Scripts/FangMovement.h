#pragma once

#include "Scripting/Script.h"
#include "Character.h"

class GameObject;
class AIMovement;

class FangMovement : public Script
{
	GENERATE_BODY(FangMovement);

public:

	void Start() override;
	void Update() override;

public:
	UID cameraUID = 0;
	UID fangUID = 0;
	UID hitGOUID = 0;
	int speed = 10;
	float distanceRayCast = 20;
	float cameraXPosition = -163;

private:
	GameObject* camera = nullptr;
	GameObject* fang = nullptr;
	GameObject* hitGo = nullptr;
	Character character;
};
