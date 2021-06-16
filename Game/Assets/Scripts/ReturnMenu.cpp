#include "ReturnMenu.h"

#include "GameObject.h"
#include "GameplaySystems.h"

EXPOSE_MEMBERS(ReturnMenu) {
	MEMBER(MemberType::SCENE_RESOURCE_UID, sceneUID),
	MEMBER(MemberType::FLOAT, padding)
};

GENERATE_BODY_IMPL(ReturnMenu);

void ReturnMenu::Start() {

}

void ReturnMenu::Update() {

}

void ReturnMenu::OnButtonClick() {
	if(sceneUID != 0)SceneManager::ChangeScene(sceneUID);
	if (Time::GetDeltaTime() == 0.f) Time::ResumeGame();
}