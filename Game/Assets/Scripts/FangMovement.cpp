#include "FangMovement.h"

#include "GameObject.h"
#include "GameplaySystems.h"

EXPOSE_MEMBERS(FangMovement) {
	// Add members here to expose them to the engine. Example:
	MEMBER(MemberType::GAME_OBJECT_UID, fangUID),
	MEMBER(MemberType::GAME_OBJECT_UID, cameraUID),
	MEMBER(MemberType::INT, speed),
	MEMBER(MemberType::FLOAT, cameraXPosition)
};

GENERATE_BODY_IMPL(FangMovement);

void FangMovement::Start() {
	fang = GameplaySystems::GetGameObject(fangUID);
	camera = GameplaySystems::GetGameObject(cameraUID);
	Debug::Log("Fang movement Start Log");
}

void FangMovement::Update() {
	if (!fang) return;
	if (!camera) return;

	ComponentTransform* cameraTransform = camera->GetComponent<ComponentTransform>();
	if (cameraTransform) {
		float modifier = 1.0f;
		if (cameraTransform->GetPosition().x < cameraXPosition) {
			if (Input::GetKeyCode(Input::KEYCODE::KEY_LSHIFT)) {
				modifier = 2.0f;
			}
			if (Input::GetKeyCode(Input::KEYCODE::KEY_W)) {
				ComponentTransform* transform = fang->GetComponent<ComponentTransform>();
				if (transform) {
					float3 newPosition = transform->GetPosition();
					newPosition.z -= speed * Time::GetDeltaTime() * modifier;
					transform->SetPosition(newPosition);
				}
			}
			if (Input::GetKeyCode(Input::KEYCODE::KEY_A)) {
				ComponentTransform* transform = fang->GetComponent<ComponentTransform>();
				if (transform) {
					float3 newPosition = transform->GetPosition();
					newPosition.x -= speed * Time::GetDeltaTime() * modifier;
					transform->SetPosition(newPosition);
				}
			}
			if (Input::GetKeyCode(Input::KEYCODE::KEY_S)) {
				ComponentTransform* transform = fang->GetComponent<ComponentTransform>();
				if (transform) {
					float3 newPosition = transform->GetPosition();
					newPosition.z += speed * Time::GetDeltaTime() * modifier;
					transform->SetPosition(newPosition);
				}
			}
			if (Input::GetKeyCode(Input::KEYCODE::KEY_D)) {
				ComponentTransform* transform = fang->GetComponent<ComponentTransform>();
				if (transform) {
					float3 newPosition = transform->GetPosition();
					newPosition.x += speed * Time::GetDeltaTime() * modifier;
					transform->SetPosition(newPosition);
				}
			}
		}
	}
}