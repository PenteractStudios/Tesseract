#pragma once

#include "Scripting/Script.h"

class HUDTest : public Script
{
	GENERATE_BODY(HUDTest);

public:

	void Start() override;
	void Update() override;

public:
	float cooldown1 = 0;
	float cooldown2 = 0;
	float cooldown3 = 0;
	float cooldown4 = 0;
	float cooldown5 = 0;
	float cooldown6 = 0;
	float cooldown7 = 0;

	int fangHP = 6;
	int onimaruHP = 4;
};

