#include "AIMeleeGrunt.h"

#include "GameObject.h"
#include "GameplaySystems.h"

#include "PlayerController.h"
#include "EnemySpawnPoint.h"
#include "HUDController.h"
#include "AIMovement.h"
#include "WinLose.h"
#include "Player.h"
#include "Onimaru.h"

#include <math.h>
#include <string>

EXPOSE_MEMBERS(AIMeleeGrunt) {
	MEMBER(MemberType::GAME_OBJECT_UID, playerUID),
	MEMBER(MemberType::GAME_OBJECT_UID, canvasUID),
	MEMBER(MemberType::GAME_OBJECT_UID, winConditionUID),
	MEMBER(MemberType::PREFAB_RESOURCE_UID, meleePunchUID),
	MEMBER(MemberType::GAME_OBJECT_UID, damageMaterialPlaceHolderUID),
	MEMBER(MemberType::GAME_OBJECT_UID, defaultMaterialPlaceHolderUID),
	MEMBER(MemberType::INT, gruntCharacter.lifePoints),
	MEMBER(MemberType::FLOAT, gruntCharacter.movementSpeed),
	MEMBER(MemberType::INT, gruntCharacter.damageHit),
	MEMBER(MemberType::INT, gruntCharacter.fallingSpeed),
	MEMBER(MemberType::FLOAT, gruntCharacter.searchRadius),
	MEMBER(MemberType::FLOAT, gruntCharacter.attackRange),
	MEMBER(MemberType::FLOAT, gruntCharacter.timeToDie),
	MEMBER(MemberType::FLOAT, gruntCharacter.pushBackDistance),
	MEMBER(MemberType::FLOAT, gruntCharacter.pushBackSpeed),
	MEMBER(MemberType::FLOAT, hurtFeedbackTimeDuration),
	MEMBER(MemberType::FLOAT, stunDuration)
};

GENERATE_BODY_IMPL(AIMeleeGrunt);

void AIMeleeGrunt::Start() {

	player = GameplaySystems::GetGameObject(playerUID);

	if (player) {
		playerController = GET_SCRIPT(player, PlayerController);
	} else {
		player = GameplaySystems::GetGameObject("Player");
		if (player) {
			playerController = GET_SCRIPT(player, PlayerController);
		}
	}

	meleePunch = GameplaySystems::GetResource<ResourcePrefab>(meleePunchUID);

	GameObject* winLose = GameplaySystems::GetGameObject(winConditionUID);

	if (winLose) {
		winLoseScript = GET_SCRIPT(winLose, WinLose);
	}

	agent = GetOwner().GetComponent<ComponentAgent>();
	if (agent) {
		agent->SetMaxSpeed(gruntCharacter.movementSpeed);
		agent->SetMaxAcceleration(AIMovement::maxAcceleration);
		agent->SetAgentObstacleAvoidance(true);
		agent->RemoveAgentFromCrowd();
	}

	animation = GetOwner().GetComponent<ComponentAnimation>();
	ownerTransform = GetOwner().GetComponent<ComponentTransform>();

	GameObject* canvas = GameplaySystems::GetGameObject(canvasUID);
	if (canvas) {
		hudControllerScript = GET_SCRIPT(canvas, HUDController);
	}

	movementScript = GET_SCRIPT(&GetOwner(), AIMovement);

	int i = 0;
	for (ComponentAudioSource& src : GetOwner().GetComponents<ComponentAudioSource>()) {
		if (i < static_cast<int>(AudioType::TOTAL)) audios[i] = &src;
		++i;
	}

	enemySpawnPointScript = GET_SCRIPT(GetOwner().GetParent(), EnemySpawnPoint);

	// Hit feedback material retrieval
	GameObject* gameObject = nullptr;
	gameObject = GameplaySystems::GetGameObject(damageMaterialPlaceHolderUID);
	if (gameObject) {
		ComponentMeshRenderer* meshRenderer = gameObject->GetComponent<ComponentMeshRenderer>();
		if (meshRenderer) {
			damageMaterialID = meshRenderer->materialId;
		}
	}

	gameObject = GameplaySystems::GetGameObject(defaultMaterialPlaceHolderUID);
	if (gameObject) {
		ComponentMeshRenderer* meshRenderer = gameObject->GetComponent<ComponentMeshRenderer>();
		if (meshRenderer) {
			defaultMaterialID = meshRenderer->materialId;
		}
	}

	gameObject = &GetOwner();
	if (gameObject) {
		// Workaround get the first children - Create a Prefab overrides childs IDs
		gameObject = gameObject->GetChildren()[0];
		if (gameObject) {
			componentMeshRenderer = gameObject->GetComponent<ComponentMeshRenderer>();
		}
	}

}

void AIMeleeGrunt::Update() {
	if (!GetOwner().IsActive()) return;
	if (!player) return;
	if (!agent) return;
	if (!movementScript) return;
	if (!hudControllerScript) return;
	if (!playerController) return;
	if (!ownerTransform) return;
	if (!animation) return;
	if (!componentMeshRenderer) return;

	if (timeSinceLastHurt < hurtFeedbackTimeDuration) {
		timeSinceLastHurt += Time::GetDeltaTime();
		if (timeSinceLastHurt > hurtFeedbackTimeDuration) {
			componentMeshRenderer->materialId = defaultMaterialID;
		}
	}

	switch (state) {
	case AIState::START:
		movementScript->Seek(state, float3(ownerTransform->GetGlobalPosition().x, 0, ownerTransform->GetGlobalPosition().z), gruntCharacter.fallingSpeed, true);
		if (ownerTransform->GetGlobalPosition().y < 3.5f + 0e-5f) {
			ownerTransform->SetGlobalPosition(float3(ownerTransform->GetGlobalPosition().x, 3.0f, ownerTransform->GetGlobalPosition().z));
			animation->SendTrigger("StartSpawn");
			if (audios[static_cast<int>(AudioType::SPAWN)]) audios[static_cast<int>(AudioType::SPAWN)]->Play();
			state = AIState::SPAWN;
		}
		break;
	case AIState::SPAWN:
		break;
	case AIState::IDLE:
		if (!playerController->IsPlayerDead()) {
			if (movementScript->CharacterInSight(player, gruntCharacter.searchRadius)) {
				animation->SendTrigger("IdleRun");
				state = AIState::RUN;
			}
		}
		break;
	case AIState::RUN:
		movementScript->Seek(state, player->GetComponent<ComponentTransform>()->GetGlobalPosition(), gruntCharacter.movementSpeed, true);
		if (movementScript->CharacterInAttackRange(player, gruntCharacter.attackRange)) {
			animation->SendTriggerSecondary("RunAttack");
			agent->SetMaxSpeed(0);
			if (agent) agent->SetMaxSpeed(gruntCharacter.movementSpeed);
			if (audios[static_cast<int>(AudioType::ATTACK)]) audios[static_cast<int>(AudioType::ATTACK)]->Play();
			state = AIState::ATTACK;
		}
		break;
	case AIState::ATTACK:
		movementScript->Stop();
		break;
	case AIState::STUNNED:
		if (stunTimeRemaining <= 0.f) {
			stunTimeRemaining = 0.f;
			animation->SendTrigger("StunnedEndStun");			
		}
		else {
			stunTimeRemaining -= Time::GetDeltaTime();
		}
		break;
	case AIState::PUSHED:
		UpdatePushBackPosition();
		break;
	case AIState::DEATH:
		break;
	}

	if (gruntCharacter.destroying) {
		if (!killSent && winLoseScript != nullptr) {
			winLoseScript->IncrementDeadEnemies();
			if (enemySpawnPointScript) enemySpawnPointScript->UpdateRemainingEnemies();
			killSent = true;

			if (playerController) {
				if (playerController->playerOnimaru.characterGameObject->IsActive()) {
					playerController->playerOnimaru.IncreaseUltimateCounter();
				} else if (playerController->playerFang.characterGameObject->IsActive()) {
					playerController->playerFang.IncreaseUltimateCounter();
				}
			}

		}
		if (gruntCharacter.timeToDie > 0) {
			gruntCharacter.timeToDie -= Time::GetDeltaTime();
		} else {
			if (hudControllerScript) {
				hudControllerScript->UpdateScore(10);
			}
			if (playerController) playerController->RemoveEnemyFromMap(&GetOwner());
			GameplaySystems::DestroyGameObject(&GetOwner());
		}
	}

}

void AIMeleeGrunt::OnAnimationFinished() {
	if (state == AIState::SPAWN) {
		animation->SendTrigger("SpawnIdle");
		state = AIState::IDLE;
		agent->AddAgentToCrowd();
	}
	else if (state == AIState::STUNNED) {
		State* current = animation->GetCurrentState();
		if (current->name == "BeginStun") {
			animation->SendTrigger("BeginStunStunned");
		}
		else if (current->name == "EndStun") {
			animation->SendTrigger("EndStunIdle");
			agent->AddAgentToCrowd();
			state = AIState::IDLE;
		}
	}

	else if (state == AIState::DEATH) {
		gruntCharacter.destroying = true;
	}

}

void AIMeleeGrunt::OnAnimationSecondaryFinished() {
	if (state == AIState::ATTACK) {
		if (animation) {
			if (animation->GetCurrentState()) {
				animation->SendTriggerSecondary("Attack" + animation->GetCurrentState()->name);
			}
		}
		state = AIState::IDLE;
	}
}

void AIMeleeGrunt::OnAnimationEvent(StateMachineEnum stateMachineEnum, const char* eventName) {
	switch (stateMachineEnum)
	{
	case StateMachineEnum::PRINCIPAL:
		if (std::strcmp(eventName, "FootstepRight") != 0) {
			if (audios[static_cast<int>(AudioType::FOOTSTEP_RIGHT)]) audios[static_cast<int>(AudioType::FOOTSTEP_RIGHT)]->Play();
		}
		else if (std::strcmp(eventName, "FootstepLeft") != 0) {
			if (audios[static_cast<int>(AudioType::FOOTSTEP_LEFT)]) audios[static_cast<int>(AudioType::FOOTSTEP_LEFT)]->Play();
		}
		break;
	case StateMachineEnum::SECONDARY:
		if (std::strcmp(eventName, "EnablePunch") == 0) {
			Debug::Log("EnablePunch");
			float3 aux = ownerTransform->GetGlobalPosition() + ownerTransform->GetGlobalRotation().Transform(float3(0, 0, 1)) * 2.5f + float3(0, 2, 0);
			if (meleePunch) {
				/*punch = GameplaySystems::Instantiate(meleePunch, aux, ownerTransform->GetGlobalRotation());
				if (punch->IsActive())
				{
					Debug::Log("Active");
				}
				else {
					//punch->Enable();
					Debug::Log("Desactive");
				}*/
				std::string s = "Punch" + std::to_string(GetOwner().id);
				Debug::Log(s.c_str());
			}
		}
		if (std::strcmp(eventName, "DisablePunch") == 0) {
			/*GameplaySystems::DestroyGameObject(punch);
			punch = nullptr;*/
			std::string s = "DisablePunch" + std::to_string(GetOwner().id);
			Debug::Log(s.c_str());
		}
		break;
	default:
		break;
	}
}

void AIMeleeGrunt::OnCollision(GameObject& collidedWith, float3 collisionNormal, float3 penetrationDistance, void* particle) {
	if (state != AIState::START && state != AIState::SPAWN) {
		if (gruntCharacter.isAlive && playerController) {
			bool hitTaken = false;
			if (collidedWith.name == "FangBullet") {
				hitTaken = true;
				if(state == AIState::STUNNED && EMPUpgraded){
					gruntCharacter.GetHit(99);
				}
				else {
					gruntCharacter.GetHit(playerController->playerFang.damageHit + playerController->GetOverPowerMode());
				}
			} else if (collidedWith.name == "OnimaruBullet") {
				hitTaken = true;
				if (state == AIState::STUNNED && EMPUpgraded) {
					gruntCharacter.GetHit(99);
				}
				else {
					gruntCharacter.GetHit(playerController->playerOnimaru.damageHit + playerController->GetOverPowerMode());
				}
			}

			if (hitTaken) {
				if (audios[static_cast<int>(AudioType::HIT)]) audios[static_cast<int>(AudioType::HIT)]->Play();
				if (componentMeshRenderer) {
					if (damageMaterialID != 0) componentMeshRenderer->materialId = damageMaterialID;
				}

				timeSinceLastHurt = 0.0f;

			}

			if (collidedWith.name == "EMP") {
				if (state == AIState::ATTACK) {
					animation->SendTrigger("RunBeginStun");
					animation->SendTriggerSecondary("AttackBeginStun");
				}
				else if (state == AIState::IDLE) {
					animation->SendTrigger("IdleBeginStun");
				}
				else if (state == AIState::RUN) {
					animation->SendTrigger("RunBeginStun");
				}
				agent->RemoveAgentFromCrowd();
				stunTimeRemaining = stunDuration;
				state = AIState::STUNNED;
			}

		}

        if (!gruntCharacter.isAlive) {
            deathType = (rand() % 2 == 0) ? true : false;
            if (state == AIState::ATTACK) {
                if (deathType) {
                    animation->SendTrigger("RunDeath1");
                    animation->SendTriggerSecondary("AttackDeath1");
                }
                else {
                    animation->SendTrigger("RunDeath2");
                    animation->SendTriggerSecondary("AttackDeath2");
                }
                
            }
            else if (state == AIState::IDLE) {
                if (deathType) {
                    animation->SendTrigger("IdleDeath1");
                }
                else {
                    animation->SendTrigger("IdleDeath1");
                }
            }
            else if (state == AIState::RUN) {
                if (deathType) {
                    animation->SendTrigger("RunDeath1");
                }
                else {
                    animation->SendTrigger("RunDeath2");
                }
            }
			else if (state == AIState::STUNNED) {
				if (deathType) {
					animation->SendTrigger("StunnedDeath1");
				}
				else {
					animation->SendTrigger("StunnedDeath2");
				}
			}
			else if (state == AIState::PUSHED) {
				if (deathType) {
					animation->SendTrigger("HurtDeath1");
				}
				else {
					animation->SendTrigger("HurtDeath2");
				}
			}

			if (audios[static_cast<int>(AudioType::DEATH)]) audios[static_cast<int>(AudioType::DEATH)]->Play();
			ComponentCapsuleCollider* collider = GetOwner().GetComponent<ComponentCapsuleCollider>();
			if (collider) collider->Disable();

			agent->RemoveAgentFromCrowd();
			if (gruntCharacter.beingPushed) gruntCharacter.beingPushed = false;
			state = AIState::DEATH;

		}
	}
}

void AIMeleeGrunt::EnableBlastPushBack() {
	if (state != AIState::START && state != AIState::SPAWN && state != AIState::DEATH) {
		gruntCharacter.beingPushed = true;
		state = AIState::PUSHED;
		if (animation->GetCurrentState()) animation->SendTrigger(animation->GetCurrentState()->name + "Hurt");
	}
}

void AIMeleeGrunt::DisableBlastPushBack() {
	if (state != AIState::START && state != AIState::SPAWN && state != AIState::DEATH) {
		gruntCharacter.beingPushed = false;
		if (animation->GetCurrentState()) animation->SendTrigger(animation->GetCurrentState()->name + "Idle");
		state = AIState::IDLE;
	}
}

bool AIMeleeGrunt::IsBeingPushed() const {
	return gruntCharacter.beingPushed;
}

void AIMeleeGrunt::UpdatePushBackPosition() {
	float3 playerPos = player->GetComponent<ComponentTransform>()->GetGlobalPosition();
	float3 enemyPos = GetOwner().GetComponent<ComponentTransform>()->GetGlobalPosition();
	float3 initialPos = enemyPos;

	float3 direction = (enemyPos - playerPos).Normalized();

	if (agent) {
		enemyPos += direction * gruntCharacter.pushBackSpeed * Time::GetDeltaTime();
		agent->SetMoveTarget(enemyPos, false);
		float distance = enemyPos.Distance(initialPos);
		currentPushBackDistance += distance;

		if (currentPushBackDistance >= gruntCharacter.pushBackDistance) {
			DisableBlastPushBack();
			currentPushBackDistance = 0.f;
		}
	}
}