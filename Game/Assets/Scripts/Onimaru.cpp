#include "Onimaru.h"

#include "GameplaySystems.h"
#include "OnimaruBullet.h"
#include "AIMeleeGrunt.h"
#include "RangedAI.h"
#include "HUDController.h"
#include "Shield.h"
#include "CameraController.h"

bool Onimaru::CanShoot() {
	return !shootingOnCooldown;
}

bool Onimaru::CanBlast() {
	return !blastInCooldown && !IsShielding() && !ultimateOn && !blastInUse;
}

void Onimaru::GetHit(float damage_) {
	//We assume that the player is always alive when this method gets called, so no need to check if character was alive before taking lives



	if (cameraController) {
		cameraController->StartShake();
	}

	lifePoints -= damage_;
	if (onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::HIT)]) onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::HIT)]->Play();
	isAlive = lifePoints > 0.0f;

	if (!isAlive) {
		if (onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::DEATH)]) onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::DEATH)]->Play();
		OnDeath();
	}
}

void Onimaru::IncreaseUltimateCounter() {
	if (!ultimateOn) ++ultimateChargePoints;
}

void Onimaru::Shoot() {
	if (CanShoot()) {
		shootingOnCooldown = true;
		//	shooting = true;
		attackCooldownRemaining = 1.f / attackSpeed;

		//NullRef on ultimate values
		if (onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::SPECIAL_SHOOT)] == nullptr) {
			onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::SPECIAL_SHOOT)] = onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::SHOOT)];
		}
		if (ultimateBullet == nullptr) {
			ultimateBullet = bullet;
		}

		ComponentAudioSource* audioSourceToPlay = !ultimateOn ? onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::SHOOT)] : onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::SPECIAL_SHOOT)];

		if (audioSourceToPlay) {
			audioSourceToPlay->Play();
		}

	}
}

void Onimaru::Blast() {
	bool releaseBlast = currentBlastDuration <= blastDelay ? false : true;
	if (releaseBlast && calculateEnemiesInRange) {
		if (blastParticles) blastParticles->PlayChildParticles();
		calculateEnemiesInRange = false;
		for (GameObject* enemy : enemiesInMap) {
			AIMeleeGrunt* meleeScript = GET_SCRIPT(enemy, AIMeleeGrunt);
			RangedAI* rangedScript = GET_SCRIPT(enemy, RangedAI);
			if (rangedScript || meleeScript) {
				if (rightHand && playerMainTransform) {
					float3 onimaruRightArmPos = rightHand->GetGlobalPosition();
					float3 enemyPos = enemy->GetComponent<ComponentTransform>()->GetGlobalPosition();
					onimaruRightArmPos = float3(onimaruRightArmPos.x, 0.f, onimaruRightArmPos.z);
					enemyPos = float3(enemyPos.x, 0.f, enemyPos.z);
					float distance = enemyPos.Distance(onimaruRightArmPos);
					float3 direction = (enemyPos - onimaruRightArmPos).Normalized();
					if (distance <= blastDistance) {
						float angle = 0.f;
						angle = RadToDeg(playerMainTransform->GetFront().AngleBetweenNorm(direction));
						if (angle <= blastAngle / 2.0f) {
							if (meleeScript) meleeScript->EnableBlastPushBack();
							else if (rangedScript) rangedScript->EnableBlastPushBack();
						}
					} else {
						if (meleeScript) {
							if (!meleeScript->IsBeingPushed()) meleeScript->DisableBlastPushBack();
						} else if (rangedScript) {
							if (!rangedScript->IsBeingPushed()) rangedScript->DisableBlastPushBack();
						}
					}
				}
			}
		}
	} else {
		currentBlastDuration += Time::GetDeltaTime();
		if (currentBlastDuration >= 1.8f) OnAnimationSecondaryFinished(); // Temporary hack
	}
}

void Onimaru::PlayAnimation() {
	if (!compAnimation) return;
	if (ultimateOn || !isAlive) return; //Ultimate will block out all movement and idle from happening

	if (compAnimation->GetCurrentState()) {
		if (movementInputDirection == MovementDirection::NONE) {
			//Primery state machine idle when alive, without input movement
			if (compAnimation->GetCurrentState()->name != states[static_cast<int>(IDLE)]) {
				compAnimation->SendTrigger(compAnimation->GetCurrentState()->name + states[static_cast<int>(IDLE)]);
			}
		} else {
			//If Movement is found, Primary state machine will be in charge of getting movement animations
			if (compAnimation->GetCurrentState()->name != (states[GetMouseDirectionState()])) {
				compAnimation->SendTrigger(compAnimation->GetCurrentState()->name + states[GetMouseDirectionState()]);
			}
		}
	}
}

void Onimaru::StartUltimate() {
	if (!compAnimation) return;
	if (compAnimation->GetCurrentState()) {
		if (compAnimation->GetCurrentStateSecondary()) {
			if (compAnimation->GetCurrentStateSecondary()->name != compAnimation->GetCurrentState()->name) {
				compAnimation->SendTriggerSecondary(compAnimation->GetCurrentStateSecondary()->name + compAnimation->GetCurrentState()->name);
			}
		}
		compAnimation->SendTrigger(compAnimation->GetCurrentState()->name + states[static_cast<int>(ULTI_INTRO)]);
	}

	if (onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::ULTIMATE)] == nullptr) {
		if (!onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::ULTIMATE)]->IsPlaying()) {
			onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::ULTIMATE)]->Play();
		}
	}

	ultimateChargePoints = 0;
	orientationSpeed = ultimateOrientationSpeed;
	attackSpeed = ultimateAttackSpeed;
	movementInputDirection = MovementDirection::NONE;
	Player::MoveTo();
	ultimateOn = true;
}

void Onimaru::FinishUltimate() {
	ultimateTimeRemaining = 0;
	orientationSpeed = normalOrientationSpeed;
	attackSpeed = originalAttackSpeed;
	ultimateOn = false;
	compAnimation->SendTrigger(compAnimation->GetCurrentState()->name + states[static_cast<int>(IDLE)]);
}

bool Onimaru::IsShielding() const {
	return shield->GetIsActive();
}

bool Onimaru::IsVulnerable() const {
	return !ultimateOn;
}

float Onimaru::GetRealShieldCooldown() {
	if (shield == nullptr || shieldGO == nullptr) return 0.0f;
	float realShieldCooldown = 1.0f;
	float chargesWasted = (float)(shield->max_charges - shield->GetNumCharges()) / (float)shield->max_charges;
	if (shield->GetIsActive()) {
		realShieldCooldown = chargesWasted;
	} else if (shield->GetCoolDown() > 0) {
		realShieldCooldown = 1.0f - (shieldCooldownRemaining / (shield->GetCoolDown() / (1.0f - chargesWasted)));
	}

	return realShieldCooldown;
}

void Onimaru::CheckCoolDowns(bool noCooldownMode) {
	//AttackCooldown
	if (attackCooldownRemaining <= 0.f) {
		attackCooldownRemaining = 0.f;
		shootingOnCooldown = false;
	} else {
		attackCooldownRemaining -= Time::GetDeltaTime();
	}

	if (noCooldownMode) {
		ultimateChargePoints = ultimateChargePointsTotal;
	}
	//Blast Cooldown
	if (noCooldownMode || blastCooldownRemaining <= 0.f) {
		blastCooldownRemaining = 0.f;
		blastInCooldown = false;
	} else {
		if (!blastInUse) blastCooldownRemaining -= Time::GetDeltaTime();
	}
	//ShieldCooldown
	if (noCooldownMode || shieldCooldownRemaining <= 0.f) {
		shieldCooldownRemaining = 0.f;
		shieldInCooldown = false;
	} else {
		shieldCooldownRemaining -= Time::GetDeltaTime();
	}
}

void Onimaru::OnDeath() {
	if (compAnimation == nullptr) return;
	if (compAnimation->GetCurrentState()) {
		if (compAnimation->GetCurrentState()->name != states[static_cast<int>(DEATH)]) {
			if (compAnimation->GetCurrentStateSecondary()) {
				compAnimation->SendTriggerSecondary(compAnimation->GetCurrentStateSecondary()->name + compAnimation->GetCurrentState()->name);
			}
			compAnimation->SendTrigger(compAnimation->GetCurrentState()->name + states[static_cast<int>(DEATH)]);
		}
	}
	ultimateOn = blastInUse = false;
}

bool Onimaru::CanSwitch() const {
	return ultimateTimeRemaining <= 0 && !ultimateOn && !IsShielding() && !blastInUse;
}

void Onimaru::OnAnimationSecondaryFinished() {
	if (compAnimation) {
		if (blastInUse) {
			blastInUse = false;
			blastInCooldown = true;
			currentBlastDuration = 0.f;
			calculateEnemiesInRange = true;
			if (compAnimation) {
				if (compAnimation->GetCurrentStateSecondary() && compAnimation->GetCurrentState()) {
					compAnimation->SendTriggerSecondary(compAnimation->GetCurrentStateSecondary()->name + compAnimation->GetCurrentState()->name);
				}
			}
		}
	}
}

bool Onimaru::IsInstantOrientation(bool useGamepad) const {
	//This must return true only when ultimate not in use and Gamepad is either not used or not connected
	return !ultimateOn && (!useGamepad || !Input::IsGamepadConnected(0));
}

void Onimaru::OnAnimationEvent(StateMachineEnum stateMachineEnum, const char* eventName) {
	if (stateMachineEnum == StateMachineEnum::PRINCIPAL) {
		if (std::strcmp(eventName, "FootstepRight")) {
			if (onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::FOOTSTEP_RIGHT)]) {
				onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::FOOTSTEP_RIGHT)]->Play();
			}
		} else if (std::strcmp(eventName, "FootstepLeft")) {
			if (onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::FOOTSTEP_LEFT)]) {
				onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::FOOTSTEP_LEFT)]->Play();
			}
		}
	}
}

void Onimaru::Init(UID onimaruUID, UID onimaruBulletUID, UID onimaruGunUID, UID onimaruRightHandUID, UID shieldUID, UID onimaruUltimateBulletUID, UID onimaruBlastEffectsUID, UID cameraUID, UID canvasUID, float maxSpread_) {
	SetTotalLifePoints(lifePoints);
	characterGameObject = GameplaySystems::GetGameObject(onimaruUID);
	if (characterGameObject && characterGameObject->GetParent()) {
		playerMainTransform = characterGameObject->GetParent()->GetComponent<ComponentTransform>();
		agent = characterGameObject->GetParent()->GetComponent<ComponentAgent>();
		compAnimation = characterGameObject->GetComponent<ComponentAnimation>();

		GameObject* cameraAux = GameplaySystems::GetGameObject(cameraUID);
		if (cameraAux) {
			lookAtMouseCameraComp = cameraAux->GetComponent<ComponentCamera>();
			cameraController = GET_SCRIPT(cameraAux, CameraController);
		}

		if (agent) {
			agent->SetMaxSpeed(movementSpeed);
			agent->SetMaxAcceleration(MAX_ACCELERATION);
		}

		GameObject* bulletAux = GameplaySystems::GetGameObject(onimaruBulletUID);
		if (bulletAux) {
			bullet = bulletAux->GetComponent<ComponentParticleSystem>();
			if (bullet) {
				bullet->SetParticlesPerSecond(float2(0.0f, 0.0f));
				bullet->Play();
			}
		}

		GameObject* ultimateBulletAux = GameplaySystems::GetGameObject(onimaruUltimateBulletUID);
		if (bulletAux) {
			ultimateBullet = ultimateBulletAux->GetComponent<ComponentParticleSystem>();
		}
	}

	originalAttackSpeed = attackSpeed;

	GameObject* onimaruGun = GameplaySystems::GetGameObject(onimaruGunUID);
	if (onimaruGun) {
		gunTransform = onimaruGun->GetComponent<ComponentTransform>();
		lookAtMousePlanePosition = gunTransform->GetGlobalPosition();
	}

	GameObject* canvasGO = GameplaySystems::GetGameObject(canvasUID);
	if (canvasGO) {
		hudControllerScript = GET_SCRIPT(canvasGO, HUDController);
	}

	shieldGO = GameplaySystems::GetGameObject(shieldUID);
	if (shieldGO) {
		shield = GET_SCRIPT(shieldGO, Shield);
		shieldGO->Disable();
	}

	if (characterGameObject) {
		//Get audio sources
		int i = 0;

		for (ComponentAudioSource& src : characterGameObject->GetComponents<ComponentAudioSource>()) {
			if (i < static_cast<int>(ONIMARU_AUDIOS::TOTAL)) onimaruAudios[i] = &src;
			i++;
		}
	}

	GameObject* rightHandGO = GameplaySystems::GetGameObject(onimaruRightHandUID);
	if (rightHandGO) rightHand = rightHandGO->GetComponent<ComponentTransform>();

	GameObject* blastParticlesGO = GameplaySystems::GetGameObject(onimaruBlastEffectsUID);
	if (blastParticlesGO) blastParticles = blastParticlesGO->GetComponent<ComponentParticleSystem>();
}

void Onimaru::OnAnimationFinished() {
	if (!compAnimation)return;
	if (ultimateOn) {
		ultimateTimeRemaining = ultimateTotalTime;
		if (compAnimation->GetCurrentState()) {
			compAnimation->SendTrigger(compAnimation->GetCurrentState()->name + states[static_cast<int>(ULTI_LOOP)]);
		}
	}
}

bool Onimaru::CanShield() {
	if (shield == nullptr || shieldGO == nullptr) return false;

	return !shieldInCooldown && !shield->GetIsActive() && !ultimateOn;
}

bool Onimaru::CanUltimate() {
	return !blastInUse && !IsShielding() && ultimateChargePoints >= ultimateChargePointsTotal;
}

void Onimaru::InitShield() {
	if (shield == nullptr || shieldGO == nullptr) return;

	if (CanShield()) {
		shield->InitShield();
		if (shieldParticles) {
			shieldParticles->Play();
		}

		shieldInCooldown = false;
		if (agent) {
			agent->SetMaxSpeed(movementSpeed / 2);
		}
		if (!shooting) {
			if (compAnimation->GetCurrentState()) {
				compAnimation->SendTriggerSecondary(compAnimation->GetCurrentState()->name + states[static_cast<int>(SHIELD)]);
			}
		} else {
			if (compAnimation->GetCurrentStateSecondary()) {
				compAnimation->SendTriggerSecondary(compAnimation->GetCurrentStateSecondary()->name + states[static_cast<int>(SHOOTSHIELD)]);
			}
		}
		if (onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::SHIELD_ON)]) {
			onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::SHIELD_ON)]->Play();
		}
		shieldGO->Enable();
	}
}

void Onimaru::FadeShield() {
	if (shield == nullptr || shieldGO == nullptr) return;
	shield->FadeShield();
	shieldInCooldown = true;
	shieldCooldownRemaining = shield->GetCoolDown();
	if (agent) agent->SetMaxSpeed(movementSpeed);

	if (!shooting) {
		if (compAnimation->GetCurrentStateSecondary() && compAnimation->GetCurrentState()) {
			compAnimation->SendTriggerSecondary(compAnimation->GetCurrentStateSecondary()->name + compAnimation->GetCurrentState()->name);
		}
	} else {
		if (compAnimation->GetCurrentStateSecondary()) {
			compAnimation->SendTriggerSecondary(compAnimation->GetCurrentStateSecondary()->name + states[static_cast<int>(SHOOTING)]);
		}
	}
	if (onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::SHIELD_OFF)]) {
		onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::SHIELD_OFF)]->Play();
	}
	shieldGO->Disable();
}

void Onimaru::Update(bool useGamepad, bool lockMovement, bool lockRotation) {
	if (shield == nullptr || shieldGO == nullptr) return;
	if (isAlive) {
		Player::Update(useGamepad, ultimateOn, false);

		if (!ultimateOn) {
			if (GetInputBool(InputActions::ABILITY_3, useGamepad)) {
				if (CanUltimate()) {
					StartUltimate();
				}
			}
		} else {

			//Ultimate execution
			if (ultimateTimeRemaining > 0) {
				ultimateTimeRemaining -= Time::GetDeltaTime();
				Shoot();
				shooting = true;
				ultimateBullet->PlayChildParticles();
				bullet->SetParticlesPerSecond(float2(0.0f, 0.0f));

				if (ultimateTimeRemaining <= 0) {
					FinishUltimate();
					ultimateBullet->StopChildParticles();
				}
			}
		}

		if (!ultimateOn && !blastInUse) {

			if (!shield->GetIsActive()) {
				if (GetInputBool(InputActions::ABILITY_1, useGamepad)) {
					InitShield();
				}
			} else {
				if (!GetInputBool(InputActions::ABILITY_1, useGamepad) || shield->GetNumCharges() == shield->max_charges) {
					FadeShield();
				}
			}

			if (GetInputBool(InputActions::SHOOT, useGamepad)) {
				if (!shooting) {
					shooting = true;
					if (bullet) {
						bullet->Play();
						bullet->SetParticlesPerSecond(float2(attackSpeed, attackSpeed));
					}
					if (compAnimation) {
						if (!shield->GetIsActive()) {
							if (compAnimation->GetCurrentState()) {
								compAnimation->SendTriggerSecondary(compAnimation->GetCurrentState()->name + states[static_cast<int>(SHOOTING)]);
							}
						} else {
							if (compAnimation->GetCurrentStateSecondary()) {
								compAnimation->SendTriggerSecondary(compAnimation->GetCurrentStateSecondary()->name + states[static_cast<int>(SHOOTSHIELD)]);
							}
						}
					}
				} else {
					Shoot();
				}
			}
		}

		if (shooting) {
			if (!GetInputBool(InputActions::SHOOT, useGamepad)) {
				shooting = false;
				if (compAnimation) {
					if (shield->GetIsActive()) {
						if (compAnimation->GetCurrentState() && compAnimation->GetCurrentStateSecondary()) {
							compAnimation->SendTriggerSecondary(compAnimation->GetCurrentStateSecondary()->name + states[static_cast<int>(SHIELD)]);
						}
					} else {
						if (compAnimation->GetCurrentStateSecondary() && compAnimation->GetCurrentState()) {
							compAnimation->SendTriggerSecondary(compAnimation->GetCurrentStateSecondary()->name + compAnimation->GetCurrentState()->name);
						}
					}
					if (bullet) bullet->SetParticlesPerSecond(float2(0.0f, 0.0f));
				}
			}
		}

		if (CanBlast()) {
			if (GetInputBool(InputActions::ABILITY_2, useGamepad)) {
				blastInUse = true;
				if (shooting) {
					if (compAnimation) {
						if (compAnimation->GetCurrentState() && compAnimation->GetCurrentStateSecondary()) {
							compAnimation->SendTriggerSecondary(compAnimation->GetCurrentStateSecondary()->name + compAnimation->GetCurrentState()->name);
						}
					}
				}
				if (hudControllerScript) {
					hudControllerScript->SetCooldownRetreival(HUDController::Cooldowns::ONIMARU_SKILL_2);
				}
				blastCooldownRemaining = blastCooldown;
				if (compAnimation) {
					if (compAnimation->GetCurrentState()) {
						compAnimation->SendTriggerSecondary(compAnimation->GetCurrentState()->name + states[static_cast<int>(BLAST)]);
					}
				}
				Blast();
			}
		}
	} else {
		if (agent) agent->RemoveAgentFromCrowd();
		movementInputDirection = MovementDirection::NONE;
	}

	if (blastInUse) {
		Blast();
	}
	PlayAnimation();
}

float Onimaru::GetRealUltimateCooldown() {
	return static_cast<float>(ultimateChargePoints) / static_cast<float>(ultimateChargePointsTotal);
}

float Onimaru::GetRealBlastCooldown() {
	return 1.f - (blastCooldownRemaining / blastCooldown);
}

void Onimaru::AddEnemy(GameObject* enemy) {
	enemiesInMap.push_back(enemy);
}

void Onimaru::RemoveEnemy(GameObject* enemy) {
	std::vector<GameObject*>::iterator enemyToRemove;
	enemyToRemove = std::find(enemiesInMap.begin(), enemiesInMap.end(), enemy);

	if (enemyToRemove != enemiesInMap.end()) {
		enemiesInMap.erase(enemyToRemove);
	}
}