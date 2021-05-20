#pragma once

#include "Scripting/Script.h"

class GameObject;
class ComponentTransform;
class ComponentCamera;
class ComponentAudioSource;
class ComponentParticleSystem;

class FangGun : public Script
{
	GENERATE_BODY(FangGun);

public:

	void Start() override;
	void Update() override;
public:
	UID prefabId = 0;
};
