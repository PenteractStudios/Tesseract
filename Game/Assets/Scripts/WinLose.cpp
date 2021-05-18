#include "WinLose.h"

#include "GameObject.h"
#include "GameplaySystems.h"

EXPOSE_MEMBERS(WinLose) {
	// Add members here to expose them to the engine. Example:
	MEMBER(MemberType::GAME_OBJECT_UID, winUID),
	MEMBER(MemberType::GAME_OBJECT_UID, loseUID),
	MEMBER(MemberType::GAME_OBJECT_UID, playerUID),
	MEMBER(MemberType::FLOAT, LoseOffsetX),
	MEMBER(MemberType::FLOAT, LoseOffsetZ)
};

GENERATE_BODY_IMPL(WinLose);

void WinLose::Start() {
	winCon = GameplaySystems::GetGameObject(winUID);
	loseCon = GameplaySystems::GetGameObject(loseUID);
	player = GameplaySystems::GetGameObject(playerUID);
}

void WinLose::Update() {
	if (winCon == nullptr) return;
	if (loseCon == nullptr) return;
	if (player == nullptr) return;

	ComponentTransform* playerTransform = player->GetComponent<ComponentTransform>();
	ComponentTransform* winConTransform = winCon->GetComponent<ComponentTransform>();
	ComponentTransform* loseConTransform = loseCon->GetComponent<ComponentTransform>();
	if (!playerTransform || !winConTransform || !loseConTransform) return;

	float3 position = playerTransform->GetGlobalPosition();
	float3 winConPos = winConTransform->GetGlobalPosition();
	float3 loseConPos = loseConTransform->GetGlobalPosition();
	if (position.x <= winConPos.x + LoseOffsetX
		&& position.x >= winConPos.x - LoseOffsetX
		&& position.z <= winConPos.z + LoseOffsetZ
		&& position.z >= winConPos.z - LoseOffsetZ) {
		SceneManager::ChangeScene("Assets/Scenes/WinScene.scene");
	}
	else if (position.x <= loseConPos.x + LoseOffsetX
		&& position.x >= loseConPos.x - LoseOffsetX
		&& position.z <= loseConPos.z + LoseOffsetZ
		&& position.z >= loseConPos.z - LoseOffsetZ) {
		SceneManager::ChangeScene("Assets/Scenes/LoseScene.scene");
	}
}