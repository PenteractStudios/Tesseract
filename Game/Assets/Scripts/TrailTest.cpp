#include "TrailTest.h"

#include "Resources/ResourcePrefab.h"
#include "GameplaySystems.h"
#include "GameObject.h"

EXPOSE_MEMBERS(TrailTest) {
	// Add members here to expose them to the engine. Example:
	// MEMBER(MemberType::BOOL, exampleMember1),
	// MEMBER(MemberType::PREFAB_RESOURCE_UID, exampleMember2),
	// MEMBER(MemberType::GAME_OBJECT_UID, exampleMember3)

	MEMBER(MemberType::PREFAB_RESOURCE_UID, prefabId),
};

GENERATE_BODY_IMPL(TrailTest);

void TrailTest::Start() {
	gameObject = &GetOwner();
}

void TrailTest::Update() {
	if (Input::GetMouseButtonRepeat(0)) {
		ResourcePrefab* prefab = GameplaySystems::GetResource<ResourcePrefab>(prefabId);
		if (prefab != nullptr) {
			UID prefabId = prefab->BuildPrefab(gameObject);
			GameObject* go = GameplaySystems::GetGameObject(prefabId);
		}
	}
}