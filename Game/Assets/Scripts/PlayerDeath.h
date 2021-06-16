#pragma once

#include "Scripting/Script.h"

class PlayerController;

class PlayerDeath : public Script
{
	GENERATE_BODY(PlayerDeath);

public:

	void Start() override;
	void Update() override;
	void OnAnimationFinished() override;
	void OnAnimationSecondaryFinished() override;
	void OnCollision(GameObject& collidedWith) override;
public:

	UID playerUID = 0;
	UID sceneUID = 0;

	GameObject* player = nullptr;
	PlayerController* playerController = nullptr;

	bool dead = false;

};

