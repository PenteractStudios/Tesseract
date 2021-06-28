#pragma once

#include "Scripting/Script.h"

class killparticle : public Script
{
	GENERATE_BODY(killparticle);

public:

	void Start() override;
	void Update() override;
	void OnCollision(GameObject& collidedWith, float3 collisionNormal, float3 penetrationDistance, void* particle = nullptr) override;

};

