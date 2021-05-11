#pragma once

#include "Scripting/Script.h"

class Resume : public Script
{
	GENERATE_BODY(Resume);

public:

	void Start() override;
	void Update() override;
	void OnButtonClick() override;

public:
	UID pauseUID;

private:
	GameObject* pauseCanvas = nullptr;
};

