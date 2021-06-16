#pragma once

class Character
{
public:
	Character() {}

	void Hit(int damage_);
	void Recover(int recoveryLife_);

	void SetTotalLifePoints(int totalLifePoints_);
	void SetDamageHit(int damageHit_);

public:
	bool isAlive = true;

	int lifePoints = 1;
	float movementSpeed = 1.0f;
	int damageHit = 1;

private:
	int totalLifePoints = 1;
};
