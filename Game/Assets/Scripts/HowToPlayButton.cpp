#include "HowToPlayButton.h"

#include "GameplaySystems.h"

EXPOSE_MEMBERS(HowToPlayButton) {
	// Add members here to expose them to the engine. Example:
	// MEMBER(MemberType::BOOL, exampleMember1),
	// MEMBER(MemberType::PREFAB_RESOURCE_UID, exampleMember2),
	// MEMBER(MemberType::GAME_OBJECT_UID, exampleMember3)
};

GENERATE_BODY_IMPL(HowToPlayButton);

void HowToPlayButton::Start() {
}

void HowToPlayButton::Update() {
}

void HowToPlayButton::OnButtonClick() {
	SceneManager::ChangeScene("Assets/Scenes/HowToPlay.scene");
}