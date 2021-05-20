#include "FangGun.h"

#include "Resources/ResourcePrefab.h"
#include "GameplaySystems.h"
#include "GameObject.h"

#include "Math/Quat.h"
#include "Geometry/Plane.h"
#include "Geometry/Frustum.h"
#include "Geometry/LineSegment.h"
#include "Math/float3x3.h"
#include "Math/float2.h"
#include <algorithm>

EXPOSE_MEMBERS(FangGun) {
	// Add members here to expose them to the engine. Example:
	// MEMBER(MemberType::BOOL, exampleMember1),
	// MEMBER(MemberType::PREFAB_RESOURCE_UID, exampleMember2),
	// MEMBER(MemberType::GAME_OBJECT_UID, exampleMember3)
	MEMBER(MemberType::PREFAB_RESOURCE_UID, prefabId)
};

GENERATE_BODY_IMPL(FangGun);

void FangGun::Start() {
}

void FangGun::Update() {
	ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();
	if (Input::GetMouseButtonDown(0)) {
		ResourcePrefab* prefab = GameplaySystems::GetResource<ResourcePrefab>(prefabId);
		if (prefab != nullptr) {
			Debug::Log("TraiddlShoot");
			//GameplaySystems::Instantiate(prefab, transform->GetGlobalPosition(), transform->GetGlobalRotation());
			//UID prefabId = prefab->BuildPrefab(scene);
			//GameObject* go = GameplaySystems::GetGameObject(prefabId);
		}
	}
}