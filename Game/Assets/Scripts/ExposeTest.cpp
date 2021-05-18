#include "ExposeTest.h"
#include "GameplaySystems.h"
#include "Resources/ResourcePrefab.h"

EXPOSE_MEMBERS(ExposeTest) {
	MEMBER(MemberType::PREFAB_RESOURCE_UID, prefabId)
};

GENERATE_BODY_IMPL(ExposeTest);

void ExposeTest::Start() {

}

void ExposeTest::Update() {
	ResourcePrefab* prefab = GameplaySystems::GetResource<ResourcePrefab>(prefabId);
	if (prefab != nullptr) {
		prefab->BuildPrefab(GameplaySystems::GetGameObject("Scene"));
	}
	Debug::Log("Expose Test");
}