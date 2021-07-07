#pragma once

#include "Scripting/Script.h"
#include "StateMachineEnum.h"

#include "AIState.h"
#include "Enemy.h"

class GameObject;
class ComponentAnimation;
class ComponentTransform;
class ComponentAgent;
class ComponentAudioSource;
class ComponentMeshRenderer;
class ResourcePrefab;
class HUDController;
class PlayerController;
class AIMovement;
class WinLose;
class EnemySpawnPoint;

class AIMeleeGrunt : public Script {
	GENERATE_BODY(AIMeleeGrunt);

public:
	enum class AudioType {
		SPAWN,
		ATTACK,
		FOOTSTEP_RIGHT,
		FOOTSTEP_LEFT,
		HIT,
		DEATH,
		TOTAL
	};

	void Start() override;
	void Update() override;
	void OnAnimationFinished() override;
	void OnAnimationSecondaryFinished() override;
	void OnAnimationEvent(StateMachineEnum stateMachineEnum, const char* eventName) override;
	void OnCollision(GameObject& collidedWith, float3 collisionNormal, float3 penetrationDistance, void* particle = nullptr) override;

	void EnableBlastPushBack();
	void DisableBlastPushBack();
	bool IsBeingPushed() const;

public:

	UID playerUID = 0;
	UID canvasUID = 0;
	UID winConditionUID = 0;
	UID meleePunchUID = 0;

	// Hit feedback
	UID defaultMaterialPlaceHolderUID = 0;
	UID damageMaterialPlaceHolderUID = 0;
	UID defaultMaterialID = 0;
	UID damageMaterialID = 0;

	GameObject* player = nullptr;
	GameObject* spawn = nullptr;
	ComponentAgent* agent = nullptr;
	ResourcePrefab* meleePunch = nullptr;
	WinLose* winLoseScript = nullptr;

	Enemy gruntCharacter = Enemy(5, 8.0f, 1, 30, 40.f, 5.f, 5.f, 5.f, 5.f);
	bool killSent = false;

	float hurtFeedbackTimeDuration = 0.5f;

	float stunDuration = 3.f;

private:

	float attackRemaining = 0.0f;
	float3 velocity = float3(0, 0, 0);
	AIState state = AIState::START;
	bool hitTaken = false;
	ComponentAnimation* animation = nullptr;
	ComponentTransform* ownerTransform = nullptr;
	int damageRecieved = 0;

	float stunTimeRemaining = 0.f;

	bool EMPUpgraded = false;
	int deathType = 0;

	HUDController* hudControllerScript = nullptr;
	PlayerController* playerController = nullptr;
	AIMovement* movementScript = nullptr;
	EnemySpawnPoint* enemySpawnPointScript = nullptr;

	ComponentAudioSource* audios[static_cast<int>(AudioType::TOTAL)] = { nullptr };
	ComponentMeshRenderer* componentMeshRenderer = nullptr;

	float timeSinceLastHurt = 0.5f;
	GameObject* punch = nullptr;

	float currentPushBackDistance = 0.f;

private:
	void UpdatePushBackPosition();
};
