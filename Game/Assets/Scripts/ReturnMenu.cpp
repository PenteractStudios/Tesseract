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

}

void ReturnMenu::Update() {

}

void ReturnMenu::OnButtonClick() {
	SceneManager::ChangeScene("Assets/Scenes/StartScene34.scene");
}