#pragma once

#include "Scripting/Script.h"
#include "Math/float3.h"

class GameObject;
class ComponentTransform;
class ComponentCamera;

enum class MovementDirection {
		NONE = 0, UP = 1, UP_LEFT = 2, LEFT = 3, DOWN_LEFT = 4, DOWN = 5, DOWN_RIGHT = 6, RIGHT = 7, UP_RIGHT = 8
};

class PlayerController : public Script
{
	GENERATE_BODY(PlayerController);

public:

	void Start() override;
	void Update() override;

public:

	GameObject* gameObject = nullptr;
	GameObject* camera = nullptr;
	GameObject* fang = nullptr;
	GameObject* robot = nullptr;

	UID fangUID = 0;
	UID robotUID = 0;
	UID mainNodeUID = 0;
	UID cameraUID = 0;

	float dashCooldown = 5.f; //seconds
	float switchCooldown = 5.f;
	float movementSpeed = 5.f;
	float dashSpeed = 100.f;
	float dashDistance = 10.f;
	float cameraOffsetZ = 20.f;
	float cameraOffsetY = 10.f;

private:

	void MoveTo(MovementDirection md);
	void InitDash(MovementDirection md);
	void Dash();
	void LookAtMouse();
	void CheckCoolDowns();
	void SwitchCharacter();
	bool CanDash();
	bool CanSwitch();
	float3 GetDirection(MovementDirection md) const;

private:

	float dashError = 2.f;
	float dashCooldownRemaing = 0.f;
	bool dashInCooldown = false;
	bool dashing = false;

	float switchCooldownRemaing = 0.f;
	bool switchInCooldown = false;

	float3 initialPosition = float3(0, 0, 0);
	float3 dashDestination = float3(0, 0, 0);
	float3 dashDirection = float3(0, 0, 0);

	ComponentTransform* transform = nullptr;
	ComponentCamera* compCamera = nullptr;
};

