#pragma once

#include "Scripting/Script.h"

class PlayerController;
class SceneTransition;

class PlayerDeath : public Script
{
	GENERATE_BODY(PlayerDeath);

public:

	void Start() override;
	void Update() override;
	void OnAnimationFinished() override;
	void OnAnimationSecondaryFinished() override;
	void OnAnimationEvent(StateMachineEnum stateMachineEnum, const char* eventName) override;
	void OnCollision(GameObject& collidedWith, float3 collisionNormal, float3 penetrationDistance, void* particle = nullptr) override;
public:

	UID playerUID = 0;
	UID sceneUID = 0;
	UID transitionUID = 0;

	GameObject* player = nullptr;
	PlayerController* playerController = nullptr;

	bool dead = false;

private:
	GameObject* transitionGO = nullptr;
	SceneTransition* sceneTransition = nullptr;

};

