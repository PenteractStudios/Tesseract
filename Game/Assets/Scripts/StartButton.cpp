#include "StartButton.h"

#include "CheckpointManager.h"

#include "GameplaySystems.h"
#include "GameObject.h"

int checkpoint;

EXPOSE_MEMBERS(StartButton) {
	MEMBER(MemberType::SCENE_RESOURCE_UID, sceneUID),
	MEMBER(MemberType::INT, checkpointNum)
};


GENERATE_BODY_IMPL(StartButton);

void StartButton::Start() {
}

void StartButton::Update() {
}

void StartButton::OnButtonClick() {
	checkpoint = checkpointNum;
	if(sceneUID != 0) SceneManager::ChangeScene(sceneUID);
	if (Time::GetDeltaTime() == 0.f) Time::ResumeGame();
}