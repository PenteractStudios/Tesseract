#include "Resume.h"

#include "GameObject.h"
#include "GameplaySystems.h"

EXPOSE_MEMBERS(Resume) {
	MEMBER(MemberType::GAME_OBJECT_UID, pauseUID),
};

GENERATE_BODY_IMPL(Resume);

void Resume::Start() {
	pauseCanvas = GameplaySystems::GetGameObject(pauseUID);
}

void Resume::Update() {
}

void Resume::OnButtonClick() {
	if (!pauseCanvas) return;

	if (pauseCanvas->IsActive()) {
		pauseCanvas->Disable();
		Time::ResumeGame();
	}
}
