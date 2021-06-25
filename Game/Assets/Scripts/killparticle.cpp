#include "killparticle.h"
#include "GameplaySystems.h"

EXPOSE_MEMBERS(killparticle) {
    // Add members here to expose them to the engine. Example:
    // MEMBER(MemberType::BOOL, exampleMember1),
    // MEMBER(MemberType::PREFAB_RESOURCE_UID, exampleMember2),
    // MEMBER(MemberType::GAME_OBJECT_UID, exampleMember3)
};

GENERATE_BODY_IMPL(killparticle);

void killparticle::Start() {

}

void killparticle::Update() {

}

void killparticle::OnCollision(GameObject& collidedWith, float3 collisionNormal, float3 penetrationDistance, void* particle)
{
    if (!particle) return;
    Debug::Log("Agh");
    ComponentParticleSystem::Particle* p = (ComponentParticleSystem::Particle*)particle;
    ComponentParticleSystem* pSystem = collidedWith.GetComponent<ComponentParticleSystem>();
    if (pSystem) pSystem->KillParticle(p);
}
