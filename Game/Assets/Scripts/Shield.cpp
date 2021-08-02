#include "Shield.h"

#include "GameObject.h"
#include "GameplaySystems.h"

EXPOSE_MEMBERS(Shield) {
	MEMBER(MemberType::INT, max_charges),
	MEMBER(MemberType::INT, cooldownCharge)
};

GENERATE_BODY_IMPL(Shield);

void Shield::Start() {}

void Shield::Update() {}

void Shield::InitShield() {
	num_charges = 0;
	isActive = true;
}

void Shield::FadeShield() {
	isActive = false;
	totalCooldown = cooldownCharge * num_charges;
}

void Shield::OnCollision(GameObject& collidedWith, float3 collisionNormal, float3 penetrationDistance, void* particle) {

	if ((collidedWith.name == "RangerProjectile" || collidedWith.name == "MeleePunch") && isActive) {
		GameplaySystems::DestroyGameObject(&collidedWith);
		num_charges++;
	}
}
