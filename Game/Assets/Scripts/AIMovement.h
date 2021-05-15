#pragma once

#include "Scripting/Script.h"

struct TesseractEvent;
class ComponentAnimation;
class ComponentTransform;

class HUDController;

enum class AIState {
	START,
	SPAWN,
	IDLE,
	RUN,
	HURT,
	ATTACK,
	DEATH
};

class AIMovement : public Script
{
	GENERATE_BODY(AIMovement);

public:

	void Start() override;
	void Update() override;
	void ReceiveEvent(TesseractEvent& e) override;
	void HitDetected(int damage_ = 1);

private:
	bool CharacterInSight(const GameObject* character);
	bool CharacterInMeleeRange(const GameObject* character);
	void Seek(const float3& newPosition, int speed);
	

public:

	UID playerUID = 0;
	UID canvasUID = 0;

	GameObject* player = nullptr;
	
	int maxSpeed = 8;
	int fallingSpeed = 30;
	float searchRadius = 40.f;
	float meleeRange = 5.f;
	int lifePoints = 5;
	float timeToDie = 5.f;
	bool dead = false;


private:

	float3 velocity = float3(0, 0, 0);	
	AIState state = AIState::START;
	bool hitTaken = false;
	ComponentAnimation* animation = nullptr;
	ComponentTransform* parentTransform = nullptr;
	int damageRecieved = 0;

	HUDController* hudControllerScript = nullptr;

};

