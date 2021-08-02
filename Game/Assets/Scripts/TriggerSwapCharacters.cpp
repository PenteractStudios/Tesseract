#include "TriggerSwapCharacters.h"

#include "GameplaySystems.h"
#include "GameObject.h"

#include "DialogueManager.h"

EXPOSE_MEMBERS(TriggerSwapCharacters) {
    MEMBER(MemberType::GAME_OBJECT_UID, gameControllerUID)
};

GENERATE_BODY_IMPL(TriggerSwapCharacters);

void TriggerSwapCharacters::Start() {
    gameController = GameplaySystems::GetGameObject(gameControllerUID);
    if(gameController) dialogueManagerScript = GET_SCRIPT(gameController, DialogueManager);
}

void TriggerSwapCharacters::Update() {
	
}

void TriggerSwapCharacters::OnCollision(GameObject& /*collidedWith*/, float3 /*collisionNormal*/, float3 /*penetrationDistance*/, void* /*particle*/) {
    
    if (dialogueManagerScript) {
        dialogueManagerScript->SetActiveDialogue(&dialogueManagerScript->dialoguesArray[9]);
    }
    GetOwner().Disable();
}