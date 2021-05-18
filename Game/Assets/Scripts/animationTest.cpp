#include "animationTest.h"

#include "GameplaySystems.h"

#include "GameObject.h"
#include "Components/ComponentAnimation.h"

EXPOSE_MEMBERS(animationTest) {
	// Add members here to expose them to the engine. Example:
	// MEMBER(MemberType::BOOL, exampleMember1),
	// MEMBER(MemberType::PREFAB_RESOURCE_UID, exampleMember2),
	// MEMBER(MemberType::GAME_OBJECT_UID, exampleMember3)
};

GENERATE_BODY_IMPL(animationTest);

void animationTest::Start() {
	GameObject* goAnimated = GameplaySystems::GetGameObject("RootNode");
	if (goAnimated) {
		animation = goAnimated->GetComponent<ComponentAnimation>();
	}

}

void animationTest::Update() {

	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_1)) {
		animation->SendTrigger("s1Ts2");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_2)) {
		animation->SendTrigger("s2Ts3");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_3)) {
		animation->SendTrigger("s3Ts1");
	}
}