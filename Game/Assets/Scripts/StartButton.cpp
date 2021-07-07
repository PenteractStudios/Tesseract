#include "StartButton.h"

#include "CheckpointManager.h"
#include "SceneTransition.h"

#include "GameplaySystems.h"
#include "GameObject.h"

int checkpoint;

EXPOSE_MEMBERS(StartButton) {
	MEMBER(MemberType::SCENE_RESOURCE_UID, sceneUID),
	MEMBER(MemberType::GAME_OBJECT_UID, transitionUID),
	MEMBER(MemberType::INT, checkpointNum),
};

GENERATE_BODY_IMPL(StartButton);

void StartButton::Start() {
    /* Audio */
	selectable = GetOwner().GetComponent<ComponentSelectable>();

	int i = 0;
	for (ComponentAudioSource& src : GetOwner().GetComponents<ComponentAudioSource>()) {
		if (i < static_cast<int>(UIAudio::TOTAL)) audios[i] = &src;
		++i;
	}

    if (transitionUID != 0) {
		transitionGO = GameplaySystems::GetGameObject(transitionUID);
		if (transitionGO) sceneTransition = GET_SCRIPT(transitionGO, SceneTransition);
	}
}

void StartButton::Update() {
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

void StartButton::OnButtonClick() {

    PlayAudio(UIAudio::CLICKED);

	checkpoint = checkpointNum;
    
    if (sceneTransition) {
		sceneTransition->StartTransition();
	} else {
		if (sceneUID != 0) SceneManager::ChangeScene(sceneUID);
		if (Time::GetDeltaTime() == 0.f) Time::ResumeGame();
	}
}

void StartButton::PlayAudio(UIAudio type) {
    if (audios[static_cast<int>(type)]) audios[static_cast<int>(type)]->Play();
}