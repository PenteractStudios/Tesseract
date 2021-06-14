#include "gg.h"

#include "GameplaySystems.h"

EXPOSE_MEMBERS(gg) {
    // Add members here to expose them to the engine. Example:
    // MEMBER(MemberType::BOOL, exampleMember1),
    MEMBER(MemberType::SCENE_RESOURCE_UID, scene),
    // MEMBER(MemberType::GAME_OBJECT_UID, exampleMember3)
};

GENERATE_BODY_IMPL(gg);

void gg::Start() {

}

void gg::Update() {

}

void gg::OnCollision(GameObject& collidedWith) {
    Debug::Log("gg");
    SceneManager::ChangeScene(scene);
}