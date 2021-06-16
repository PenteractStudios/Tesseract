#pragma once

#include "Scripting/Script.h"

class GameObject;

class StartButton : public Script
{
	GENERATE_BODY(StartButton);

public:

	void Start() override;
	void Update() override;
	void OnButtonClick() override;

public:
	UID sceneUID = 0;
	int checkpointNum = -1;

private:
	GameObject* player = nullptr;
};
