#pragma once

#include "Scripting/Script.h"
class GameObject;
class ComponentTransform;
class ComponentCamera;
class ComponentAudioSource;
class ComponentParticleSystem;

class TrailTest : public Script
{
	GENERATE_BODY(TrailTest);

public:

	void Start() override;
	void Update() override;
	void LookAtMouse();

public:
	UID prefabId = 0;
	UID gunId = 0;
	UID sceneId = 0;
	int speed = 10;
	UID cameraUID = 0;
	float cameraOffsetZ = 20.f;
	float cameraOffsetY = 10.f;
	GameObject* camera = nullptr;
private:

	float3 facePointDir = float3(0, 0, 0);

	GameObject* gameObject = nullptr;
	GameObject* scene = nullptr;
	GameObject* fangGun = nullptr;
	ComponentTransform* transform = nullptr;
	ComponentTransform* FangGuntransform = nullptr;
	ComponentCamera* compCamera = nullptr;
};
