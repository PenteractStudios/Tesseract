#pragma once

#include "Scripting/Script.h"

class TestMSAA : public Script
{
	GENERATE_BODY(TestMSAA);

public:

	void Start() override;
	void Update() override;

};

