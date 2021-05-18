#include "TreeFloating.h"

#include "GameObject.h"
#include "GameplaySystems.h"

#include <cmath>

EXPOSE_MEMBERS(TreeFloating) {
	// Add members here to expose them to the engine. Example:
	MEMBER(MemberType::GAME_OBJECT_UID, treesUID)
};

GENERATE_BODY_IMPL(TreeFloating);

void TreeFloating::Start() {
	offset = 0.f;
	trees = GameplaySystems::GetGameObject(treesUID);
	if (trees) transform = trees->GetComponent<ComponentTransform>();
	if (transform) initialPosition = transform->GetPosition();
}

void TreeFloating::Update() {
	
	if (trees) transform = trees->GetComponent<ComponentTransform>();
	if (!transform) return;
	offset = offset + Time::GetDeltaTime();
	transform->SetPosition(initialPosition + float3(0.f, Sin(offset), 0.f));
}