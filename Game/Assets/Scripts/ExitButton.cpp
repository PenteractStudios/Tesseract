#include "ExitButton.h"

#include "GameplaySystems.h"

EXPOSE_MEMBERS(ExitButton) {
	// Add members here to expose them to the engine. Example:
	// MEMBER(MemberType::BOOL, exampleMember1),
	// MEMBER(MemberType::PREFAB_RESOURCE_UID, exampleMember2),
	// MEMBER(MemberType::GAME_OBJECT_UID, exampleMember3)
};

GENERATE_BODY_IMPL(ExitButton);

void ExitButton::Start() {
}

void ExitButton::Update() {
}

void ExitButton::OnButtonClick() {
	SceneManager::ExitGame();
}