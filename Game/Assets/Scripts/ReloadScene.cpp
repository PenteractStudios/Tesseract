#include "ReloadScene.h"
#include "GameplaySystems.h"

EXPOSE_MEMBERS(ReloadScene) {
	MEMBER(MemberType::SCENE_RESOURCE_UID, sceneUID)
};

GENERATE_BODY_IMPL(ReloadScene);

void ReloadScene::Start() {

}

void ReloadScene::Update() {

}

void ReloadScene::OnButtonClick() {
	if(sceneUID != 0) SceneManager::ChangeScene(sceneUID);
}