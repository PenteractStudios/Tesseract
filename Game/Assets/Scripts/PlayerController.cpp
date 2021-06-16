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
#define MAX_ACCELERATION 9999

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
	MEMBER(MemberType::GAME_OBJECT_UID, onimaruGunUID),
	MEMBER(MemberType::GAME_OBJECT_UID, switchParticlesUID),
	MEMBER(MemberType::GAME_OBJECT_UID, canvasUID),
	MEMBER(MemberType::FLOAT, distanceRayCast),
	MEMBER(MemberType::FLOAT, switchCooldown),
	MEMBER(MemberType::FLOAT, dashCooldown),
	MEMBER(MemberType::FLOAT, dashSpeed),
	MEMBER(MemberType::FLOAT, dashDuration),
	MEMBER(MemberType::FLOAT, cameraOffsetZ),
	MEMBER(MemberType::FLOAT, cameraOffsetY),
	MEMBER(MemberType::FLOAT, cameraOffsetX),
	MEMBER(MemberType::INT, fangCharacter.lifePoints),
	MEMBER(MemberType::FLOAT, fangCharacter.movementSpeed),
	MEMBER(MemberType::INT, fangCharacter.damageHit),
	MEMBER(MemberType::FLOAT, fangCharacter.attackSpeed),
	MEMBER(MemberType::INT, onimaruCharacter.lifePoints),
	MEMBER(MemberType::FLOAT, onimaruCharacter.movementSpeed),
	MEMBER(MemberType::INT, onimaruCharacter.damageHit),
	MEMBER(MemberType::FLOAT, onimaruCharacter.attackSpeed),
	MEMBER(MemberType::INT, rangedDamageTaken),
	MEMBER(MemberType::INT, meleeDamageTaken),
	MEMBER(MemberType::BOOL, useSmoothCamera),
	MEMBER(MemberType::FLOAT, smoothCameraSpeed),
	MEMBER(MemberType::INT, onimaruRecoveryRate),
	MEMBER(MemberType::INT, fangRecoveryRate),
	MEMBER(MemberType::BOOL, debugGetHit),
	MEMBER(MemberType::FLOAT, switchDelay),
};

GENERATE_BODY_IMPL(PlayerController);

void PlayerController::Start() {
	player = GameplaySystems::GetGameObject(mainNodeUID);
	fang = GameplaySystems::GetGameObject(fangUID);
	onimaru = GameplaySystems::GetGameObject(onimaruUID);
	camera = GameplaySystems::GetGameObject(cameraUID);
	GameObject* canvasGO = GameplaySystems::GetGameObject(canvasUID);
	if (canvasGO) {
		hudControllerScript = GET_SCRIPT(canvasGO, HUDController);
	}

	//animation
	if (player) {
		transform = player->GetComponent<ComponentTransform>();
		if (transform) {
			initialPosition = transform->GetGlobalPosition();
		}
	}
	if (camera) {
		compCamera = camera->GetComponent<ComponentCamera>();
		if (compCamera) {
			GameplaySystems::SetRenderCamera(compCamera);
		}
		cameraTransform = camera->GetComponent<ComponentTransform>();
	}
	if (fang) {
		fang->Enable();

		ComponentCapsuleCollider* capsuleCollider = fang->GetComponent<ComponentCapsuleCollider>(); // workaround collider doesn't activate if onimaru starts disabled
		if (capsuleCollider) capsuleCollider->Enable();

		fangLeftGun = GameplaySystems::GetGameObject(fangLeftGunUID);
		if (fangLeftGun) {
			fangLeftGunTransform = fangLeftGun->GetComponent<ComponentTransform>();
		}

		fangRightGun = GameplaySystems::GetGameObject(fangRightGunUID);
		if (fangRightGun) {
			fangRightGunTransform = fangRightGun->GetComponent<ComponentTransform>();
		}
		
		fangAnimation = fang->GetComponent<ComponentAnimation>();
		fangTrail = GameplaySystems::GetResource<ResourcePrefab>(fangTrailUID);
		fangBullet = GameplaySystems::GetResource<ResourcePrefab>(fangBulletUID);

		if (fangAnimation) {
			fangCurrentState = fangAnimation->GetCurrentState();
		}
	}
	if (onimaru) {
		onimaru->Disable();
		onimaruGun = GameplaySystems::GetGameObject(onimaruGunUID);
		
		ComponentCapsuleCollider* capsuleCollider = onimaru->GetComponent<ComponentCapsuleCollider>(); // workaround collider doesn't activate if onimaru starts disabled
		if (capsuleCollider) capsuleCollider->Enable();
		
		if (onimaruGun) {
			onimaruGunTransform = onimaruGun->GetComponent<ComponentTransform>();
		}
		onimaruAnimation = onimaru->GetComponent<ComponentAnimation>();
		onimaruBullet = GameplaySystems::GetResource<ResourcePrefab>(onimaruBulletUID);
		if (onimaruAnimation) {
			onimaruCurrentState = onimaruAnimation->GetCurrentState();
		}
	}
	
	switchEffects = GameplaySystems::GetGameObject(switchParticlesUID);

	firstTime = true;

	agent = GetOwner().GetComponent<ComponentAgent>();
	if (agent) {
		agent->SetMaxSpeed(fangCharacter.movementSpeed);
		agent->SetMaxAcceleration(MAX_ACCELERATION);
	}

	//Get audio sources
	int i = 0;

	for (ComponentAudioSource& src : GetOwner().GetComponents<ComponentAudioSource>()) {
		if (i < static_cast<int>(AudioType::TOTAL)) audios[i] = &src;
		i++;
	}
}

void PlayerController::MoveTo(MovementDirection md) {
	float movementSpeed = ((fang->IsActive()) ? fangCharacter.movementSpeed : onimaruCharacter.movementSpeed);
	float3 newPosition = transform->GetGlobalPosition() + GetDirection(md);
	//with navigation
	agent->SetMaxSpeed(movementSpeed);
	agent->SetMoveTarget(newPosition, false);
}

void PlayerController::LookAtMouse() {
	if (camera && compCamera) {
		float2 mousePos = Input::GetMousePositionNormalized();
		LineSegment ray = compCamera->frustum.UnProjectLineSegment(mousePos.x, mousePos.y);
		float3 planeTransform = transform->GetGlobalPosition();
		if (fang->IsActive() && fangLeftGunTransform) planeTransform = fangLeftGunTransform->GetGlobalPosition();
		else if (!fang->IsActive() && onimaruGunTransform) planeTransform = onimaruGunTransform->GetGlobalPosition();
		Plane p = Plane(planeTransform, float3(0, 1, 0));
		facePointDir = float3(0, 0, 0);
		facePointDir = p.ClosestPoint(ray) - (transform->GetGlobalPosition());
		Quat quat = transform->GetGlobalRotation();
		float angle = Atan2(facePointDir.x, facePointDir.z);
		Quat rotation = quat.RotateAxisAngle(float3(0, 1, 0), angle);
		transform->SetGlobalRotation(rotation);
	}
}

void PlayerController::InitDash(MovementDirection md) {
	if (CanDash()) {

		if (hudControllerScript) {
			hudControllerScript->SetCooldownRetreival(HUDController::Cooldowns::FANG_SKILL_1);
		}

		if (md != MovementDirection::NONE) {
			dashDirection = GetDirection(md);
			dashMovementDirection = md;
		} else {
			dashDirection = facePointDir;
		}

		dashCooldownRemaining = dashCooldown;
		dashRemaining = dashDuration;
		dashInCooldown = true;
		dashing = true;
		if (agent) {
			agent->SetMaxSpeed(dashSpeed);
		}
		if (audios[static_cast<int>(AudioType::DASH)]) {
			audios[static_cast<int>(AudioType::DASH)]->Play();
		} else {
			Debug::Log(AUDIOSOURCE_NULL_MSG);
		}
	}
}

void PlayerController::Dash() {
	if (dashing) {
		float3 newPosition = transform->GetGlobalPosition();
		newPosition += dashSpeed * dashDirection;
		agent->SetMoveTarget(newPosition, false);
	}
}

bool PlayerController::CanDash() {
	return !dashing && !dashInCooldown && fang->IsActive();
}

bool PlayerController::CanSwitch() {
	return !switchInCooldown;
}

void PlayerController::SwitchCharacter() {
	if (!fang) return;
	if (!onimaru) return;
	if (!agent) return;
	if (CanSwitch()) {
		bool doVisualSwitch = currentSwitchDelay < switchDelay ? false : true;
		if (doVisualSwitch) {
			if (audios[static_cast<int>(AudioType::SWITCH)]) {
				audios[static_cast<int>(AudioType::SWITCH)]->Play();
			}
			if (fang->IsActive()) {
				fang->Disable();
				onimaru->Enable();

				ComponentCapsuleCollider* capsuleCollider = onimaru->GetComponent<ComponentCapsuleCollider>(); // workaround collider doesn't activate if onimaru starts disabled
				if (capsuleCollider) capsuleCollider->Enable();

				if (hudControllerScript){
					hudControllerScript->UpdateHP(static_cast<float>(onimaruCharacter.lifePoints), static_cast<float>(fangCharacter.lifePoints));
					hudControllerScript->ResetHealthRegenerationEffects(static_cast<float>(fangCharacter.lifePoints));
				}
				
				fangRecovering = 0.0f;
			}
			else {
				onimaru->Disable();
				fang->Enable();

				ComponentCapsuleCollider* capsuleCollider = fang->GetComponent<ComponentCapsuleCollider>(); // workaround collider doesn't activate if onimaru starts disabled
				if (capsuleCollider) capsuleCollider->Enable();

				if(hudControllerScript){
					hudControllerScript->UpdateHP(static_cast<float>(fangCharacter.lifePoints), static_cast<float>(onimaruCharacter.lifePoints));
					hudControllerScript->ResetHealthRegenerationEffects(static_cast<float>(onimaruCharacter.lifePoints));
				}
			
				onimaruRecovering = 0.0f;
			}
			if (hudControllerScript) {
				hudControllerScript->ChangePlayerHUD(static_cast<float>(fangCharacter.lifePoints), static_cast<float>(onimaruCharacter.lifePoints));
				hudControllerScript->ResetCooldownProgressBar();
			}
			currentSwitchDelay = 0.f;
			playSwitchParticles = true;
			switchInCooldown = true;
			if (noCooldownMode) switchInProgress = false;
		}
		else {
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
}

bool PlayerController::CanShoot() {
	return !shooting && ((fang->IsActive() && fangTrail) || (onimaru->IsActive() && onimaruBullet));
}

void PlayerController::ResetSwitchStatus() {
	switchInProgress = false;
	playSwitchParticles = true;
	currentSwitchDelay = 0.f;
}

void PlayerController::Shoot() {
	if (CanShoot()) {
		shooting = true;
		if (fang->IsActive()) {
			fangAttackCooldownRemaining = 1.f / fangCharacter.attackSpeed;
			if (audios[static_cast<int>(AudioType::SHOOT)]) {
				audios[static_cast<int>(AudioType::SHOOT)]->Play();
			}
			else {
				Debug::Log(AUDIOSOURCE_NULL_MSG);
			}
			fangAttackCooldownRemaining = 1.f / fangCharacter.attackSpeed;
			ComponentTransform* shootingGunTransform = nullptr;
			if (rightShot) {
				fangAnimation->SendTriggerSecondary(fangAnimation->GetCurrentState()->name + PlayerController::states[12]);
				shootingGunTransform = fangRightGunTransform;
			}
			else {
				fangAnimation->SendTriggerSecondary(fangAnimation->GetCurrentState()->name + PlayerController::states[11]);
				shootingGunTransform = fangLeftGunTransform;
			}
			if (fangTrail && fangBullet && shootingGunTransform) {
				GameplaySystems::Instantiate(fangBullet, shootingGunTransform->GetGlobalPosition(), transform->GetGlobalRotation());
				GameplaySystems::Instantiate(fangTrail, shootingGunTransform->GetGlobalPosition(), transform->GetGlobalRotation());
			}
		}
		else {
			onimaruAttackCooldownRemaining = 1.f / onimaruCharacter.attackSpeed;
			if (audios[static_cast<int>(AudioType::SHOOT)]) {
				audios[static_cast<int>(AudioType::SHOOT)]->Play();
			}
			else {
				Debug::Log(AUDIOSOURCE_NULL_MSG);
			}
			onimaruAttackCooldownRemaining = 1.f / onimaruCharacter.attackSpeed;
			if (onimaruBullet) {
				GameObject* bullet = GameplaySystems::Instantiate(onimaruBullet, onimaruGunTransform->GetGlobalPosition(), Quat(0.0f, 0.0f, 0.0f, 0.0f));
				if (bullet) {
					onimaruBulletcript = GET_SCRIPT(bullet, OnimaruBullet);
					if(onimaruBulletcript) onimaruBulletcript->SetOnimaruDirection(onimaruGunTransform->GetGlobalRotation());
				}
			}
		}
	}
}

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

bool PlayerController::IsDead() {
	return (!fangCharacter.isAlive || !onimaruCharacter.isAlive);
}

void PlayerController::CheckCoolDowns() {
	if (noCooldownMode || switchCooldownRemaining <= 0.f) {
		switchCooldownRemaining = 0.f;
		switchInCooldown = false;
		if (!noCooldownMode) switchInProgress = false;
	}
	else {
		switchCooldownRemaining -= Time::GetDeltaTime();
	}
	//Dash Cooldown
	if (noCooldownMode || dashCooldownRemaining <= 0.f) {
		dashCooldownRemaining = 0.f;
		dashInCooldown = false;
		dashMovementDirection = MovementDirection::NONE;
	}
	else {
		dashCooldownRemaining -= Time::GetDeltaTime();
	}
	//Dash duration
	if (dashRemaining <= 0.f) {
		dashRemaining = 0.f;
		dashing = false;
		if (agent) agent->SetMaxSpeed(fangCharacter.movementSpeed);
	}
	else {
		dashRemaining -= Time::GetDeltaTime();
	}

	if (fang->IsActive()) {
		if (fangAttackCooldownRemaining <= 0.f) {
			fangAttackCooldownRemaining = 0.f;
			shooting = false;
		} else {
			fangAttackCooldownRemaining -= Time::GetDeltaTime();
		}
	}
	else {
		if (onimaruAttackCooldownRemaining <= 0.f) {
			onimaruAttackCooldownRemaining = 0.f;
			shooting = false;
		} else {
			onimaruAttackCooldownRemaining -= Time::GetDeltaTime();
		}
	}

	if (onimaru->IsActive() && fangCharacter.lifePoints != FANG_MAX_HEALTH) {
		if (fangRecovering >= fangRecoveryRate) {
			fangCharacter.Recover(1);
			fangRecovering = 0.0f;
		} else {
			fangRecovering += Time::GetDeltaTime();
		}
	}

	if (fang->IsActive() && onimaruCharacter.lifePoints != ONIMARU_MAX_HEALTH) {
		if (onimaruRecovering >= onimaruRecoveryRate) {
			onimaruCharacter.Recover(1);
			onimaruRecovering = 0.0f;
		} else {
			onimaruRecovering += Time::GetDeltaTime();
		}
	}
}

MovementDirection PlayerController::GetInputMovementDirection() const {
	MovementDirection md = MovementDirection::NONE;
	if (Input::GetKeyCode(Input::KEYCODE::KEY_W)) {
		md = MovementDirection::UP;
	}

	if (Input::GetKeyCode(Input::KEYCODE::KEY_S)) {
		md = MovementDirection::DOWN;
	}

	if (Input::GetKeyCode(Input::KEYCODE::KEY_A)) {
		if (md == MovementDirection::UP) md = MovementDirection::UP_LEFT;
		else if (md == MovementDirection::DOWN) md = MovementDirection::DOWN_LEFT;
		else md = MovementDirection::LEFT;
	}

	if (Input::GetKeyCode(Input::KEYCODE::KEY_D)) {
		if (md == MovementDirection::UP) md = MovementDirection::UP_RIGHT;
		else if (md == MovementDirection::DOWN) md = MovementDirection::DOWN_RIGHT;
		else md = MovementDirection::RIGHT;
	}
	return md;
}

float3 PlayerController::GetDirection(MovementDirection md) const {
	float3 direction;
	switch (md) {
	case MovementDirection::UP:
		direction = float3(0, 0, -1);
		break;
	case MovementDirection::UP_LEFT:
		direction = float3(-0.5, 0, -0.5);
		break;
	case MovementDirection::UP_RIGHT:
		direction = float3(0.5, 0, -0.5);
		break;
	case MovementDirection::DOWN:
		direction = float3(0, 0, 1);
		break;
	case MovementDirection::DOWN_LEFT:
		direction = float3(-0.5, 0, 0.5);
		break;
	case MovementDirection::DOWN_RIGHT:
		direction = float3(0.5, 0, 0.5);
		break;
	case MovementDirection::RIGHT:
		direction = float3(1, 0, 0);
		break;
	case MovementDirection::LEFT:
		direction = float3(-1, 0, 0);
		break;
	default:
		return float3(0, 0, 0);
	}
	return direction.Normalized();
}

int PlayerController::GetMouseDirectionState(MovementDirection input) {
	float3 inputDirection = GetDirection(input);
	float dot = Dot(inputDirection.Normalized(), facePointDir.Normalized());
	float3 cross = Cross(inputDirection.Normalized(), facePointDir.Normalized());

	if (dot > 0.707) {
		return 2; //RunForward
	} else if (dot < -0.707) {
		return 1; //RunBackward
	} else if (cross.y > 0) {
		return 4; //RunRight
	} else {
		return 3; //RunLeft
	}
}

void PlayerController::PlayAnimation(MovementDirection md) {
	ComponentAnimation* animation = nullptr;
	if (fang->IsActive()) {
		animation = fangAnimation;
	}
	else {
		animation = onimaruAnimation;
	}

	int dashAnimation = 0;
	if (dashing) {
		dashAnimation = 4;
		md = dashMovementDirection;
	}

	if (!animation) return;

	if (md == MovementDirection::NONE) {
		if (IsDead()) {
			if (animation->GetCurrentState()->name != PlayerController::states[9]) {
				animation->SendTrigger(animation->GetCurrentState()->name + PlayerController::states[9]);
				if (fang->IsActive()) {
					if (animation->GetCurrentStateSecondary()->name == "RightShot") {
						animation->SendTriggerSecondary("RightShotDeath");
					} else if (animation->GetCurrentStateSecondary()->name == "LeftShot") {
						animation->SendTriggerSecondary("LeftShotDeath");
					}
				}
				else {
					animation->SendTriggerSecondary("ShootingDeath");
				}
			}
		}
		else {
			if (animation->GetCurrentState()->name != PlayerController::states[0]) {
				animation->SendTrigger(animation->GetCurrentState()->name + PlayerController::states[0]);
			}
		}
	}
	else {
		if (animation->GetCurrentState()->name != PlayerController::states[GetMouseDirectionState(md) + dashAnimation]) {
			animation->SendTrigger(animation->GetCurrentState()->name + PlayerController::states[GetMouseDirectionState(md) + dashAnimation]);
		}
	}
}

void PlayerController::UpdatePlayerStats() {
	if (hudControllerScript) {
		if (firstTime) {
			hudControllerScript->UpdateHP(static_cast<float>(fangCharacter.lifePoints), static_cast<float>(onimaruCharacter.lifePoints));
			firstTime = false;
		}

		if (hitTaken && fang->IsActive() && fangCharacter.lifePoints >= 0) {
			hudControllerScript->UpdateHP(static_cast<float>(fangCharacter.lifePoints), static_cast<float>(onimaruCharacter.lifePoints));
			hitTaken = false;
		}
		else if (hitTaken && onimaru->IsActive() && onimaruCharacter.lifePoints >= 0) {
			hudControllerScript->UpdateHP(static_cast<float>(onimaruCharacter.lifePoints), static_cast<float>(fangCharacter.lifePoints));
			hitTaken = false;
		}

		if (fang->IsActive() && onimaruCharacter.lifePoints != ONIMARU_MAX_HEALTH) {
			float healthRecovered = (onimaruRecovering / onimaruRecoveryRate);
			if (hudControllerScript){
				hudControllerScript->HealthRegeneration(onimaruCharacter.lifePoints, healthRecovered);
			}
		} else if (onimaru->IsActive() && fangCharacter.lifePoints != FANG_MAX_HEALTH) {
			float healthRecovered = (fangRecovering / fangRecoveryRate);
			if (hudControllerScript){
				hudControllerScript->HealthRegeneration(fangCharacter.lifePoints, healthRecovered);
			}
		}

		float realDashCooldown = 1.0f - (dashCooldownRemaining / dashCooldown);
		float realSwitchCooldown = 1.0f - (switchCooldownRemaining / switchCooldown);
		hudControllerScript->UpdateCooldowns(0.0f, 0.0f, 0.0f, realDashCooldown, 0.0f, 0.0f, realSwitchCooldown);

		if (IsDead()) {
			PlayAnimation(MovementDirection::NONE);
		}
	}
}

void PlayerController::UpdateCameraPosition() {
	float3 playerGlobalPos = transform->GetGlobalPosition();

	float3 desiredPosition = playerGlobalPos + float3(cameraOffsetX, cameraOffsetY, cameraOffsetZ);
	float3 smoothedPosition = desiredPosition;

	if (useSmoothCamera) {
		smoothedPosition = float3::Lerp(cameraTransform->GetGlobalPosition(), desiredPosition, smoothCameraSpeed * Time::GetDeltaTime());
	}

	cameraTransform->SetGlobalPosition(smoothedPosition);
}

void PlayerController::TakeDamage(bool ranged) {
	if (!fang || !onimaru || invincibleMode) return;
	if (fang->IsActive()) {
		fangCharacter.Hit((ranged) ? rangedDamageTaken : meleeDamageTaken);
		if (audios[static_cast<int>(AudioType::FANGHIT)]) audios[static_cast<int>(AudioType::FANGHIT)]->Play();
		if (!fangCharacter.isAlive) {
			if (audios[static_cast<int>(AudioType::FANGDEATH)]) audios[static_cast<int>(AudioType::FANGDEATH)]->Play();
		}
	}
	else {
		onimaruCharacter.Hit((ranged) ? rangedDamageTaken : meleeDamageTaken);
		if (audios[static_cast<int>(AudioType::ONIHIT)]) audios[static_cast<int>(AudioType::ONIHIT)]->Play();
		if (!onimaruCharacter.isAlive) {
			if (audios[static_cast<int>(AudioType::ONIDEATH)]) audios[static_cast<int>(AudioType::ONIDEATH)]->Play();
		}
	}
	hitTaken = true;
}

void PlayerController::Update() {
	if (!player) return;
	if (!camera) return;
	if (!transform) return;
	if (!agent) return;

	CheckCoolDowns();
	UpdatePlayerStats();

	if (!IsDead()) {
		Dash();
		UpdateCameraPosition();

		if (firstTime) {
			if (fang->IsActive()) {
				hudControllerScript->UpdateHP(static_cast<float>(fangCharacter.lifePoints), static_cast<float>(onimaruCharacter.lifePoints));
			}
			else {
				hudControllerScript->UpdateHP(static_cast<float>(onimaruCharacter.lifePoints), static_cast<float>(fangCharacter.lifePoints));
			}
			firstTime = false;
		}

		MovementDirection md;
		md = GetInputMovementDirection();
		if (Input::GetMouseButtonDown(2)) {
			InitDash(md);
		}
		if (!dashing) {
			LookAtMouse();
			MoveTo(md);

			if (switchInProgress || (noCooldownMode && Input::GetKeyCodeUp(Input::KEYCODE::KEY_R))) {
				switchInProgress = true;
				SwitchCharacter();
			}

			if (!switchInProgress && Input::GetKeyCodeUp(Input::KEYCODE::KEY_R)) {
				switchInProgress = true;
				switchCooldownRemaining = switchCooldown;
			}
		}
		if (fang->IsActive()) {
			if (Input::GetMouseButtonDown(0)) Shoot();
		}
		else {
			if (Input::GetMouseButtonDown(0)) {
				onimaruAnimation->SendTriggerSecondary(onimaruAnimation->GetCurrentState()->name + PlayerController::states[13]);
			}
			else if (Input::GetMouseButtonRepeat(0)) {
				Shoot();
			}
			else if (Input::GetMouseButtonUp(0)) {
				if (onimaruAnimation) {
					onimaruAnimation->SendTriggerSecondary(PlayerController::states[13] + onimaruAnimation->GetCurrentState()->name);
				}
			}
		}
		PlayAnimation(md);
	}
	else {
		if(agent) agent->RemoveAgentFromCrowd();
	}
}