#include "FangMovement.h"

#include "GameObject.h"
#include "GameplaySystems.h"
#include "Resources/ResourceMaterial.h"
#include "AIMovement.h"

EXPOSE_MEMBERS(FangMovement) {
	// Add members here to expose them to the engine. Example:
	MEMBER(MemberType::GAME_OBJECT_UID, fangUID),
		MEMBER(MemberType::GAME_OBJECT_UID, cameraUID),
		MEMBER(MemberType::GAME_OBJECT_UID, hitGOUID),
		MEMBER(MemberType::INT, speed),
		MEMBER(MemberType::INT, distanceRayCast),
		MEMBER(MemberType::FLOAT, cameraXPosition)
};

GENERATE_BODY_IMPL(FangMovement);

void FangMovement::Start() {
	character = Character();
	character.lifeSlots = 10;

	fang = GameplaySystems::GetGameObject(fangUID);
	camera = GameplaySystems::GetGameObject(cameraUID);
	Debug::Log("Fang movement Start Log");
}

void FangMovement::Update() {
	if (!fang) return;
	if (!camera) return;

	ComponentTransform* transform = fang->GetComponent<ComponentTransform>();

	if (Input::GetMouseButtonDown(0)) {
		if (transform) {
			float3 start = transform->GetPosition();
			float3 end = transform->GetGlobalRotation() * float3(0, 0, 1);
			end.Normalize();
			end *= distanceRayCast;
			int mask = static_cast<int>(MaskType::ENEMY);
			hitGo = Physics::Raycast(start, start + end, mask);
			if (hitGo) {
				hitGOUID = hitGo->GetID();
				ComponentMeshRenderer* meshRenderer = hitGo->GetComponent<ComponentMeshRenderer>();
				if (meshRenderer != nullptr) {
					ResourceMaterial* material = GameplaySystems::GetResource<ResourceMaterial>(meshRenderer->materialId);
					material->diffuseColor = float4(Colors::Red(), 1.0);
				}
				AIMovement* enemyScript = static_cast<AIMovement*>(hitGo->GetComponent<ComponentScript>()->GetScriptInstance());
				enemyScript->HitDetected();

			} else {
				hitGOUID = 0;
			}
		}
	}

	ComponentTransform* cameraTransform = camera->GetComponent<ComponentTransform>();
	if (cameraTransform) {
		float modifier = 1.0f;
		if (cameraTransform->GetPosition().x < cameraXPosition) {
			
			/*
			if (Input::GetKeyCode(Input::KEYCODE::KEY_LSHIFT)) {
				modifier = 2.0f;
			}
			if (Input::GetKeyCode(Input::KEYCODE::KEY_W)) {
				if (transform) {
					float3 newPosition = transform->GetPosition();
					newPosition.z -= speed * Time::GetDeltaTime() * modifier;
					transform->SetPosition(newPosition);
				}
			}
			if (Input::GetKeyCode(Input::KEYCODE::KEY_A)) {
				if (transform) {
					float3 newPosition = transform->GetPosition();
					newPosition.x -= speed * Time::GetDeltaTime() * modifier;
					transform->SetPosition(newPosition);
				}
			}
			if (Input::GetKeyCode(Input::KEYCODE::KEY_S)) {
				if (transform) {
					float3 newPosition = transform->GetPosition();
					newPosition.z += speed * Time::GetDeltaTime() * modifier;
					transform->SetPosition(newPosition);
				}
			}
			if (Input::GetKeyCode(Input::KEYCODE::KEY_D)) {
				if (transform) {
					float3 newPosition = transform->GetPosition();
					newPosition.x += speed * Time::GetDeltaTime() * modifier;
					transform->SetPosition(newPosition);
				}
			}
*/
			//float horizontalAxis = Input::GetControllerAxisValue(Input::SDL_CONTROLLER_AXIS_LEFTX, 0);
			//float verticalAxis = Input::GetControllerAxisValue(Input::SDL_CONTROLLER_AXIS_LEFTX, 1);

			//if (horizontalAxis != 0) {
			//	float3 newPosition = transform->GetPosition();
			//	newPosition.x += horizontalAxis * speed * Time::GetDeltaTime() * modifier;
			//	transform->SetPosition(newPosition);
			//}

			//if (verticalAxis != 0) {
			//	float3 newPosition = transform->GetPosition();
			//	newPosition.z += horizontalAxis * speed * Time::GetDeltaTime() * modifier;
			//	transform->SetPosition(newPosition);
			//}


		}
	}
}