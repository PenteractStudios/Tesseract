#include "testCollision.h"
#include "GameplaySystems.h"

EXPOSE_MEMBERS(testCollision) {
    // Add members here to expose them to the engine. Example:
    // MEMBER(MemberType::BOOL, exampleMember1),
    // MEMBER(MemberType::PREFAB_RESOURCE_UID, exampleMember2),
    // MEMBER(MemberType::GAME_OBJECT_UID, exampleMember3)
};

GENERATE_BODY_IMPL(testCollision);

void testCollision::Start() {

}

void testCollision::Update() {

}

void testCollision::OnCollision() {
    Debug::Log("Collision Detected");
}