#pragma once

#include "Scripting/Script.h"

class GameObject;
class ComponentText;

class StatsDisplayer : public Script
{
	GENERATE_BODY(StatsDisplayer);

public:

	void Start() override;
	void Update() override;

public:
	UID fangUID;
	UID canvasUID;

private:
	GameObject* canvas = nullptr;
	GameObject* player = nullptr;

	ComponentText* fpsText = nullptr;
	ComponentText* trigText = nullptr;
	ComponentText* positionText = nullptr;
	ComponentText* cameraText = nullptr;

	bool panelActive = false;
};

