#include "TriggerPowerUp.h"

#include "GameplaySystems.h"
#include "GameObject.h"

#include "DialogueManager.h"

EXPOSE_MEMBERS(TriggerPowerUp) {
    MEMBER(MemberType::GAME_OBJECT_UID, gameControllerUID)
};

GENERATE_BODY_IMPL(TriggerPowerUp);

void TriggerPowerUp::Start() {
    gameController = GameplaySystems::GetGameObject(gameControllerUID);
    if (gameController) dialogueManagerScript = GET_SCRIPT(gameController, DialogueManager);
}

void TriggerPowerUp::Update() {
	
}

void TriggerPowerUp::OnCollision(GameObject& /*collidedWith*/, float3 /*collisionNormal*/, float3 /*penetrationDistance*/, void* /*particle*/) {

    if (dialogueManagerScript) {
        dialogueManagerScript->ActivatePowerUpDialogue();
    }
    GetOwner().Disable();
}