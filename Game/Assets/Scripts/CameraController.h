#pragma once

#include "Scripting/Script.h"

class PlayerController;
class ComponentTransform;

class CameraController : public Script {
	GENERATE_BODY(CameraController);


public:

	void Start() override;
	void Update() override;
	void StartShake();
public:
	float cameraOffsetZ = 20.f;
	float cameraOffsetY = 20.f;
	float cameraOffsetX = 0.f;
	bool useSmoothCamera = true;
	float smoothCameraSpeed = 5.0f;
	UID playerControllerObjUID = 0;
	PlayerController* playerController = nullptr;
	ComponentTransform* transform = nullptr;
	float shakeTotalTime = 1.0f;
	float shakeTimer = 0.0f;
	float shakeMultiplier = 0.2f;

private:
	float2 GetRandomPosInUnitaryCircle(float2 center);

};

