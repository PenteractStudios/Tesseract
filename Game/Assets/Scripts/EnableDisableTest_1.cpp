#include "EnableDisableTest_1.h"

#include "GameplaySystems.h"

EXPOSE_MEMBERS(EnableDisableTest_1) {
    // Add members here to expose them to the engine. Example:
    // MEMBER(MemberType::BOOL, exampleMember1),
    MEMBER(MemberType::FLOAT, timer),
    MEMBER(MemberType::GAME_OBJECT_UID, gameObjectUID)
};

GENERATE_BODY_IMPL(EnableDisableTest_1);

void EnableDisableTest_1::Start() {
	
    gameObject = GameplaySystems::GetGameObject(gameObjectUID);

}

void EnableDisableTest_1::Update() {
	
    if (!gameObject) return;

    if (timer < 0) {
        canDisable = true;
        canEnable = false;
        dir = 1;
    }
    else if (timer > 10) {
        canDisable = false;
        canEnable = true;
        dir = -1;
    }

    if (canDisable) {
        gameObject->Disable();
        canDisable = false;
    }
    else if (canEnable) {
        gameObject->Enable();
        canEnable = false;
    }

    timer += (dir * Time::GetDeltaTime());


}