#pragma once

#include "Scripting/Script.h"

class GameObject;

class WinLose : public Script
{
	GENERATE_BODY(WinLose);

public:

	void Start() override;
	void Update() override;
	void IncrementDeadEnemies();

public:
	UID sceneUID = 0;
	UID winUID = 0;
	UID playerUID = 0;
	UID enemiesUID = 0;

	float loseOffsetX = 2.0f;
	float loseOffsetZ = 2.0f;
	int totalEnemies = 0;

private:
	GameObject* winCon = nullptr;
	GameObject* player = nullptr;
	GameObject* enemies = nullptr;
	int deadEnemies = 0;

};
