#include "WinLose.h"

#include "GameObject.h"
#include "GameplaySystems.h"
#include "EnemySpawnPoint.h"

EXPOSE_MEMBERS(WinLose) {
	MEMBER(MemberType::SCENE_RESOURCE_UID, sceneUID),
	MEMBER(MemberType::GAME_OBJECT_UID, winUID),
	MEMBER(MemberType::GAME_OBJECT_UID, playerUID),
	MEMBER(MemberType::GAME_OBJECT_UID, enemiesUID),
	MEMBER(MemberType::FLOAT, loseOffsetX),
	MEMBER(MemberType::FLOAT, loseOffsetZ)
};

GENERATE_BODY_IMPL(WinLose);

void WinLose::Start() {
	winCon = GameplaySystems::GetGameObject(winUID);
	player = GameplaySystems::GetGameObject(playerUID);
	enemies = GameplaySystems::GetGameObject(enemiesUID);
	if (enemies != nullptr && enemies->HasChildren()) {
		for (GameObject* spawn : enemies->GetChildren()) {
			if (spawn->IsActive()) {
				EnemySpawnPoint* enemySpawnPointScript = GET_SCRIPT(spawn, EnemySpawnPoint);
				if (enemySpawnPointScript != nullptr) {
					totalEnemies += enemySpawnPointScript->amountOfEnemies;
				}
			}
		}
	}

}

void WinLose::Update() {
	if (winCon == nullptr) return;
	if (player == nullptr) return;

	ComponentTransform* playerTransform = player->GetComponent<ComponentTransform>();
	ComponentTransform* winConTransform = winCon->GetComponent<ComponentTransform>();
	if (!playerTransform || !winConTransform) return;

	float3 position = playerTransform->GetGlobalPosition();
	float3 winConPos = winConTransform->GetGlobalPosition();
	if (position.x <= winConPos.x + loseOffsetX
		&& position.x >= winConPos.x - loseOffsetX
		&& position.z <= winConPos.z + loseOffsetZ
		&& position.z >= winConPos.z - loseOffsetZ
		// TODO: Commented for Pablo
		//&& deadEnemies >= totalEnemies
		) {
		if(sceneUID != 0) SceneManager::ChangeScene(sceneUID);
	}
}

void WinLose::IncrementDeadEnemies() {
	deadEnemies++;
}