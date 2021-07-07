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

	/* Audio */
	selectable = GetOwner().GetComponent<ComponentSelectable>();

	int i = 0;
	for (ComponentAudioSource& src : GetOwner().GetComponents<ComponentAudioSource>()) {
		if (i < static_cast<int>(UIAudio::TOTAL)) audios[i] = &src;
		++i;
	}
}

void Resume::Update() {
	/* Audio */
	if (selectable) {
		ComponentEventSystem* eventSystem = UserInterface::GetCurrentEventSystem();
		if (eventSystem) {
			ComponentSelectable* hoveredComponent = eventSystem->GetCurrentlyHovered();
			if (hoveredComponent) {
				bool hovered = selectable->GetID() == hoveredComponent->GetID() ? true : false;
				if (hovered) {
					if (playHoveredAudio) {
						PlayAudio(UIAudio::HOVERED);
						playHoveredAudio = false;
					}
				}
				else {
					playHoveredAudio = true;
				}
			}
			else {
				playHoveredAudio = true;
			}
		}
	}
}

void Resume::OnButtonClick() {
	PlayAudio(UIAudio::CLICKED);
	if (!pauseCanvas) return;

	if (pauseCanvas->IsActive()) {
		pauseCanvas->Disable();
		if (hudCanvas) hudCanvas->Enable();
		Time::ResumeGame();
	}
}

void Resume::PlayAudio(UIAudio type) {
	if (audios[static_cast<int>(type)]) audios[static_cast<int>(type)]->Play();
}
