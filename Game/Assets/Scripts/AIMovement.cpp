#include "AIMovement.h"

#include "GameObject.h"
#include "GameplaySystems.h"
#include "TesseractEvent.h"

#include "PlayerController.h"

EXPOSE_MEMBERS(AIMovement) {
    MEMBER(MemberType::GAME_OBJECT_UID, playerUID),
    MEMBER(MemberType::INT, maxSpeed),
    MEMBER(MemberType::INT, lifePoints),
    MEMBER(MemberType::FLOAT, searchRadius),
    MEMBER(MemberType::FLOAT, meleeRange),
    MEMBER(MemberType::FLOAT, timeToDie)

};

GENERATE_BODY_IMPL(AIMovement);

void AIMovement::Start() {
    player = GameplaySystems::GetGameObject(playerUID);
    animation = GetOwner().GetParent()->GetComponent<ComponentAnimation>();   
    parentTransform = GetOwner().GetParent()->GetComponent<ComponentTransform>();
}

void AIMovement::Update() {
    if (!GetOwner().IsActive()) return;

    if (hitTaken && lifePoints > 0) {
        if (state == AIState::IDLE || state == AIState::HURT) {
            animation->SendTrigger("IdleHurt");
        }
        else if (state == AIState::RUN) {
            animation->SendTrigger("RunHurt");
        }
        else if (state == AIState::ATTACK) {
            animation->SendTrigger("AttackHurt");
        }
        lifePoints -= damageRecieved;
        state = AIState::HURT;
        hitTaken = false;
    }

    switch (state)
    {
    case AIState::START:
        if (Camera::CheckObjectInsideFrustum(&GetOwner())) {
            Seek(float3(parentTransform->GetGlobalPosition().x, 0, parentTransform->GetGlobalPosition().z), fallingSpeed);
            if (parentTransform->GetGlobalPosition().y < 2.7 + 0e-5f) {
                animation->SendTrigger("StartSpawn");
                state = AIState::SPAWN;
            }
        }
        break;
    case AIState::SPAWN:                
        break;
    case AIState::IDLE:
        if (player) {
            if (CharacterInSight(player)) {
                animation->SendTrigger("IdleRun");
                state = AIState::RUN;
            }
        }
        break;
    case AIState::RUN:
        Seek(player->GetComponent<ComponentTransform>()->GetGlobalPosition(), maxSpeed);
        if (CharacterInMeleeRange(player)) {
            animation->SendTrigger("RunAttack");
            state = AIState::ATTACK;
        }
        break;
    case AIState::HURT:                
        break;
    case AIState::ATTACK:
        break;
    case AIState::DEATH:
        break;
    }

    if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_K)) {
        hitTaken = true;
    }

    if(dead){
        if (timeToDie > 0) {
            timeToDie -= Time::GetDeltaTime();
        }
        else {
            GameplaySystems::DestroyGameObject(GetOwner().GetParent());
        }
    }
    	
}

void AIMovement::ReceiveEvent(TesseractEvent& e)
{
    switch (e.type)
    {
    case TesseractEventType::ANIMATION_FINISHED:

        if (state == AIState::SPAWN) {
            animation->SendTrigger("SpawnIdle");
            state = AIState::IDLE;
        }

        else if(state == AIState::ATTACK)
        {
            PlayerController* playerController = GET_SCRIPT(player, PlayerController);
            playerController->HitDetected();
            animation->SendTrigger("AttackIdle");
            state = AIState::IDLE;
        }
        else if (state == AIState::HURT && lifePoints > 0) {
            animation->SendTrigger("HurtIdle");
            state = AIState::IDLE;
        }

        else if (state == AIState::HURT && lifePoints <= 0) {
            animation->SendTrigger("HurtDeath");
            state = AIState::DEATH;
        }
        else if (state == AIState::DEATH) {
            dead = true;
        }
        break;
    }
}

void AIMovement::HitDetected(int damage_) {
    damageRecieved = damage_;
    hitTaken = true;
}

bool AIMovement::CharacterInSight(const GameObject* character)
{
    ComponentTransform* target = character->GetComponent<ComponentTransform>();
    if (target) {
        float3 posTarget = target->GetGlobalPosition();
        return posTarget.Distance(parentTransform->GetGlobalPosition()) < searchRadius;
    }

    return false;
}

bool AIMovement::CharacterInMeleeRange(const GameObject* character)
{
    ComponentTransform* target = character->GetComponent<ComponentTransform>();
    if (target) {
        float3 posTarget = target->GetGlobalPosition();
        return posTarget.Distance(parentTransform->GetGlobalPosition()) < meleeRange;
    }

    return false;
}

void AIMovement::Seek(const float3& newPosition, int speed)
{

    float3 position = parentTransform->GetGlobalPosition();
    float3 direction = newPosition - position;

    velocity = direction.Normalized() * speed;

    position += velocity * Time::GetDeltaTime();

    parentTransform->SetGlobalPosition(position);

    if (state != AIState::START) {
        Quat newRotation = Quat::LookAt(float3(0, 0, 1), direction.Normalized(), float3(0, 1, 0), float3(0, 1, 0));
        parentTransform->SetGlobalRotation(newRotation);
    }
}
