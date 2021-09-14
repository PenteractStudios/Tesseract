#pragma once

#include "Scripting/Script.h"

class GlobalVariablesTest : public Script
{
	GENERATE_BODY(GlobalVariablesTest);

public:

	void Start() override;
	void Update() override;
};

