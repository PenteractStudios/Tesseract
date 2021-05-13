#pragma once

class Character
{
public:

	Character() {}

	void Hit(int damage);

public:

	bool isAlive = true;

	int lifeSlots = 5;

};

