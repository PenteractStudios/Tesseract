#include "HUDTest.h"
#include "HUDController.h"

#include "GameplaySystems.h"

EXPOSE_MEMBERS(HUDTest) {
    // Add members here to expose them to the engine. Example:
    MEMBER(MemberType::FLOAT, cooldown1),
    MEMBER(MemberType::FLOAT, cooldown2),
    MEMBER(MemberType::FLOAT, cooldown3),
    MEMBER(MemberType::FLOAT, cooldown4),
    MEMBER(MemberType::FLOAT, cooldown5),
    MEMBER(MemberType::FLOAT, cooldown6),
    MEMBER(MemberType::FLOAT, cooldown7),
    MEMBER(MemberType::INT, fangHP),
    MEMBER(MemberType::INT, onimaruHP),
};

GENERATE_BODY_IMPL(HUDTest);

void HUDTest::Start() {
	
}

void HUDTest::Update() {
    if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_SPACE)) {
        HUDController::ChangePlayerHUD();
    }

    HUDController::UpdateCooldowns(cooldown1, cooldown2, cooldown3, cooldown4, cooldown5, cooldown6, cooldown7);
    HUDController::UpdateHP(fangHP, onimaruHP);
}