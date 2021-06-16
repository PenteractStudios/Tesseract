#include "Resume.h"

#include "GameObject.h"
#include "GameplaySystems.h"

EXPOSE_MEMBERS(Resume) {
	MEMBER(MemberType::GAME_OBJECT_UID, hudUID),
	MEMBER(MemberType::GAME_OBJECT_UID, pauseUID),
};

GENERATE_BODY_IMPL(Resume);

void Resume::Start() {
	pauseCanvas = GameplaySystems::GetGameObject(pauseUID);
	hudCanvas = GameplaySystems::GetGameObject(hudUID);
}

void Resume::Update() {
}

void Resume::OnButtonClick() {
	if (!pauseCanvas) return;

	if (pauseCanvas->IsActive()) {
		pauseCanvas->Disable();
		if (hudCanvas) hudCanvas->Enable();
		Time::ResumeGame();
	}
}
