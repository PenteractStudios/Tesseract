#include "MeleePunch.h"
#include "GameplaySystems.h"
#include "GameObject.h"

EXPOSE_MEMBERS(MeleePunch) {
	MEMBER(MemberType::FLOAT, life)
};

GENERATE_BODY_IMPL(MeleePunch);

void MeleePunch::Start() {
	
}

void MeleePunch::Update() {
	if (life >= 0) {
		life -= Time::GetDeltaTime();
	}
	else {
		GameplaySystems::DestroyGameObject(&GetOwner());
	}
}

void MeleePunch::OnCollision(GameObject& collidedWith) {
	if (collidedWith.name == "Onimaru" || collidedWith.name == "Fang") {
		GameplaySystems::DestroyGameObject(&GetOwner());
	}
}