#include "TrailScript.h"

#include "GameObject.h"
#include "GameplaySystems.h"
EXPOSE_MEMBERS(TrailScript) {
	// Add members here to expose them to the engine. Example:
	// MEMBER(MemberType::BOOL, exampleMember1),
	// MEMBER(MemberType::PREFAB_RESOURCE_UID, exampleMember2),
	// MEMBER(MemberType::GAME_OBJECT_UID, exampleMember3)
	MEMBER(MemberType::INT, speed),
};

GENERATE_BODY_IMPL(TrailScript);

void TrailScript::Start() {
}

void TrailScript::Update() {
	if (life >= 0) {
		life -= Time::GetDeltaTime();
		ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();
		float3 aux = transform->GetGlobalRotation() * float3(0.0f, 0.0f, 1.0f);

		if (itsVertical) {
			if (!firstTime) {
				transform->SetGlobalRotation(Quat::RotateAxisAngle(aux, (pi / 2)).Mul(transform->GetGlobalRotation()));
				firstTime = true;
			}
		}

		aux *= speed * Time::GetDeltaTime();
		float3 newPosition = transform->GetGlobalPosition();
		newPosition += aux;
		transform->SetGlobalPosition(newPosition);
	}
	else {
		GameplaySystems::DestroyGameObject(&GetOwner());
	}
}