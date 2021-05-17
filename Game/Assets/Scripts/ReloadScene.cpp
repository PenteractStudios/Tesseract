#include "ReloadScene.h"
#include "GameplaySystems.h"

EXPOSE_MEMBERS(ReloadScene) {
	//TODO EXPOSE SCENE NAME?
};

GENERATE_BODY_IMPL(ReloadScene);

void ReloadScene::Start() {

}

void ReloadScene::Update() {

}

void ReloadScene::OnButtonClick() {
	SceneManager::ChangeScene("Assets/Scenes/Level1-34.scene");
}