#pragma once

#include "Scripting/Script.h"

class ModifyTiling : public Script
{
	GENERATE_BODY(ModifyTiling);

public:

	void Start() override;
	void Update() override;

	bool isTiling = false;
	bool isOffset = false;
	float multiplierX = 0.0f;
	float multiplierY = 0.0f;

private:
	float2 tiling = { 0.0f, 0.0f };
	float2 offset = { 0.0f, 0.0f };
};

