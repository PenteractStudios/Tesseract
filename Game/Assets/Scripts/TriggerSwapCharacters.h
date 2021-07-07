#pragma once

#include "Scripting/Script.h"

class GameObject;
class DialogueManager;
class TriggerSwapCharacters : public Script
{
	GENERATE_BODY(TriggerSwapCharacters);

public:

	void Start() override;
	void Update() override;
	void OnCollision(GameObject& collidedWith, float3 collisionNormal, float3 penetrationDistance, void* particle = nullptr) override;

public:
	UID gameControllerUID = 0;
	GameObject* gameController = nullptr;
	DialogueManager* dialogueManagerScript = nullptr;
};

