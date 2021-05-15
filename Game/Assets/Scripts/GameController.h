#pragma once

#include "Scripting/Script.h"

#include "Geometry/Frustum.h"

class GameObject;
class ComponentCamera;
class ComponentTransform;


class GameController : public Script {
	GENERATE_BODY(GameController);

public:

	void Start() override;
	void Update() override;

	void Rotate(float2 mouseMotion, Frustum* frustum, ComponentTransform* transform);

public:
	UID gameCameraUID;
	UID godCameraUID;
	UID staticCamera1UID;
	UID staticCamera2UID;
	UID staticCamera3UID;
	UID staticCamera4UID;
	UID playerUID;
	UID pauseUID;
	UID hudUID;
	UID enemySpawnPointsUID;

	float speed = 50.f;
	float rotationSpeedX = 10.f;
	float rotationSpeedY = 10.f;
	float focusDistance = 100.f;
	float transitionSpeed = 1000.f;

private:
	void DoTransition();
	void SpawnEnemies();

private:
	GameObject* gameCamera = nullptr;
	GameObject* godCamera = nullptr;
	ComponentCamera* camera = nullptr;
	ComponentCamera* staticCamera1 = nullptr;
	ComponentCamera* staticCamera2 = nullptr;
	ComponentCamera* staticCamera3 = nullptr;
	ComponentCamera* staticCamera4 = nullptr;
	GameObject* player = nullptr;
	GameObject* pauseCanvas = nullptr;
	GameObject* hudCanvas = nullptr;

	float yaw = 0.f;
	float pitch = 0.f;
	bool showWireframe = false;
	bool godModeAvailable = false;
	bool transitionFinished = false;
	bool isPaused = false;
};

