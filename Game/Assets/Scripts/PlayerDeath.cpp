#include "PlayerDeath.h"

#include "GameObject.h"
#include "GameplaySystems.h"

#include "PlayerController.h"

EXPOSE_MEMBERS(PlayerDeath) {
	MEMBER(MemberType::SCENE_RESOURCE_UID, sceneUID),
    MEMBER(MemberType::GAME_OBJECT_UID, playerUID)
};

GENERATE_BODY_IMPL(PlayerDeath);

void PlayerDeath::Start() {
	player = GameplaySystems::GetGameObject(playerUID);
	if(player) playerController = GET_SCRIPT(player, PlayerController);
}

void PlayerDeath::Update() {
	if (player) {
		if (playerController) {
			dead = playerController->IsDead();
		}
	}
	
}

void PlayerDeath::OnAnimationFinished() {
	if (dead) {
		if(sceneUID != 0) SceneManager::ChangeScene(sceneUID);
	}	
}

void PlayerDeath::OnAnimationSecondaryFinished()
{
	if (playerController) {
		if (playerController->shooting && playerController->fang->IsActive()) {
			ComponentAnimation* animation = playerController->fangAnimation;
			if (playerController->rightShot) {
				animation->SendTriggerSecondary(playerController->states[12] + animation->GetCurrentState()->name);
				playerController->rightShot = false;
			}
			else {
				animation->SendTriggerSecondary(playerController->states[11] + animation->GetCurrentState()->name);
				playerController->rightShot = true;
			}
		}
	}
}

void PlayerDeath::OnCollision(GameObject& collidedWith) {
	
	if (collidedWith.name == "RangerProjectile") {
		playerController->TakeDamage(true);
	}
	else if (collidedWith.name == "MeleePunch") {
		playerController->TakeDamage(false);
	}
}
