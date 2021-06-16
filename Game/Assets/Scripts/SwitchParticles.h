#pragma once

#include "Scripting/Script.h"

class GameObject;
class ComponentParticleSystem;

class SwitchParticles : public Script
{
	GENERATE_BODY(SwitchParticles);

public:
	UID playerUID = 0;
	UID explosionUID = 0;
	UID absorptionUID = 0;
	UID lightUID = 0;
	UID groundLightUID = 0;
	UID shockWaveUID = 0;

public:
	void Start() override;
	void Update() override;
	void Play();

private:
	GameObject* player = nullptr;
	ComponentParticleSystem* absorption = nullptr;
	ComponentParticleSystem* explosion = nullptr;
	ComponentParticleSystem* light = nullptr;
	ComponentParticleSystem* groundLight = nullptr;
	ComponentParticleSystem* shockWave = nullptr;
	bool playing = false;
};

