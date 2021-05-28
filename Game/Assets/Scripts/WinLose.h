#pragma once

#include "Scripting/Script.h"

class GameObject;

class WinLose : public Script
{
	GENERATE_BODY(WinLose);

public:

	void Start() override;
	void Update() override;

public:
	UID winUID;
	UID playerUID;

	float LoseOffsetX = 2.0f;
	float LoseOffsetZ = 2.0f;

private:
	GameObject* winCon = nullptr;
	GameObject* player = nullptr;


};
