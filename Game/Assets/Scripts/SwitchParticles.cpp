#include "SwitchParticles.h"

#include "GameplaySystems.h"

EXPOSE_MEMBERS(SwitchParticles) {
	MEMBER(MemberType::GAME_OBJECT_UID, playerUID),
		MEMBER(MemberType::GAME_OBJECT_UID, absorptionUID),
		MEMBER(MemberType::GAME_OBJECT_UID, explosionUID),
		MEMBER(MemberType::GAME_OBJECT_UID, lightUID),
		MEMBER(MemberType::GAME_OBJECT_UID, groundLightUID),
		MEMBER(MemberType::GAME_OBJECT_UID, shockWaveUID)
};

GENERATE_BODY_IMPL(SwitchParticles);

void SwitchParticles::Start() {

	player = GameplaySystems::GetGameObject(playerUID);

	GameObject* effect = GameplaySystems::GetGameObject(absorptionUID);

	if (effect) {
		absorption = effect->GetComponent<ComponentParticleSystem>();
	}

	effect = GameplaySystems::GetGameObject(explosionUID);

	if (effect) {
		explosion = effect->GetComponent<ComponentParticleSystem>();
	}

	effect = GameplaySystems::GetGameObject(lightUID);

	if (effect) {
		light = effect->GetComponent<ComponentParticleSystem>();
	}

	effect = GameplaySystems::GetGameObject(groundLightUID);

	if (effect) {
		groundLight = effect->GetComponent<ComponentParticleSystem>();
	}

	effect = GameplaySystems::GetGameObject(shockWaveUID);

	if (effect) {
		shockWave = effect->GetComponent<ComponentParticleSystem>();
	}
	
}

void SwitchParticles::Update() {
	if (!player || !absorption || !explosion || !light || !groundLight || !shockWave) return;

	if (playing) {
		absorption->Play();
		explosion->Play();
		light->Play();
		groundLight->Play();
		shockWave->Play();
		playing = false;
	}
	
}

void SwitchParticles::Play() {
	playing = true;
}
