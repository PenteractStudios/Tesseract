#pragma once

#include "Scripting/Script.h"

class Shield : public Script
{
	GENERATE_BODY(Shield);

public:

	void Start() override;
	void Update() override;
	void OnCollision(GameObject& collidedWith, float3 collisionNormal, float3 penetrationDistance, void* particle = nullptr) override;
	void InitShield();
	void FadeShield();

	bool GetIsActive() { return isActive; }
	void SetIsActive(bool a) { isActive = a; }
	int GetCoolDown() { return totalCooldown; }
	int GetNumCharges() { return num_charges; }

public:

	int max_charges = 0;
	int cooldownCharge = 0;

private:
	int num_charges = 0;
	int totalCooldown = 0;
	bool isActive = false;

};
