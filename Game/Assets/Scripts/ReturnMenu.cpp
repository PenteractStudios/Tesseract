#include "ReturnMenu.h"

#include "GameObject.h"
#include "GameplaySystems.h"

EXPOSE_MEMBERS(ReturnMenu) {
	// Add members here to expose them to the engine. Example:
	// MEMBER(MemberType::BOOL, exampleMember1),
	MEMBER(MemberType::FLOAT, padding),
};

GENERATE_BODY_IMPL(ReturnMenu);

void ReturnMenu::Start() {
	GameObject exitButton = GetOwner();
	ComponentTransform2D* transform2D = exitButton.GetComponent<ComponentTransform2D>();
	if (transform2D) {
		float2 buttonSize = transform2D->GetSize();
		float2 newSize = float2(Screen::GetScreenWitdh(), Screen::GetScreenHeight());
		transform2D->SetPosition(float3(newSize.x / 2 - buttonSize.x - padding, newSize.y / 2 - buttonSize.y - padding, transform2D->GetPosition().z));
	}
}

void ReturnMenu::Update() {
	GameObject exitButton = GetOwner();
	ComponentTransform2D* transform2D = exitButton.GetComponent<ComponentTransform2D>();
	if (transform2D) {
		float2 buttonSize = transform2D->GetSize();
		float2 newSize = float2(Screen::GetScreenWitdh(), Screen::GetScreenHeight());
		transform2D->SetPosition(float3(newSize.x / 2 - buttonSize.x - padding, newSize.y / 2 - buttonSize.y - padding, transform2D->GetPosition().z));
	}
}

void ReturnMenu::OnButtonClick() {
	SceneManager::ChangeScene("Assets/Scenes/StartScene.scene");
}