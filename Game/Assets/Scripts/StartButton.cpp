#include "StartButton.h"

#include "GameController.h"

#include "GameplaySystems.h"
#include "GameObject.h"

int checkpoint;

EXPOSE_MEMBERS(StartButton) {
	// Add members here to expose them to the engine. Example:
	MEMBER(MemberType::INT, checkpointNum),
	// MEMBER(MemberType::PREFAB_RESOURCE_UID, exampleMember2),
	// MEMBER(MemberType::GAME_OBJECT_UID, exampleMember3)
};


GENERATE_BODY_IMPL(StartButton);

void StartButton::Start() {
}

void StartButton::Update() {
}

void StartButton::OnButtonClick() {
	checkpoint = checkpointNum;
	SceneManager::ChangeScene("Assets/Scenes/Level1-34.scene");
}