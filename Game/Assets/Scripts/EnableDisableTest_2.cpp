#include "EnableDisableTest_2.h"

#include "GameplaySystems.h"

EXPOSE_MEMBERS(EnableDisableTest_2) {
    // Add members here to expose them to the engine. Example:
    // MEMBER(MemberType::BOOL, exampleMember1),
    // MEMBER(MemberType::PREFAB_RESOURCE_UID, exampleMember2),
    // MEMBER(MemberType::GAME_OBJECT_UID, exampleMember3)
};

GENERATE_BODY_IMPL(EnableDisableTest_2);

void EnableDisableTest_2::Start() {
	
}

void EnableDisableTest_2::Update() {
	
}

void EnableDisableTest_2::OnEnable()
{
    Debug::Log("Enabled");
}

void EnableDisableTest_2::OnDisable()
{
    Debug::Log("Disabled");
}
