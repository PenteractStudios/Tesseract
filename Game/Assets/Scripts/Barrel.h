#pragma once

#include "Scripting/Script.h"

class ComponentSphereCollider;
class ComponentParticleSystem;
class ComponentAudioSource;

class CameraController;
class ResourcePrefab;
class Barrel : public Script
{
	GENERATE_BODY(Barrel);

public:

	void Start() override;
	void Update() override;

	void OnCollision(GameObject& collidedWith, float3 collisionNormal, float3 penetrationDistance, void* particle) override;

public:
	UID barrelUID = 0;
	UID cameraUID = 0;
	UID particlesUID = 0;

	ComponentSphereCollider* barrelCollider = nullptr;
	CameraController* cameraController = nullptr;
	ComponentParticleSystem* particles = nullptr;
	GameObject* barrel = nullptr;
	ComponentAudioSource* audio = nullptr ;
	bool isHit = false;
	bool destroy = false;
	float timeToDestroy = 1.0f;
};
