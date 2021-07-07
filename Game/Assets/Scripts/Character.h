#pragma once

#include "GameplaySystems.h"

#define MAX_ACCELERATION 9999

class ComponentAudioSource;
class ComponentAgent;
class ComponentAnimation;
class ComponentTransform;

class Character {
public:
	Character() {}

	virtual void GetHit(float damage_);
	virtual void OnDeath();
	void Recover(int recoveryLife_);

	void SetTotalLifePoints(int totalLifePoints_);
	void SetDamageHit(float damageHit_);
public:
	bool isAlive = true;
	float lifePoints = 1;
	float damageHit = 1;
	float movementSpeed = 1.0f;
	ComponentAgent* agent = nullptr;
	ComponentAnimation* compAnimation = nullptr;
	State* currentState = nullptr;
	GameObject* characterGameObject = nullptr;

private:
	float totalLifePoints = 1;
};
