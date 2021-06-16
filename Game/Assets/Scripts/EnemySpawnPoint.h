#pragma once

#include "Scripting/Script.h"

class GameObject;
class ResourcePrefab;
class WinLose;

class EnemySpawnPoint : public Script {
	GENERATE_BODY(EnemySpawnPoint);

public:
	void Start() override;
	void Update() override;
	void UpdateRemainingEnemies();
	int GetRemainingEnemies();

public:
	UID winUID = 0;
	UID prefabUID = 0;

	/* Optional related spawn points */
	UID relatedSpawn1UID = 0;
	UID relatedSpawn2UID = 0;
	UID relatedSpawn3UID = 0;
	UID relatedSpawn4UID = 0;

	int amountOfEnemies = 0;
	int offset = 1;

private:
	GameObject* gameObject = nullptr;
	GameObject* winCon = nullptr;

	WinLose* winConditionScript = nullptr;
	ResourcePrefab* prefab = nullptr;
	int iterator = 0;
	int remainingEnemies = 0;
	bool relatedSpawnEnemyStatus[4] = {false};

	/* Optional related spawn points */
	GameObject* relatedSpawn1 = nullptr;
	GameObject* relatedSpawn2 = nullptr;
	GameObject* relatedSpawn3 = nullptr;
	GameObject* relatedSpawn4 = nullptr;
	
	EnemySpawnPoint* relatedSpawnScript1 = nullptr;
	EnemySpawnPoint* relatedSpawnScript2 = nullptr;
	EnemySpawnPoint* relatedSpawnScript3 = nullptr;
	EnemySpawnPoint* relatedSpawnScript4 = nullptr;
	
};
