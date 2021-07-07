#include "PlayerController.h"

#include "GameObject.h"
#include "GameplaySystems.h"

#include "AIMeleeGrunt.h"
#include "RangedAI.h"
#include "HUDController.h"
#include "OnimaruBullet.h"
#include "SwitchParticles.h"
#include "Math/Quat.h"
#include "Geometry/Plane.h"
#include "Geometry/Frustum.h"
#include "Geometry/LineSegment.h"
#include "Math/float3x3.h"
#include "Math/float2.h"
#include <algorithm>
#include <string>

#define PI 3.14159
#define AUDIOSOURCE_NULL_MSG "shootAudioSource is NULL"

EXPOSE_MEMBERS(PlayerController) {
	// Add members here to expose them to the engine. Example:
	MEMBER(MemberType::GAME_OBJECT_UID, fangUID),
	MEMBER(MemberType::GAME_OBJECT_UID, onimaruUID),
	MEMBER(MemberType::GAME_OBJECT_UID, mainNodeUID),
	MEMBER(MemberType::GAME_OBJECT_UID, cameraUID),
	MEMBER(MemberType::PREFAB_RESOURCE_UID, fangBulletUID),
	MEMBER(MemberType::PREFAB_RESOURCE_UID, fangTrailUID),
	MEMBER(MemberType::PREFAB_RESOURCE_UID, onimaruBulletUID),
	MEMBER(MemberType::GAME_OBJECT_UID, fangLeftGunUID),
	MEMBER(MemberType::GAME_OBJECT_UID, fangRightGunUID),
	MEMBER(MemberType::GAME_OBJECT_UID, EMPUID),
	MEMBER(MemberType::GAME_OBJECT_UID, onimaruGunUID),
	MEMBER(MemberType::GAME_OBJECT_UID, onimaruRightHandUID),
	MEMBER(MemberType::GAME_OBJECT_UID, onimaruShieldUID),
	MEMBER(MemberType::GAME_OBJECT_UID, switchParticlesUID),
	MEMBER(MemberType::GAME_OBJECT_UID, canvasUID),
	MEMBER(MemberType::GAME_OBJECT_UID, fangUltimateUID),
	MEMBER(MemberType::FLOAT, switchCooldown),
	MEMBER(MemberType::INT, playerFang.ultimateCooldown),
	MEMBER(MemberType::FLOAT, playerFang.ultimateMovementSpeed),
	MEMBER(MemberType::FLOAT, playerFang.lifePoints),
	MEMBER(MemberType::FLOAT, playerFang.movementSpeed),
	MEMBER(MemberType::FLOAT, playerFang.damageHit),
	MEMBER(MemberType::FLOAT, playerFang.attackSpeed),
	MEMBER(MemberType::FLOAT, playerFang.dashCooldown),
	MEMBER(MemberType::FLOAT, playerFang.dashSpeed),
	MEMBER(MemberType::FLOAT, playerFang.dashDuration),
	MEMBER(MemberType::FLOAT, playerFang.EMPRadius),
	MEMBER(MemberType::FLOAT, playerFang.EMPCooldown),
	MEMBER(MemberType::FLOAT, playerOnimaru.lifePoints),
	MEMBER(MemberType::FLOAT, playerOnimaru.movementSpeed),
	MEMBER(MemberType::FLOAT, playerOnimaru.damageHit),
	MEMBER(MemberType::FLOAT, playerOnimaru.attackSpeed),
	MEMBER(MemberType::FLOAT, playerOnimaru.blastCooldown),
	MEMBER(MemberType::FLOAT, playerOnimaru.blastDistance),
	MEMBER(MemberType::FLOAT, playerOnimaru.blastAngle),
	MEMBER(MemberType::FLOAT, rangedDamageTaken),
	MEMBER(MemberType::FLOAT, meleeDamageTaken),
	MEMBER(MemberType::BOOL, useSmoothCamera),
	MEMBER(MemberType::FLOAT, smoothCameraSpeed),
	MEMBER(MemberType::FLOAT, onimaruRecoveryRate),
	MEMBER(MemberType::FLOAT, fangRecoveryRate),
	MEMBER(MemberType::BOOL, debugGetHit),
	MEMBER(MemberType::FLOAT, switchDelay),
	MEMBER(MemberType::FLOAT, playerOnimaru.ultimateAttackSpeed),
	MEMBER(MemberType::FLOAT, playerOnimaru.ultimateTotalTime),
	MEMBER(MemberType::FLOAT, playerOnimaru.ultimateRotationSpeed),
	MEMBER(MemberType::INT, playerOnimaru.ultimateChargePoints),
	MEMBER(MemberType::INT, playerOnimaru.ultimateChargePointsTotal),
	MEMBER(MemberType::FLOAT, playerOnimaru.maxBulletSpread),
	MEMBER(MemberType::GAME_OBJECT_UID, onimaruUltimateProjectileOriginUID)
};

GENERATE_BODY_IMPL(PlayerController);

void PlayerController::Start() {
	playerFang.Init(fangUID, fangTrailUID, fangLeftGunUID, fangRightGunUID, fangBulletUID, cameraUID, canvasUID, EMPUID, fangUltimateUID);
	playerOnimaru.Init(onimaruUID, onimaruBulletUID, onimaruGunUID, onimaruRightHandUID, onimaruShieldUID,onimaruUltimateProjectileOriginUID, cameraUID, canvasUID, playerOnimaru.maxBulletSpread);

	GameObject* canvasGO = GameplaySystems::GetGameObject(canvasUID);
	if (canvasGO) {
		hudControllerScript = GET_SCRIPT(canvasGO, HUDController);
	}

	camera = GameplaySystems::GetGameObject(cameraUID);
	//animation
	if (camera) {
		compCamera = camera->GetComponent<ComponentCamera>();
		if (compCamera) {
			GameplaySystems::SetRenderCamera(compCamera);
		}
		cameraTransform = camera->GetComponent<ComponentTransform>();
	}


	switchEffects = GameplaySystems::GetGameObject(switchParticlesUID);

	firstTime = true;
	//Get audio sources
	int i = 0;

	for (ComponentAudioSource& src : GetOwner().GetComponents<ComponentAudioSource>()) {
		if (i < static_cast<int>(AudioType::TOTAL)) audios[i] = &src;
		i++;
	}
}


//Debug
void PlayerController::SetInvincible(bool status) {
	invincibleMode = status;
}

void PlayerController::SetOverpower(bool status) {
	overpowerMode = status ? 999 : 1;
}

int PlayerController::GetOverPowerMode() {
	return overpowerMode;
}

void PlayerController::SetNoCooldown(bool status) {
	noCooldownMode = status;
	ResetSwitchStatus();
}
//Switch
bool PlayerController::CanSwitch() {

	if (playerFang.characterGameObject->IsActive()) {
		return !switchInCooldown && playerFang.CanSwitch() && !playerFang.ultimateOn;
	} else {
		return !switchInCooldown && playerOnimaru.CanSwitch();
	}

}

void PlayerController::ResetSwitchStatus() {
	switchInProgress = false;
	playSwitchParticles = true;
	currentSwitchDelay = 0.f;
}

void PlayerController::SwitchCharacter() {
	if (!playerFang.characterGameObject) return;
	if (!playerOnimaru.characterGameObject) return;
	bool doVisualSwitch = currentSwitchDelay < switchDelay ? false : true;
	if (doVisualSwitch) {
		if (audios[static_cast<int>(AudioType::SWITCH)]) {
			audios[static_cast<int>(AudioType::SWITCH)]->Play();
		}
		if (playerFang.characterGameObject->IsActive()) {
			playerFang.characterGameObject->Disable();
			playerOnimaru.characterGameObject->Enable();

			if (hudControllerScript) {
				hudControllerScript->UpdateHP(playerOnimaru.lifePoints, playerFang.lifePoints);
				hudControllerScript->ResetHealthRegenerationEffects(playerFang.lifePoints);
			}

			fangRecovering = 0.0f;
		} else {
			playerOnimaru.characterGameObject->Disable();
			playerFang.characterGameObject->Enable();

			if (hudControllerScript) {
				hudControllerScript->UpdateHP(playerFang.lifePoints, playerOnimaru.lifePoints);
				hudControllerScript->ResetHealthRegenerationEffects(playerOnimaru.lifePoints);
			}

			onimaruRecovering = 0.0f;
		}
		if (hudControllerScript) {
			hudControllerScript->ChangePlayerHUD(playerFang.lifePoints, playerOnimaru.lifePoints);
			hudControllerScript->ResetCooldownProgressBar();
		}
		currentSwitchDelay = 0.f;
		playSwitchParticles = true;
		switchInCooldown = true;
		if (noCooldownMode) switchInProgress = false;
	} else {
		if (playSwitchParticles) {
			if (switchEffects) {
				SwitchParticles* script = GET_SCRIPT(switchEffects, SwitchParticles);
				if (script) {
					script->Play();
				}
			}
			switchInProgress = true;
			playSwitchParticles = false;
		}
		currentSwitchDelay += Time::GetDeltaTime();
	}
}
//Timers
void PlayerController::CheckCoolDowns() {

	playerFang.CheckCoolDowns(noCooldownMode);
	playerOnimaru.CheckCoolDowns(noCooldownMode);
	if (noCooldownMode || switchCooldownRemaining <= 0.f) {
		switchCooldownRemaining = 0.f;
		switchInCooldown = false;
		if (!noCooldownMode) switchInProgress = false;
	} else {
		switchCooldownRemaining -= Time::GetDeltaTime();
	}

	if (playerOnimaru.characterGameObject->IsActive() && playerFang.lifePoints != FANG_MAX_HEALTH) {
		if (fangRecovering >= fangRecoveryRate) {
			playerFang.Recover(1);
			fangRecovering = 0.0f;
		} else {
			fangRecovering += Time::GetDeltaTime();
		}
	}

	if (playerFang.characterGameObject->IsActive() && playerOnimaru.lifePoints != ONIMARU_MAX_HEALTH) {
		if (onimaruRecovering >= onimaruRecoveryRate) {
			playerOnimaru.Recover(1);
			onimaruRecovering = 0.0f;
		} else {
			onimaruRecovering += Time::GetDeltaTime();
		}
	}
}
//HUD
void PlayerController::UpdatePlayerStats() {
	if (hudControllerScript) {
		if (firstTime) {
			hudControllerScript->UpdateHP(playerFang.lifePoints, playerOnimaru.lifePoints);
			firstTime = false;
		}

		if (hitTaken && playerFang.IsActive() && playerFang.lifePoints >= 0) {
			hudControllerScript->UpdateHP(playerFang.lifePoints, playerOnimaru.lifePoints);
			hitTaken = false;
		} else if (hitTaken && playerOnimaru.IsActive() && playerOnimaru.lifePoints >= 0) {
			hudControllerScript->UpdateHP(playerOnimaru.lifePoints, playerFang.lifePoints);
			hitTaken = false;
		}

		if (playerFang.IsActive() && playerOnimaru.lifePoints != ONIMARU_MAX_HEALTH) {
			float healthRecovered = (onimaruRecovering / onimaruRecoveryRate);
			if (hudControllerScript) {
				hudControllerScript->HealthRegeneration(playerOnimaru.lifePoints, healthRecovered);
			}
		} else if (playerOnimaru.IsActive() && playerFang.lifePoints != FANG_MAX_HEALTH) {
			float healthRecovered = (fangRecovering / fangRecoveryRate);
			if (hudControllerScript) {
				hudControllerScript->HealthRegeneration(playerFang.lifePoints, healthRecovered);
			}
		}

		float realSwitchCooldown = 1.0f - (switchCooldownRemaining / switchCooldown);
		hudControllerScript->UpdateCooldowns(playerOnimaru.GetRealShieldCooldown(), playerOnimaru.GetRealBlastCooldown(), playerOnimaru.GetRealUltimateCooldown(), playerFang.GetRealDashCooldown(), playerFang.GetRealEMPCooldown(), playerFang.GetRealUltimateCooldown(), realSwitchCooldown);
	}
}

void PlayerController::TakeDamage(bool ranged) {
	if (!invincibleMode) {
		float damage = (ranged) ? rangedDamageTaken : meleeDamageTaken;
		if (playerFang.IsActive()) {
			playerFang.GetHit(damage);
		} else {
			playerOnimaru.GetHit(damage);
		}
		hitTaken = true;
	}
}

void PlayerController::AddEnemyInMap(GameObject* enemy) {
	playerOnimaru.AddEnemy(enemy);
}

void PlayerController::RemoveEnemyFromMap(GameObject* enemy) {
	playerOnimaru.RemoveEnemy(enemy);
}

void PlayerController::Update() {
	if (!playerFang.characterGameObject) return;
	if (!playerOnimaru.characterGameObject) return;
	if (!camera) return;

	if (playerFang.characterGameObject->IsActive()) {
		playerFang.Update();
	} else {
		playerOnimaru.Update();
	}

	if (playerFang.isAlive && playerOnimaru.isAlive) {
		CheckCoolDowns();
		UpdatePlayerStats();

		if (firstTime) {
			if (playerFang.characterGameObject->IsActive()) {
				hudControllerScript->UpdateHP(playerFang.lifePoints, playerOnimaru.lifePoints);
			} else {
				hudControllerScript->UpdateHP(playerOnimaru.lifePoints, playerFang.lifePoints);
			}
			firstTime = false;
		}
		if (CanSwitch()) {
			if (switchInProgress || (noCooldownMode && Input::GetKeyCodeUp(Input::KEYCODE::KEY_R))) {
				switchInProgress = true;
				SwitchCharacter();
			}
			if (!switchInProgress && Input::GetKeyCodeUp(Input::KEYCODE::KEY_R)) {
				switchInProgress = true;
				switchCooldownRemaining = switchCooldown;
			}
		}
	}
}
