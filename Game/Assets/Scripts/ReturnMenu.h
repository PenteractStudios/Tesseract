#pragma once

#include "Scripting/Script.h"

class GameObject;
class ReturnMenu : public Script
{
	GENERATE_BODY(ReturnMenu);

public:
	void Start() override;
	void Update() override;
	void OnButtonClick() override;

public:
	UID buttonUID = 0;
	UID sceneUID = 0;
	float padding = 20.f;
};

