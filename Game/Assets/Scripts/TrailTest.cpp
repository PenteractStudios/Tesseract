#include "TrailTest.h"

#include "Resources/ResourcePrefab.h"
#include "GameplaySystems.h"
#include "GameObject.h"
#include "TrailScript.h"
#include "Math/Quat.h"
#include "Geometry/Plane.h"
#include "Geometry/Frustum.h"
#include "Geometry/LineSegment.h"
#include "Math/float3x3.h"
#include "Math/float2.h"
#include <algorithm>
#include <string>

#define PI 3.14159

EXPOSE_MEMBERS(TrailTest) {
	// Add members here to expose them to the engine. Example:
	// MEMBER(MemberType::BOOL, exampleMember1),
	// MEMBER(MemberType::PREFAB_RESOURCE_UID, exampleMember2),
	// MEMBER(MemberType::GAME_OBJECT_UID, exampleMember3)
	MEMBER(MemberType::FLOAT, cameraOffsetZ),
		MEMBER(MemberType::FLOAT, cameraOffsetY),
		MEMBER(MemberType::GAME_OBJECT_UID, sceneId),
		MEMBER(MemberType::GAME_OBJECT_UID, cameraUID),
		MEMBER(MemberType::GAME_OBJECT_UID, gunId),
		MEMBER(MemberType::PREFAB_RESOURCE_UID, prefabId),
		MEMBER(MemberType::INT, speed)
};

GENERATE_BODY_IMPL(TrailTest);

void TrailTest::Start() {
	gameObject = &GetOwner();
	scene = GameplaySystems::GetGameObject(sceneId);
	camera = GameplaySystems::GetGameObject(cameraUID);
	fangGun = GameplaySystems::GetGameObject(gunId);
	if (gameObject) {
		transform = gameObject->GetComponent<ComponentTransform>();
	}
	if (fangGun) {
		FangGuntransform = fangGun->GetComponent<ComponentTransform>();
	}
	if (camera) {
		compCamera = camera->GetComponent<ComponentCamera>();
		if (compCamera) GameplaySystems::SetRenderCamera(compCamera);
	}
}

void TrailTest::LookAtMouse() {
	float2 mousePos = Input::GetMousePositionNormalized();
	LineSegment ray = compCamera->frustum.UnProjectLineSegment(mousePos.x, mousePos.y);
	float3 cameraGlobalPos = camera->GetComponent<ComponentTransform>()->GetGlobalPosition();
	Plane p = Plane(transform->GetGlobalPosition(), float3(0, 1, 0));
	facePointDir = float3(0, 0, 0);
	cameraGlobalPos.z = 0;
	facePointDir = p.ClosestPoint(ray) - (transform->GetGlobalPosition());
	Quat quat = transform->GetRotation();
	float angle = Atan2(facePointDir.x, facePointDir.z);
	Quat rotation = quat.RotateAxisAngle(float3(0, 1, 0), angle);
	transform->SetRotation(rotation);
}

void TrailTest::Update() {
	LookAtMouse();

	ComponentTransform* cameraTransform = camera->GetComponent<ComponentTransform>();
	cameraTransform->SetPosition(float3(transform->GetGlobalPosition().x,
		transform->GetGlobalPosition().y + cameraOffsetY,
		(transform->GetGlobalPosition().z + cameraOffsetZ)));

	ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();
	FangGuntransform = fangGun->GetComponent<ComponentTransform>();
	if (Input::GetMouseButtonDown(0)) {
		ResourcePrefab* prefab = GameplaySystems::GetResource<ResourcePrefab>(prefabId);
		if (prefab != nullptr) {
			fangGun->GetComponent<ComponentParticleSystem>()->Play();
			GameplaySystems::Instantiate(prefab, FangGuntransform->GetGlobalPosition(), transform->GetGlobalRotation());
			float3 frontTrail = transform->GetGlobalRotation() * float3(0.0f, 0.0f, 1.0f);
			GameObject* secondTrail = GameplaySystems::Instantiate(prefab, FangGuntransform->GetGlobalPosition(), Quat::RotateAxisAngle(frontTrail, (pi / 2)).Mul(transform->GetGlobalRotation()));
			TrailScript* seconTrailScript = GET_SCRIPT(secondTrail, TrailScript);
		}
	}
	if (Input::GetKeyCode(Input::KEYCODE::KEY_W)) {
		if (transform) {
			float3 newPosition = transform->GetPosition();
			newPosition.z -= speed * Time::GetDeltaTime();
			transform->SetPosition(newPosition);
		}
	}
	if (Input::GetKeyCode(Input::KEYCODE::KEY_A)) {
		if (transform) {
			float3 newPosition = transform->GetPosition();
			newPosition.x -= speed * Time::GetDeltaTime();
			transform->SetPosition(newPosition);
		}
	}
	if (Input::GetKeyCode(Input::KEYCODE::KEY_S)) {
		if (transform) {
			float3 newPosition = transform->GetPosition();
			newPosition.z += speed * Time::GetDeltaTime();
			transform->SetPosition(newPosition);
		}
	}
	if (Input::GetKeyCode(Input::KEYCODE::KEY_D)) {
		if (transform) {
			float3 newPosition = transform->GetPosition();
			newPosition.x += speed * Time::GetDeltaTime();
			transform->SetPosition(newPosition);
		}
	}
}