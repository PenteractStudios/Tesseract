#include "CreditsButton.h"

#include "GameplaySystems.h"

EXPOSE_MEMBERS(CreditsButton) {
	// Add members here to expose them to the engine. Example:
	// MEMBER(MemberType::BOOL, exampleMember1),
	// MEMBER(MemberType::PREFAB_RESOURCE_UID, exampleMember2),
	// MEMBER(MemberType::GAME_OBJECT_UID, exampleMember3)
};

GENERATE_BODY_IMPL(CreditsButton);

void CreditsButton::Start() {
}

void CreditsButton::Update() {
}

void CreditsButton::OnButtonClick() {
	SceneManager::ChangeScene("Assets/Scenes/CreditsScene.scene");
}