#pragma once

#include "Scripting/Script.h"
#include "Math/float3.h"

class ComponentAgent;
class GameObject;

class ZombunnyController : public Script
{
	GENERATE_BODY(ZombunnyController);

public:

	void Start() override;
	void Update() override;

public:
	float3 targetPosition = float3(0, 0, 0);
	float agentSpeed = 5.0f;

private:
	ComponentAgent* navAgent;

	float3 lastTargetPosition = float3(0, 0, 0);
};

