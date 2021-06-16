#include "GodModeController.h"
#include "SpawnOnClick.h"
#include "PlayerController.h"

#include "GameplaySystems.h"
#include "GameObject.h"

#include "Components/UI/ComponentToggle.h"

EXPOSE_MEMBERS(GodModeController) {
	/* UI toggles*/
	MEMBER(MemberType::GAME_OBJECT_UID, uiCanvasUID),
	MEMBER(MemberType::GAME_OBJECT_UID, spawnMeleeUID),
	MEMBER(MemberType::GAME_OBJECT_UID, spawnRangedUID),
	/* Enemy groups*/
	MEMBER(MemberType::GAME_OBJECT_UID, enemiesUID),
	MEMBER(MemberType::GAME_OBJECT_UID, debugEnemiesUID),
	/* Cameras */
	MEMBER(MemberType::GAME_OBJECT_UID, godCameraUID),
	MEMBER(MemberType::GAME_OBJECT_UID, gameCameraUID),
	/* Player controller */
	MEMBER(MemberType::GAME_OBJECT_UID, playerControllerUID)
};

GENERATE_BODY_IMPL(GodModeController);

void GodModeController::Start() {
	/* Owner */
	gameObject = &GetOwner();
	/* UI toggles*/
	uiCanvas = GameplaySystems::GetGameObject(uiCanvasUID);
	spawnMelee = GameplaySystems::GetGameObject(spawnMeleeUID);
	spawnRanged = GameplaySystems::GetGameObject(spawnRangedUID);
	/* Enemy groups*/
	enemies = GameplaySystems::GetGameObject(enemiesUID);
	debugEnemies = GameplaySystems::GetGameObject(debugEnemiesUID);
	/* Cameras */
	godCamera = GameplaySystems::GetGameObject(godCameraUID);
	gameCamera = GameplaySystems::GetGameObject(gameCameraUID);
	/* Player controller */
	playerController = GameplaySystems::GetGameObject(playerControllerUID);
	if (playerController) playerControllerScript = GET_SCRIPT(playerController, PlayerController);

	for (GameObject* child : uiCanvas->GetChildren()) {
		if (child->HasComponent<ComponentToggle>()) {
			ComponentToggle* toggle = child->GetComponent<ComponentToggle>();
			if (toggle) toggles.push_back(toggle);
		}
	}
}

void GodModeController::Update() {}

void GodModeController::OnChildToggle(unsigned int index, bool isChecked) {
	switch (index) {
	case 0:
		if (isChecked) {
			if (toggles[index + 1]->IsChecked()) {
				toggles[index + 1]->SetChecked(false);
			}
			if (spawnMelee) spawnMelee->Enable();
		}
		else {
			if (spawnMelee) spawnMelee->Disable();
		}
		break;
	case 1:
		if (isChecked) {
			if (toggles[index - 1]->IsChecked()) {
				toggles[index - 1]->SetChecked(false);
			}
			if (spawnRanged) spawnRanged->Enable();
		}
		else {
			if (spawnRanged) spawnRanged->Disable();
		}
		break;
	case 2:
		if (isChecked) {
			if (enemies) enemies->Enable();
			if (debugEnemies) debugEnemies->Enable();
		}
		else {
			if (enemies) enemies->Disable();
			if (debugEnemies) debugEnemies->Disable();
		}
		break;
	case 3:
		if (isChecked) {
			if (toggles[index + 1]->IsChecked()) {
				toggles[index + 1]->SetChecked(false);
			}
			if (gameCamera) {
				GameplaySystems::SetRenderCamera(gameCamera->GetComponent<ComponentCamera>());
			}
		}
		else if (!toggles[index + 1]->IsChecked()) {
			toggles[index]->SetChecked(true);
		}
		break;
	case 4:
		if (isChecked) {
			if (toggles[index - 1]->IsChecked()) {
				toggles[index - 1]->SetChecked(false);
			}

			if (godCamera) {
				GameplaySystems::SetRenderCamera(godCamera->GetComponent<ComponentCamera>());
			}
		}
		else if (!toggles[index - 1]->IsChecked()) {
			toggles[index]->SetChecked(true);
		}
		break;
	case 5:
		if (isChecked) {
			if (playerControllerScript) playerControllerScript->SetOverpower(true);
		}
		else {
			if (playerControllerScript) playerControllerScript->SetOverpower(false);
		}
		break;
	case 6:
		if (isChecked) {
			if (playerControllerScript) playerControllerScript->SetNoCooldown(true);
		}
		else {
			if (playerControllerScript) playerControllerScript->SetNoCooldown(false);
		}
		break;
	case 7:
		if (isChecked) {
			if (playerControllerScript) playerControllerScript->SetInvincible(true);
		}
		else {
			if (playerControllerScript) playerControllerScript->SetInvincible(false);
		}
		break;

	default:
		break;
	}
}