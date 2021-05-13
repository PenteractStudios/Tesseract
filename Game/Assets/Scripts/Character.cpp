#include "Character.h"

void Character::Hit(int damage) {
	lifeSlots -= damage;
	if (lifeSlots <= 0) {
		isAlive = false;
	}
}