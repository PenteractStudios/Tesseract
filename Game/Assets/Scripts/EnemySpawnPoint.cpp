#include "EnemySpawnPoint.h"

#include "Resources/ResourcePrefab.h"
#include "GameplaySystems.h"
#include "GameObject.h"
#include "WinLose.h"

EXPOSE_MEMBERS(EnemySpawnPoint) {
	MEMBER(MemberType::GAME_OBJECT_UID, winUID),
	MEMBER(MemberType::PREFAB_RESOURCE_UID, prefabUID),
	MEMBER(MemberType::GAME_OBJECT_UID, relatedSpawn1UID),
	MEMBER(MemberType::GAME_OBJECT_UID, relatedSpawn2UID),
	MEMBER(MemberType::GAME_OBJECT_UID, relatedSpawn3UID),
	MEMBER(MemberType::GAME_OBJECT_UID, relatedSpawn4UID),
	MEMBER(MemberType::INT, amountOfEnemies),
	MEMBER(MemberType::INT, offset),
};

GENERATE_BODY_IMPL(EnemySpawnPoint);

void EnemySpawnPoint::Start() {
	gameObject = &GetOwner();
	prefab = GameplaySystems::GetResource<ResourcePrefab>(prefabUID);
	winCon = GameplaySystems::GetGameObject(winUID);
	remainingEnemies = amountOfEnemies;
	if (winCon) winConditionScript = GET_SCRIPT(winCon, WinLose);

	relatedSpawn1 = GameplaySystems::GetGameObject(relatedSpawn1UID);
	relatedSpawn2 = GameplaySystems::GetGameObject(relatedSpawn2UID);
	relatedSpawn3 = GameplaySystems::GetGameObject(relatedSpawn3UID);
	relatedSpawn4 = GameplaySystems::GetGameObject(relatedSpawn4UID);

	if (relatedSpawn1) {
		relatedSpawnScript1 = GET_SCRIPT(relatedSpawn1, EnemySpawnPoint);
	}
	else {
		relatedSpawnEnemyStatus[0] = true;
	}
	if (relatedSpawn2) {
		relatedSpawnScript2 = GET_SCRIPT(relatedSpawn2, EnemySpawnPoint);
	}
	else {
		relatedSpawnEnemyStatus[1] = true;
	}
	if (relatedSpawn3) {
		relatedSpawnScript3 = GET_SCRIPT(relatedSpawn3, EnemySpawnPoint);
	}
	else {
		relatedSpawnEnemyStatus[2] = true;
	}
	if (relatedSpawn4) {
		relatedSpawnScript4 = GET_SCRIPT(relatedSpawn4, EnemySpawnPoint);
	}
	else {
		relatedSpawnEnemyStatus[3] = true;
	}
}

void EnemySpawnPoint::Update() {
	if (!gameObject->IsActive()) return;

	if (relatedSpawnEnemyStatus[0] && relatedSpawnEnemyStatus[1] && relatedSpawnEnemyStatus[2] && relatedSpawnEnemyStatus[3]) {
		for (; iterator < amountOfEnemies; ++iterator) {
			if (prefab) {
				UID prefabUID = prefab->BuildPrefab(gameObject);
				GameObject* go = GameplaySystems::GetGameObject(prefabUID);
				ComponentTransform* goTransform = go->GetComponent<ComponentTransform>();
				if (go) {
					ComponentBoundingBox* goBounds = nullptr;
					for (auto& child : go->GetChildren()) {
						if (child->HasComponent<ComponentMeshRenderer>()) {
							goBounds = child->GetComponent<ComponentBoundingBox>();
							break;
						}
					}
					if (goTransform && goBounds) {
						float3 newPosition = float3(0, 0, 0);
						float newXval = goBounds->GetLocalMaxPointAABB().x - abs(goBounds->GetLocalMinPointAABB().x);
						newXval = newXval < 1.f ? 1.f : newXval;
						newPosition.x += iterator * offset * newXval;
						goTransform->SetPosition(newPosition);
					}
				}
			}
		}
	}
	else {
		for (unsigned int i = 0; i < 4; ++i) {
			if (!relatedSpawnEnemyStatus[i]) {
				switch (i) {
				case 0:
					if (relatedSpawnScript1 && relatedSpawnScript1->GetRemainingEnemies() == 0) relatedSpawnEnemyStatus[i] = true;
					break;
				case 1:
					if (relatedSpawnScript2 && relatedSpawnScript2->GetRemainingEnemies() == 0) relatedSpawnEnemyStatus[i] = true;
					break;
				case 2:
					if (relatedSpawnScript3 && relatedSpawnScript3->GetRemainingEnemies() == 0) relatedSpawnEnemyStatus[i] = true;
					break;
				case 3:
					if (relatedSpawnScript4 && relatedSpawnScript4->GetRemainingEnemies() == 0) relatedSpawnEnemyStatus[i] = true;
					break;
				default:
					break;
				}
			}
		}
	}
}

void EnemySpawnPoint::UpdateRemainingEnemies() {
	remainingEnemies--;
}

int EnemySpawnPoint::GetRemainingEnemies() {
	return remainingEnemies;
}