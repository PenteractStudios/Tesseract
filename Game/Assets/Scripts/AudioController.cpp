#include "AudioController.h"

#include "GameObject.h"
#include "Components/ComponentAudioSource.h"

#include "GameplaySystems.h"

EXPOSE_MEMBERS(AudioController) {
    MEMBER(MemberType::GAME_OBJECT_UID, audioSourceUID)
};

GENERATE_BODY_IMPL(AudioController);

void AudioController::Start() {
    if (audioSourceUID != 0) {
        audioSourceGameObject = GameplaySystems::GetGameObject(audioSourceUID);
        if (audioSourceGameObject != nullptr) {
            ComponentAudioSource* audioComponent = audioSourceGameObject->GetComponent<ComponentAudioSource>();
            if (audioComponent != nullptr) {
                audioComponent->Play();
            }
        }
    }
}

void AudioController::Update() {
	
}