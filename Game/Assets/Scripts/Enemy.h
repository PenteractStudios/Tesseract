#pragma once

#include "Character.h"

class Enemy : public Character {
public:
	// ------- Contructors ------- //
	Enemy() {};

	Enemy(int lifePoints_, float movementSpeed_, int damageHit_, int fallingSpeed_, float searchRadius_, float attackRange_, float timeToDie_, float pushBackDistance_, float pushBackSpeed_)
		: fallingSpeed(fallingSpeed_)
		, searchRadius(searchRadius_) 
		, attackRange(attackRange_)
		, timeToDie(timeToDie_)
		, pushBackDistance(pushBackDistance_)
		, pushBackSpeed(pushBackSpeed_) {
		lifePoints = lifePoints_;
		movementSpeed = movementSpeed_;
		damageHit = damageHit_;
		SetTotalLifePoints(lifePoints);
	}

	// ------- Core Functions ------ //
	void SetFallingSpeed(int fallingSpeed_);
	void SetSearchRadius(float searchRadius_);
	void SetAttackRange(float attackRange_);
	void SetTimeToDie(float timeToDie_);

public:
	int fallingSpeed = 5;
	float searchRadius = 10.f;
	float attackRange = 2.0f;
	float timeToDie = 5.f;
	float pushBackDistance = 5.f;
	float pushBackSpeed = 5.f;
	bool destroying = false;
	bool beingPushed = false;
};
