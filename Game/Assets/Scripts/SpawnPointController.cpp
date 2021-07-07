#include "SpawnPointController.h"

#include "GameObject.h"
#include "GameplaySystems.h"

EXPOSE_MEMBERS(SpawnPointController) {
	MEMBER(MemberType::PREFAB_RESOURCE_UID, meleeEnemyPrefabUID),
		MEMBER(MemberType::PREFAB_RESOURCE_UID, rangeEnemyPrefabUID),
		MEMBER(MemberType::GAME_OBJECT_UID, doorUID),
};

GENERATE_BODY_IMPL(SpawnPointController);

void SpawnPointController::Start() {
	gameObject = &GetOwner();

	/* Enemy prefabs to be used by the controller's children */
	meleeEnemyPrefab = GameplaySystems::GetResource<ResourcePrefab>(meleeEnemyPrefabUID);
	rangeEnemyPrefab = GameplaySystems::GetResource<ResourcePrefab>(rangeEnemyPrefabUID);

	doorObstacle = GameplaySystems::GetGameObject(doorUID);
}

void SpawnPointController::Update() {}

void SpawnPointController::OnCollision(GameObject& collidedWith, float3 collisionNormal, float3 penetrationDistance, void* particle) {
	if (!gameObject) return;
	for (GameObject* child : gameObject->GetChildren()) {
		if (!child->IsActive()) child->Enable();
	}
	ComponentBoxCollider* boxCollider = gameObject->GetComponent<ComponentBoxCollider>();
	if (boxCollider) boxCollider->Disable();
}

void SpawnPointController::OpenDoor() {
	if (doorObstacle && doorObstacle->IsActive()) {
		Debug::Log("Open door");
		doorObstacle->Disable();
	}
	else {
		if (!doorObstacle) {
			Debug::Log("Door obstacle null");
		}
	}
}