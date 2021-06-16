#include "AIMeleeGrunt.h"

#include "GameObject.h"
#include "GameplaySystems.h"

#include "PlayerController.h"
#include "EnemySpawnPoint.h"
#include "HUDController.h"
#include "AIMovement.h"
#include "WinLose.h"
#include "Player.h"

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
    MEMBER(MemberType::FLOAT, hurtFeedbackTimeDuration)
};

GENERATE_BODY_IMPL(AIMeleeGrunt);

void AIMeleeGrunt::Start() {

	player = GameplaySystems::GetGameObject(playerUID);
	
    if (player) {
		playerController = GET_SCRIPT(player, PlayerController);
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
    gameObject  = GameplaySystems::GetGameObject(damageMaterialPlaceHolderUID);
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

    switch (state)
    {
    case AIState::START:
        if (Camera::CheckObjectInsideFrustum(GetOwner().GetChildren()[0])) {
            movementScript->Seek(state, float3(ownerTransform->GetGlobalPosition().x, 0, ownerTransform->GetGlobalPosition().z), gruntCharacter.fallingSpeed, true);
            if (ownerTransform->GetGlobalPosition().y < 2.7 + 0e-5f) {
                animation->SendTrigger("StartSpawn");
                if (audios[static_cast<int>(AudioType::SPAWN)]) audios[static_cast<int>(AudioType::SPAWN)]->Play();
                state = AIState::SPAWN;
            }
        }
        break;
    case AIState::SPAWN:
        break;
    case AIState::IDLE:
        if (!playerController->IsDead()) {
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
            float3 aux = ownerTransform->GetGlobalPosition() + ownerTransform->GetGlobalRotation().Transform(float3(0, 0, 1)) * 2 + float3(0, 2, 0);
            if (meleePunch) GameplaySystems::Instantiate(meleePunch, aux, ownerTransform->GetGlobalRotation());
            if (audios[static_cast<int>(AudioType::ATTACK)]) audios[static_cast<int>(AudioType::ATTACK)]->Play();
            state = AIState::ATTACK;        
        }
        break;
    case AIState::ATTACK:
        break;
    case AIState::DEATH:
        break;
    }

    if (gruntCharacter.destroying) {
        if (!killSent && winLoseScript != nullptr) {
            winLoseScript->IncrementDeadEnemies();
            if(enemySpawnPointScript) enemySpawnPointScript->UpdateRemainingEnemies();
            killSent = true;
        }
        if (gruntCharacter.timeToDie > 0) {
            gruntCharacter.timeToDie -= Time::GetDeltaTime();
        }
        else {
            if (hudControllerScript) {
                hudControllerScript->UpdateScore(10);
            }
            GameplaySystems::DestroyGameObject(&GetOwner());
        }
    }

}

void AIMeleeGrunt::OnAnimationFinished()
{
    if (state == AIState::SPAWN) {
        animation->SendTrigger("SpawnIdle");
        state = AIState::IDLE;
        agent->AddAgentToCrowd();
    }    

    else if (state == AIState::DEATH) {
        gruntCharacter.destroying = true;
    }

}

void AIMeleeGrunt::OnAnimationSecondaryFinished()
{
    if (state == AIState::ATTACK)
    {
        if (animation) animation->SendTriggerSecondary("Attack" + animation->GetCurrentState()->name);
        state = AIState::IDLE;
    }
}

void AIMeleeGrunt::OnCollision(GameObject& collidedWith)
{
    if (state != AIState::START && state != AIState::SPAWN) {
        if (gruntCharacter.isAlive && playerController) {
            bool hitTaken = false;
            if (collidedWith.name == "FangBullet") {
                hitTaken = true;
                gruntCharacter.Hit(playerController->fangCharacter.damageHit + playerController->GetOverPowerMode());
            }
            else if (collidedWith.name == "OnimaruBullet") {
                hitTaken = true;
                gruntCharacter.Hit(playerController->onimaruCharacter.damageHit + playerController->GetOverPowerMode());
            }

            if (hitTaken) {
                if (audios[static_cast<int>(AudioType::HIT)]) audios[static_cast<int>(AudioType::HIT)]->Play();
                if (componentMeshRenderer) {
                    if(damageMaterialID != 0) componentMeshRenderer->materialId = damageMaterialID;
                }

                timeSinceLastHurt = 0.0f;

            }

        }

        if (!gruntCharacter.isAlive) {
            if (state == AIState::ATTACK) {
                animation->SendTrigger("RunDeath");
                animation->SendTriggerSecondary("AttackDeath");
            }
            else if (state == AIState::IDLE) {
                animation->SendTrigger("IdleDeath");
            }
            else if (state == AIState::RUN) {
                animation->SendTrigger("RunDeath");
            }

            if (audios[static_cast<int>(AudioType::DEATH)]) audios[static_cast<int>(AudioType::DEATH)]->Play();
            ComponentCapsuleCollider* collider = GetOwner().GetComponent<ComponentCapsuleCollider>();
            if (collider) collider->Disable();

            agent->RemoveAgentFromCrowd();
            state = AIState::DEATH;
        }
    }
}
