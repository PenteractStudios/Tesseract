#pragma once

#include "Scripting/Script.h"

class AudioController : public Script
{
	GENERATE_BODY(AudioController);

public:

	void Start() override;
	void Update() override;

public:
	UID audioSourceUID = 0;

private:
	GameObject* audioSourceGameObject = nullptr;
};

